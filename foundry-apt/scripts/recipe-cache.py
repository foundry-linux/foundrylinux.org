#!/usr/bin/env python3
"""Conservative per-recipe cache receipts, never version/file-presence shortcuts.

Candidate dependency closure metadata is part of the key. Actual installed
versions used by the previous build must still be installed or available as
the candidate before reuse. An unresolved environment disables reuse.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess


def run(*args):
    return subprocess.check_output(args, text=True, stderr=subprocess.PIPE).strip()


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def stanzas(path):
    result = []
    for block in re.split(r'\n\s*\n', path.read_text()):
        fields = {}
        key = None
        for line in block.splitlines():
            if line.startswith('#'):
                continue
            if line.startswith((' ', '\t')) and key:
                fields[key] += ' ' + line.strip()
            elif ':' in line:
                key, value = line.split(':', 1)
                fields[key] = value.strip()
        if fields:
            result.append(fields)
    return result


def tree(path):
    # Include untracked recipe inputs and file modes as well as tracked content.
    return {str(p.relative_to(path)): {'sha256': digest(p), 'executable': bool(p.stat().st_mode & 0o111)}
            for p in sorted(path.rglob('*')) if p.is_file() and not any(
                part in {'.git', '.debhelper', '__pycache__'} for part in p.relative_to(path).parts)}


def dependency_names(control):
    names = set()
    for field in ['Build-Depends', 'Build-Depends-Indep', 'Build-Depends-Arch']:
        for group in control.get(field, '').split(','):
            for alternative in group.split('|'):
                match = re.match(r'\s*([a-z0-9][a-z0-9+.-]*)', alternative)
                if match:
                    names.add(match[1])
    return names


def policy(names):
    text = run('apt-cache', 'policy', *sorted(names))
    result = {}
    package = None
    for line in text.splitlines():
        if line and not line[0].isspace() and line.endswith(':'):
            package = line[:-1]
            result[package] = {}
        elif package and line.strip().startswith(('Installed:', 'Candidate:')):
            key, value = line.strip().split(': ', 1)
            result[package][key.lower()] = value
    return result


def dependency_context(root, recipe):
    fields = stanzas(root / 'packages' / recipe / 'debian/control')[0]
    names = dependency_names(fields) | {'build-essential', 'dpkg-dev', 'debhelper'}
    # Virtual debhelper-compat is provided by debhelper, already included.
    names.discard('debhelper-compat')
    text = run('apt-cache', 'depends', '--recurse', '--no-recommends', '--no-suggests',
               '--no-conflicts', '--no-breaks', '--no-replaces', '--no-enhances', *sorted(names))
    closure = set(names)
    for line in text.splitlines():
        name = line.strip()
        if re.fullmatch(r'[a-z0-9][a-z0-9+.-]*(?::[a-z0-9-]+)?', name):
            closure.add(name)
    versions = policy(closure)
    if not versions or any(name not in versions for name in names):
        raise ValueError('Unresolved build dependencies; refresh APT metadata before caching')
    candidates = {name: item.get('candidate') for name, item in versions.items()}
    if any(not value or value == '(none)' for value in candidates.values()):
        raise ValueError('Missing candidate in dependency closure; refusing cache reuse')
    return candidates, versions


def inputs(root, recipe, dependency_lock, builder, arch):
    if not builder:
        raise ValueError('FOUNDRY_BUILDER_ID is required for cache reuse')
    recipes = root / 'packages'
    binary_owners = {}
    for directory in recipes.iterdir():
        control = directory / 'debian/control'
        if control.is_file():
            for item in stanzas(control):
                if 'Package' in item:
                    binary_owners[item['Package']] = directory.name
    visited = {}

    def visit(name):
        if name in visited:
            return
        directory = recipes / name
        visited[name] = tree(directory)
        control = stanzas(directory / 'debian/control')[0]
        dependencies = {binary_owners[n] for n in dependency_names(control) if n in binary_owners}
        extra = directory / '.build-cache-depends'
        if extra.exists():
            dependencies |= {line.split('#', 1)[0].strip() for line in extra.read_text().splitlines()}
            dependencies.discard('')
        for dependency in sorted(dependencies):
            if dependency == name:
                continue
            if not re.fullmatch(r'[a-z0-9][a-z0-9+.-]*', dependency) or not (recipes / dependency / 'debian/control').is_file():
                raise ValueError(f'Invalid local recipe dependency: {dependency}')
            visit(dependency)
    visit(recipe)
    return {'schema': 1, 'builder': builder, 'architecture': arch,
            'recipes': visited, 'shared_scripts': tree(root / 'scripts'),
            'builder_recipe': tree(root / 'docker'), 'dependency_candidates': dependency_lock,
            'build_environment': {name: os.environ.get(name, '') for name in sorted(set(
                ['DEB_BUILD_OPTIONS', 'DEB_BUILD_PROFILES', 'DEB_BUILD_MAINT_OPTIONS', 'SOURCE_DATE_EPOCH',
                 'CFLAGS', 'CXXFLAGS', 'CPPFLAGS', 'LDFLAGS', 'CC', 'CXX',
                 'FOUNDRY_BUILD_JOBS', 'FOUNDRY_COMPRESSION_THREADS', 'FOUNDRY_XZ_MEMLIMIT'] +
                [name for name in os.environ if re.search(r'_(VERSION|SHA256|SHA|COMMIT|TAG|REVISION|URL)$', name)]))}}


def artifacts(root, recipe, arch, built_after_ns=0):
    directory = root / 'packages' / recipe
    blocks = stanzas(directory / 'debian/control')
    version = run('dpkg-parsechangelog', '-l', str(directory / 'debian/changelog'), '-SVersion')
    expected = {b['Package'] for b in blocks if 'Package' in b and
                set(b.get('Architecture', '').split()) & {'all', 'any', arch, 'linux-any', 'any-' + arch}}
    source = blocks[0]['Source']
    found = set()
    outputs = {}
    for path in sorted((root / 'dist').glob('*.deb')):
        values = run('dpkg-deb', '--show',
            '--showformat=${Package}\t${Version}\t${Architecture}\t${Source}', str(path)).split('\t')
        package, actual_version, actual_arch = values[:3]
        actual_source = values[3].split()[0] if len(values) > 3 and values[3] else package
        if (package in expected or actual_source == source) and actual_version == version and actual_arch in {'all', arch}:
            if path.stat().st_mtime_ns < built_after_ns:
                raise ValueError(f'Output was not refreshed by this build: {path.name}')
            if package in expected:
                found.add(package)
            outputs[path.name] = digest(path)
    if not expected or expected - found:
        raise ValueError(f'Incomplete binary output for {recipe}: {sorted(expected - found)}')
    # Every current recipe publishes a Sources index, including binary repacks.
    # A source-emission failure must not make an incomplete output set cacheable.
    dsc = root / 'dist' / f'{source}_{version.split(":", 1)[-1]}.dsc'
    if not dsc.is_file() or dsc.stat().st_mtime_ns < built_after_ns:
        raise ValueError(f'Missing or stale source descriptor: {dsc.name}')
    descriptor = next((b for b in stanzas(dsc) if 'Source' in b and 'Version' in b), {})
    if descriptor.get('Source') != source or descriptor.get('Version') != version:
        raise ValueError(f'Source descriptor metadata mismatch: {dsc.name}')
    if dsc.exists():
        outputs[dsc.name] = digest(dsc)
        in_checksums = False
        source_files = 0
        for line in dsc.read_text().splitlines():
            if line == 'Checksums-Sha256:':
                in_checksums = True
            elif in_checksums and line.startswith(' '):
                sha, size, name = line.split()
                path = root / 'dist' / name
                if Path(name).name != name or not path.is_file() or digest(path) != sha or path.stat().st_size != int(size):
                    raise ValueError(f'Missing/corrupt source output: {name}')
                outputs[name] = sha
                source_files += 1
            elif in_checksums:
                break
        if not source_files:
            raise ValueError(f'Source descriptor has no SHA256 file inventory: {dsc.name}')
    return outputs


def verify(receipt, current_inputs, dist, versions):
    if receipt['inputs'] != current_inputs:
        return False
    for name, built_version in receipt.get('installed_dependencies', {}).items():
        available = versions.get(name, {})
        if built_version not in {available.get('installed'), available.get('candidate')}:
            return False
    for name, sha in receipt['outputs'].items():
        if Path(name).name != name or not (dist / name).is_file() or digest(dist / name) != sha:
            return False
    return bool(receipt['outputs'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['check', 'record'])
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--recipe', required=True)
    parser.add_argument('--built-after-ns', type=int, default=0)
    args = parser.parse_args()
    root = args.root.resolve()
    if not re.fullmatch(r'[a-z0-9][a-z0-9+.-]*', args.recipe):
        parser.error('Invalid recipe name')
    receipt = root / 'dist/.recipe-cache' / (args.recipe + '.json')
    try:
        candidates, versions = dependency_context(root, args.recipe)
        key = inputs(root, args.recipe, candidates, os.environ.get('FOUNDRY_BUILDER_ID'), run('dpkg', '--print-architecture'))
        if args.action == 'check':
            recorded = json.loads(receipt.read_text()) if receipt.exists() else None
            if (not recorded or not verify(recorded, key, root / 'dist', versions) or
                    artifacts(root, args.recipe, key['architecture']) != recorded['outputs']):
                print(f'CACHE MISS {args.recipe}')
                return 1
            print(f'CACHE HIT {args.recipe}: verified inputs and all recorded outputs')
            return 0
        result = {'inputs': key, 'outputs': artifacts(root, args.recipe, key['architecture'], args.built_after_ns),
                  'installed_dependencies': {n: v['installed'] for n, v in versions.items()
                      if v.get('installed') not in {None, '(none)'}}}
        receipt.parent.mkdir(parents=True, exist_ok=True)
        temporary = receipt.with_suffix('.tmp')
        temporary.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n')
        temporary.replace(receipt)
        print(f'CACHE RECORDED {args.recipe}')
        return 0
    except (ValueError, KeyError, OSError, subprocess.CalledProcessError) as error:
        print(f'CACHE UNAVAILABLE {args.recipe}: {error}')
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
