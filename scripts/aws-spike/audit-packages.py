#!/usr/bin/env python3
"""Check built binary inventory against the selected recipes' Debian metadata."""
import argparse
import json
import pathlib
import re
import subprocess

p = argparse.ArgumentParser()
p.add_argument('--apt-root', type=pathlib.Path, required=True)
a = p.parse_args()
expected = []
for recipe in sorted((a.apt_root / 'packages').iterdir()):
    if not recipe.is_dir() or (recipe / '.shared-electron-only').exists():
        continue
    control = recipe / 'debian/control'
    changelog = recipe / 'debian/changelog'
    if not control.exists() or not changelog.exists():
        continue
    match = re.match(r'\S+ \(([^)]+)\)', changelog.read_text())
    if not match:
        raise SystemExit(f'Cannot determine version: {changelog}')
    version = match[1]
    for stanza in re.split(r'\n\s*\n', control.read_text()):
        fields = dict(re.findall(r'^(Package|Architecture):\s*(.+)$', stanza, re.M))
        if 'Package' not in fields:
            continue
        arches = fields.get('Architecture', '').split()
        if not set(arches).intersection({'all', 'any', 'amd64', 'linux-any', 'any-amd64'}):
            continue
        expected.append({'recipe': recipe.name, 'package': fields['Package'],
                         'version': version, 'arch': 'all' if arches == ['all'] else 'amd64'})
actual = {}
for deb in sorted((a.apt_root / 'dist').glob('*.deb')):
    result = subprocess.check_output(['dpkg-deb', '--show',
        '--showformat=${Package}\t${Version}\t${Architecture}', str(deb)], text=True)
    key = tuple(result.split('\t'))
    actual.setdefault(key, []).append(deb.name)
missing = [item for item in expected if (item['package'], item['version'], item['arch']) not in actual]
print(json.dumps({'expected_binary_packages': len(expected), 'artifact_files': sum(map(len, actual.values())),
    'missing': missing, 'expected': expected, 'artifacts': [{'package': key[0], 'version': key[1],
        'arch': key[2], 'files': files} for key, files in actual.items()]}, indent=2))
raise SystemExit(1 if missing else 0)
