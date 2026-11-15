#!/usr/bin/env python3
"""Run in the package-builder container: python3 scripts/test_recipe_cache.py."""
import copy
import importlib.util
import os
from pathlib import Path
import subprocess
import tempfile
import time
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('recipe_cache', Path(__file__).with_name('recipe-cache.py'))
cache = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cache)


class CacheTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'dist').mkdir()
        for name in ['scripts', 'docker']:
            (self.root / name).mkdir()
            (self.root / name / 'shared').write_text('original')
        for name in ['libfixture', 'consumer', 'unrelated']:
            d = self.root / 'packages' / name / 'debian'
            d.mkdir(parents=True)
            deps = 'libfixture' if name == 'consumer' else 'debhelper-compat (= 13)'
            (d / 'control').write_text(f'Source: {name}\nBuild-Depends: {deps}\n\nPackage: {name}\nArchitecture: all\nDescription: fixture\n')
            (d / 'changelog').write_text(f'{name} (1.0-1) unstable; urgency=medium\n\n  * Fixture.\n\n -- Test <test@example.org>  Sun, 15 Nov 2026 00:00:00 +0000\n')

    def inputs(self, name='consumer', builder='sha256:builder', dependencies=None):
        return cache.inputs(self.root, name, dependencies or {'gcc': '15'}, builder, 'amd64')

    def test_unrelated_recipe_does_not_invalidate_but_dependency_does(self):
        before = self.inputs()
        (self.root / 'packages/unrelated/new-source').write_text('new')
        self.assertEqual(before, self.inputs())
        (self.root / 'packages/libfixture/new-source').write_text('new')
        self.assertNotEqual(before, self.inputs())

    def test_same_version_edit_shared_toolchain_and_dependency_version(self):
        before = self.inputs()
        self.assertNotEqual(before, self.inputs(builder='sha256:new'))
        self.assertNotEqual(before, self.inputs(dependencies={'gcc': '16'}))
        (self.root / 'packages/consumer/patch').write_text('same version new source')
        self.assertNotEqual(before, self.inputs())
        before = self.inputs()
        (self.root / 'scripts/shared').write_text('new shared build helper')
        self.assertNotEqual(before, self.inputs())

    def test_explicit_local_dependency_and_cycles(self):
        (self.root / 'packages/consumer/.build-cache-depends').write_text('unrelated\n')
        (self.root / 'packages/unrelated/.build-cache-depends').write_text('consumer\n')
        self.assertEqual(set(self.inputs()['recipes']), {'consumer', 'libfixture', 'unrelated'})

    def test_source_override_changes_key(self):
        before = self.inputs()
        with patch.dict(os.environ, {'CONSUMER_SHA256': 'different'}):
            self.assertNotEqual(before, self.inputs())

    def test_absent_builder_fails_closed(self):
        with self.assertRaises(ValueError):
            self.inputs(builder='')

    def test_output_corruption_missing_and_dependency_drift(self):
        output = self.root / 'dist/consumer.deb'
        output.write_bytes(b'fixture')
        key = self.inputs()
        receipt = {'inputs': key, 'outputs': {output.name: cache.digest(output)},
                   'installed_dependencies': {'gcc': '15'}}
        versions = {'gcc': {'installed': '(none)', 'candidate': '15'}}
        self.assertTrue(cache.verify(receipt, key, output.parent, versions))
        self.assertFalse(cache.verify(receipt, key, output.parent, {'gcc': {'candidate': '16'}}))
        output.write_bytes(b'corrupt')
        self.assertFalse(cache.verify(receipt, key, output.parent, versions))
        output.unlink()
        self.assertFalse(cache.verify(receipt, key, output.parent, versions))

    def test_artifact_metadata_and_stale_output(self):
        package = self.root / 'deb'
        (package / 'DEBIAN').mkdir(parents=True)
        (package / 'DEBIAN/control').write_text('Package: consumer\nVersion: 1.0-1\nArchitecture: all\nMaintainer: Test <test@example.org>\nDescription: fixture\n')
        output = self.root / 'dist/consumer_1.0-1_all.deb'
        subprocess.run(['dpkg-deb', '--build', str(package), str(output)], check=True, stdout=subprocess.DEVNULL)
        with self.assertRaisesRegex(ValueError, 'source descriptor'):
            cache.artifacts(self.root, 'consumer', 'amd64')
        source = self.root / 'dist/consumer_1.0-1.tar.xz'
        source.write_bytes(b'source-fixture')
        dsc = self.root / 'dist/consumer_1.0-1.dsc'
        dsc.write_text(f'Source: consumer\nVersion: 1.0-1\nChecksums-Sha256:\n {cache.digest(source)} {source.stat().st_size} {source.name}\n')
        self.assertIn(output.name, cache.artifacts(self.root, 'consumer', 'amd64'))
        (package / 'DEBIAN/control').write_text('Package: consumer-dbgsym\nSource: consumer\nVersion: 1.0-1\nArchitecture: all\nMaintainer: Test <test@example.org>\nDescription: debug fixture\n')
        debug = self.root / 'dist/consumer-dbgsym_1.0-1_all.deb'
        subprocess.run(['dpkg-deb', '--build', str(package), str(debug)], check=True, stdout=subprocess.DEVNULL)
        self.assertIn(debug.name, cache.artifacts(self.root, 'consumer', 'amd64'))
        source.write_bytes(b'corrupt-source')
        with self.assertRaisesRegex(ValueError, 'source output'):
            cache.artifacts(self.root, 'consumer', 'amd64')
        source.write_bytes(b'source-fixture')
        with self.assertRaises(ValueError):
            cache.artifacts(self.root, 'consumer', 'amd64', time.time_ns() + 1_000_000)
        control = self.root / 'packages/consumer/debian/control'
        control.write_text(control.read_text() + '\nPackage: consumer-extra\nArchitecture: all\nDescription: another required binary\n')
        with self.assertRaisesRegex(ValueError, 'consumer-extra'):
            cache.artifacts(self.root, 'consumer', 'amd64')


if __name__ == '__main__':
    unittest.main()
