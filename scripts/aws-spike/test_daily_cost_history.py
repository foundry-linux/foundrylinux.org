"""Offline fixture tests for history boundaries and cost accounting."""
import argparse
import copy
import importlib.util
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location('history', Path(__file__).with_name('daily-cost-history.py'))
history_tool = importlib.util.module_from_spec(spec)
spec.loader.exec_module(history_tool)


class SimulationTests(unittest.TestCase):
    def setUp(self):
        self.history = {
            'window': {'start_date': '2026-10-31', 'end_date': '2026-11-01', 'days': 2, 'timezone': 'Asia/Bangkok'},
            'coverage': [{'package': 'example', 'status': 'ok'}],
            'events': [
                {'package': 'example', 'kind': 'release', 'day': '2026-10-31', 'version': 'v1.1'},
                {'package': 'example', 'kind': 'release', 'day': '2026-10-31', 'version': 'v1.2'},
                {'package': 'example', 'kind': 'repo_change', 'day': '2026-10-31', 'version': 'abc'},
            ],
        }
        self.measured = {'markers': [{'package': 'example', 'seconds': 60}],
                         'artifacts': [{'package': 'example', 'bytes': 200_000_000}],
                         'iso_sizes': {'warm': 1_000_000_000}}
        self.scenario = {
            'compute_per_hour': 3.6, 'ipv4_per_hour': 0, 'ebs_gib': 30,
            'ebs_per_gib_month': .1, 'warm_workflow_seconds': 100,
            'discovery_seconds': 10, 'startup_seconds': 20, 'validation_seconds': 30,
            'publication_seconds': 40, 'artifact_retention_days': 2,
            'storage_per_gb_month': .03, 'egress_per_gb': .1,
        }

    def test_coalesces_releases_and_repo_changes_once_per_recipe_day(self):
        result = history_tool.simulate(self.history, self.measured, self.scenario)
        self.assertEqual(result['totals']['package_builds'], 1)
        self.assertEqual(result['daily'][0]['package_build_seconds_proxy'], 60)
        self.assertEqual(result['daily'][1]['packages'], [])
        self.assertAlmostEqual(result['daily'][0]['total_usd'], .4812)
        self.assertAlmostEqual(result['daily'][1]['total_usd'], .4022)

    def test_projection_repeats_history_and_expires_retention(self):
        result = history_tool.simulate(self.history, self.measured, self.scenario, 3)
        self.assertEqual(result['totals']['package_builds'], 2)
        self.assertEqual(result['daily'][2]['history_day'], '2026-10-31')
        self.assertEqual(result['daily'][2]['date'], '2026-11-02')
        self.assertAlmostEqual(result['daily'][2]['artifact_retained_gb_model'], 2.2)

    def test_shared_allowance_resets_by_calendar_month_not_history_window(self):
        self.scenario.update(available_egress_free_gb_per_calendar_month=1, public_download_gb_per_day=1.2, public_download_per_gb=.1, egress_per_gb=0)
        result = history_tool.simulate(self.history, self.measured, self.scenario)
        self.assertAlmostEqual(result['daily'][0]['egress_usd'], .02)
        self.assertAlmostEqual(result['daily'][1]['egress_usd'], .02)

    def test_same_region_upload_has_no_egress_charge(self):
        self.scenario.update(egress_per_gb=0, public_download_per_gb=.1)
        result = history_tool.simulate(self.history, self.measured, self.scenario)
        self.assertEqual(result['totals']['egress_usd'], 0)
        self.scenario['public_download_gb_per_day'] = 2
        result = history_tool.simulate(self.history, self.measured, self.scenario)
        self.assertAlmostEqual(result['totals']['egress_usd'], .4)

    def test_policy_filters_preserve_local_fixes(self):
        self.scenario['upstream_version_prefixes'] = {'example': 'v1.1'}
        selected = history_tool.selected_events(self.history, self.scenario)
        self.assertEqual({e['version'] for e in selected}, {'v1.1', 'abc'})
        self.scenario['activity'] = 'upstream'
        self.assertEqual(len(history_tool.selected_events(self.history, self.scenario)), 1)

    def test_unknown_recipe_is_reported_not_claimed_zero_work(self):
        self.measured['markers'] = []
        result = history_tool.simulate(self.history, self.measured, self.scenario)
        self.assertEqual(result['daily'][0]['unknown_timing_packages'], ['example'])
        self.assertTrue(any('missing timing' in w for w in result['warnings']))

    def test_price_change_does_not_invent_hardware_speedup(self):
        old = history_tool.simulate(self.history, self.measured, self.scenario)
        self.scenario['compute_per_hour'] /= 2
        new = history_tool.simulate(self.history, self.measured, self.scenario)
        self.assertEqual(old['daily'][0]['runner_seconds_model'], new['daily'][0]['runner_seconds_model'])
        self.assertAlmostEqual(new['totals']['compute_usd'], old['totals']['compute_usd'] / 2)

    def test_monorepo_tag_filters_and_prereleases(self):
        self.assertTrue(history_tool.stable('wf-asset-v1.2', 'blender-asset-finder-cli'))
        self.assertFalse(history_tool.stable('v1.2', 'blender-asset-finder-cli'))
        self.assertFalse(history_tool.stable('v1.2-rc1', 'example'))
        self.assertTrue(history_tool.stable('Ghidra_12.1_build', 'ghidra'))

    def test_smaller_host_changes_only_build_stage(self):
        old = history_tool.simulate(self.history, self.measured, self.scenario)
        self.scenario.update(build_compute_per_hour=1.8, build_runtime_multiplier=1.5)
        new = history_tool.simulate(self.history, self.measured, self.scenario)
        self.assertAlmostEqual(new['totals']['build_compute_usd'], old['totals']['build_compute_usd'] * .75)
        for key in ['other_compute_usd', 'ebs_usd', 'egress_usd', 'artifact_storage_usd']:
            self.assertEqual(new['totals'][key], old['totals'][key])
        self.assertEqual(new['daily'][0]['runner_seconds_model'], 340)

    def test_source_orig_uploads_only_for_upstream_changes(self):
        self.scenario['include_source_uploads'] = True
        self.measured['source_catalog'] = [{'package': 'example', 'status': 'ok'}]
        self.measured['source_artifacts'] = [
            {'package': 'example', 'kind': 'orig', 'bytes': 300_000_000},
            {'package': 'example', 'kind': 'dsc', 'bytes': 1_000_000}]
        upstream = history_tool.simulate(self.history, self.measured, self.scenario)
        self.history['events'] = [self.history['events'][-1]]
        local = history_tool.simulate(self.history, self.measured, self.scenario)
        self.assertAlmostEqual(upstream['daily'][0]['upload_gb_proxy'], 1.501)
        self.assertAlmostEqual(local['daily'][0]['upload_gb_proxy'], 1.201)

    def test_rejects_invalid_projection_and_financial_inputs(self):
        for days in [0, -1]:
            with self.subTest(days=days), self.assertRaises(ValueError):
                history_tool.simulate(self.history, self.measured, self.scenario, days)
        for key, value in [('compute_per_hour', -1), ('compute_per_hour', float('nan')),
                           ('validation_seconds', float('inf')), ('billing_days_per_month', 0)]:
            scenario = copy.deepcopy(self.scenario)
            scenario[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                history_tool.simulate(self.history, self.measured, scenario)

    def test_missing_artifact_size_marks_partial_cost(self):
        self.measured['artifacts'] = []
        result = history_tool.simulate(self.history, self.measured, self.scenario)
        self.assertTrue(result['is_partial'])
        self.assertTrue(any('size' in w.lower() for w in result['warnings']))

    def test_local_calendar_boundaries(self):
        with tempfile.TemporaryDirectory() as folder:
            args = argparse.Namespace(timezone='Asia/Bangkok', end_date='2026-11-01', days=1, cache=folder)
            collector = history_tool.Collector(args)
            recipe = {'package': 'example'}
            make = lambda stamp: collector.event(recipe, 'release', 'v1', stamp, 'https://example.test', 'test')
            self.assertIsNone(make('2026-10-31T16:59:59Z'))
            self.assertEqual(make('2026-10-31T17:00:00Z')['day'], '2026-11-01')
            self.assertIsNone(make('2026-11-01T17:00:00Z'))


if __name__ == '__main__':
    unittest.main()
