#!/usr/bin/env python3
"""Select local candidates before live-build installs the edition's packages.

Remote APT indexes must already be authenticated and refreshed. Only this
build-local file repository is trusted without a signature; remote policies
remain unchanged. APT checks the SHA256 fields in its generated Packages index.
"""
import argparse
import hashlib
import json
import pathlib
import re
import shutil
import subprocess

LOCAL = pathlib.Path('/opt/foundry-local-apt')
SOURCE = pathlib.Path('/etc/apt/sources.list.d/00-foundry-local.list')
PREF = pathlib.Path('/etc/apt/preferences.d/foundry-local.pref')
FIELDS = ['Depends', 'Pre-Depends', 'Recommends', 'Provides', 'Conflicts', 'Breaks', 'Replaces']


def command(*args, **kwargs):
    return subprocess.check_output(args, text=True, **kwargs).strip()


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def verify():
    manifest = json.loads((LOCAL / 'manifest.json').read_text())
    for item in manifest:
        package = item['package']
        status = command('dpkg-query', '-W', '-f=${Status}\t${Version}', package)
        if status != f"install ok installed\t{item['version']}":
            raise SystemExit(f'Local candidate not installed as selected: {package}: {status}')
        for field, expected in item['fields'].items():
            actual = command('dpkg-query', '-W', '-f=${' + field + '}', package)
            if actual != expected:
                raise SystemExit(f'Local candidate metadata mismatch: {package} {field}')
        expected = item['md5sums_sha256']
        if expected:
            paths = [pathlib.Path('/var/lib/dpkg/info') / (package + suffix + '.md5sums')
                     for suffix in ['', ':' + item['architecture']]]
            if not any(path.is_file() and sha(path) == expected for path in paths):
                raise SystemExit(f'Local candidate payload metadata mismatch: {package}')
    evidence = pathlib.Path('/usr/share/foundry/build')
    evidence.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(LOCAL / 'manifest.json', evidence / 'local-packages.json')
    print(f'PASS: {len(manifest)} local candidates installed once from the build repository')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=pathlib.Path)
    p.add_argument('--archives', type=pathlib.Path)
    p.add_argument('--edition', choices=['anvil', 'atelier'])
    p.add_argument('--package-list', type=pathlib.Path)
    p.add_argument('--allow-older', action='store_true')
    p.add_argument('--verify', action='store_true')
    a = p.parse_args()
    if a.verify:
        verify()
        return
    if not all([a.root, a.archives, a.edition, a.package_list]):
        p.error('--root, --archives, --edition and --package-list are required')
    root = a.root.resolve()
    local = root / LOCAL.relative_to('/')
    if local.exists():
        raise SystemExit(f'Refusing to reuse a previous local repository: {local}')
    a.package_list.write_text('')
    archives = sorted(a.archives.glob('*.deb'))
    if not archives:
        return
    def inside(*args):
        return command('chroot', str(root), *args)
    selected = {}
    for archive in archives:
        package = command('dpkg-deb', '-f', str(archive), 'Package')
        version = command('dpkg-deb', '-f', str(archive), 'Version')
        if not re.fullmatch(r'[a-z0-9][a-z0-9+.-]+', package):
            raise SystemExit(f'Invalid binary package name: {package!r}')
        previous = selected.get(package)
        if previous and subprocess.run(['dpkg', '--compare-versions', version, 'le', previous[1]]).returncode == 0:
            continue
        selected[package] = (archive, version)
    local.mkdir(parents=True)
    metadata = {}
    for package, (archive, version) in sorted(selected.items()):
        match = re.search(r'^\s*Candidate:\s*(\S+)', inside('apt-cache', 'policy', package), re.M)
        candidate = match.group(1) if match else '(none)'
        if not a.allow_older and candidate != '(none)' and subprocess.run(
                ['dpkg', '--compare-versions', version, 'lt', candidate]).returncode == 0:
            print(f'Skipping older local build: {package} {version} < {candidate}')
            continue
        dest = local / f'{package}.deb'
        shutil.copyfile(archive, dest)
        # dpkg-deb exits nonzero when md5sums is absent (e.g. empty metapackage).
        control = subprocess.run(['dpkg-deb', '--ctrl-tarfile', str(archive)], capture_output=True, check=True)
        import io
        import tarfile
        with tarfile.open(fileobj=io.BytesIO(control.stdout)) as tar:
            member = next((m for m in tar.getmembers() if m.name.lstrip('./') == 'md5sums'), None)
            md5 = hashlib.sha256(tar.extractfile(member).read()).hexdigest() if member else None
        metadata[package] = {'package': package, 'version': version, 'archive_sha256': sha(dest),
            'architecture': command('dpkg-deb', '-f', str(archive), 'Architecture'),
            'md5sums_sha256': md5,
            'fields': {field: command('dpkg-deb', '-f', str(archive), field) for field in FIELDS}}
    def index():
        with (local / 'Packages').open('w') as output:
            subprocess.run(['dpkg-scanpackages', '.', '/dev/null'], cwd=local, stdout=output, check=True)
        release = command('apt-ftparchive', 'release', '.', cwd=local)
        (local / 'Release').write_text('Origin: FoundryLocal\nLabel: Foundry build inputs\n' + release + '\n')
        inside('apt-get', 'update', '-o', 'APT::Update::Error-Mode=any',
               '-o', f'Dir::Etc::sourcelist={SOURCE}', '-o', 'Dir::Etc::sourceparts=-',
               '-o', 'APT::Get::List-Cleanup=0')
    source = root / SOURCE.relative_to('/')
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text(f'deb [trusted=yes] file:{LOCAL} ./\n')
    preference = root / PREF.relative_to('/')
    preference.parent.mkdir(parents=True, exist_ok=True)
    preference.write_text('Package: *\nPin: release o=FoundryLocal\nPin-Priority: ' +
                          ('1001' if a.allow_older else '501') + '\n')
    index()
    dependencies = inside('apt-cache', 'depends', '--recurse', '--no-recommends', '--no-suggests',
                         '--no-conflicts', '--no-breaks', '--no-replaces', '--no-enhances', f'foundry-{a.edition}')
    closure = set(re.findall(r'^([a-z0-9][a-z0-9+.-]+)(?::[a-z0-9-]+)?$', dependencies, re.M))
    for package in list(metadata):
        if package != f'foundry-{a.edition}' and not package.startswith('calamares-settings-') and package not in closure:
            print(f'Skipping local package outside {a.edition} dependency closure: {package}')
            (local / f'{package}.deb').unlink()
            del metadata[package]
    index()
    (local / 'manifest.json').write_text(json.dumps(list(metadata.values()), indent=2) + '\n')
    shutil.copyfile(__file__, local / 'verify.py')
    a.package_list.write_text(''.join(f'{name}={item["version"]}\n' for name, item in metadata.items()))
    print(f'Staged {len(metadata)} verified local candidates for the initial package installation')


if __name__ == '__main__':
    main()
