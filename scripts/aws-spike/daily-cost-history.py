#!/usr/bin/env python3
"""Collect upstream/repo activity once; simulate daily build costs offline.

No command starts EC2, builds packages, or publishes artifacts. Collection uses
read-only GitHub GraphQL (gh), public package APIs, and local git history.
"""
import argparse
import concurrent.futures
import csv
from datetime import date, datetime, time, timedelta, timezone
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import sys
import urllib.parse
import urllib.request
from zoneinfo import ZoneInfo

ROOT = Path(__file__).resolve().parents[2]
VERSION = 1
SNAPSHOTS = {'libvgm': 'ValleyBell/libvgm', 'xemu': 'lgblgblgb/xemu', 'drmon': 'developer-resources-co/drdevtools'}
PYPI = {f'python3-{x}': x for x in ['glfw', 'inators', 'librosa', 'mss', 'picire', 'pydub']}

def dump(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + '\n')

def utcnow():
    return datetime.now(timezone.utc).isoformat()

def parse_time(value):
    return datetime.fromisoformat(value.replace('Z', '+00:00'))

def stable(tag, package):
    patterns = {'ghidra': r'Ghidra_\d+(?:\.\d+)+_build', 'vgmstream': r'r\d+',
                'blender-asset-finder': r'blender-asset-finder-v?\d+(?:\.\d+)*',
                'blender-asset-finder-cli': r'wf-asset-v?\d+(?:\.\d+)*'}
    return bool(re.fullmatch(patterns.get(package, r'[vV]?\d+(?:\.\d+)*'), tag))

def discover(repo):
    recipes = []
    for folder in sorted((repo / 'foundry-apt/packages').iterdir()):
        if not folder.is_dir() or (folder / '.shared-electron-only').exists():
            continue
        watch = folder / 'debian/watch'
        build = folder / 'build.sh'
        content = watch.read_text() if watch.exists() else ''
        urls = re.findall(r'https://github.com/([\w.-]+/[\w.-]+)', content)
        item = {'package': folder.name, 'watch_path': str(watch.relative_to(repo)) if watch.exists() else None,
                'watch_sha256': hashlib.sha256(content.encode()).hexdigest(), 'stream': 'release'}
        if folder.name in PYPI:
            item.update(provider='pypi', source=PYPI[folder.name])
        elif folder.name in SNAPSHOTS:
            item.update(provider='github', source=SNAPSHOTS[folder.name], stream='snapshot')
        elif urls:
            item.update(provider='github', source=urls[0])
        elif folder.name == 'powerdevil':
            item.update(provider='github', source='KDE/powerdevil', date_note='KDE mirror tags; commit/tag dates are not release publication dates')
        elif folder.name == 'bsnes-jg':
            item.update(provider='gitlab', source='jgemu/bsnes')
        else:
            item.update(provider='local', source='repository commits')
        recipes.append(item)
    return recipes

class Collector:
    def __init__(self, args):
        self.args = args
        self.tz = ZoneInfo(args.timezone)
        self.end_day = date.fromisoformat(args.end_date)
        self.start_day = self.end_day - timedelta(days=args.days - 1)
        self.start = datetime.combine(self.start_day, time.min, self.tz)
        self.end = datetime.combine(self.end_day + timedelta(days=1), time.min, self.tz)
        self.cache = Path(args.cache)
        self.cache.mkdir(parents=True, exist_ok=True)

    def request(self, kind, payload):
        key = hashlib.sha256((kind + '\n' + payload).encode()).hexdigest()
        path = self.cache / f'{key}.json'
        if path.exists() and not self.args.refresh:
            receipt = json.loads(path.read_text())
            age = (datetime.now(timezone.utc) - parse_time(receipt['retrieved_at'])).total_seconds()
            if 0 <= age <= self.args.cache_max_age_hours * 3600:
                if receipt.get('error'):
                    raise RuntimeError(receipt['error'])
                return receipt['response'], str(path)
        receipt = {'kind': kind, 'request': payload, 'retrieved_at': utcnow()}
        try:
            if kind == 'graphql':
                result = subprocess.run(['gh', 'api', 'graphql', '-f', 'query=' + payload], text=True, capture_output=True)
                if result.returncode:
                    raise RuntimeError(result.stderr.strip())
                data = json.loads(result.stdout)
                if data.get('errors'):
                    raise RuntimeError(json.dumps(data['errors']))
            else:
                req = urllib.request.Request(payload, headers={'User-Agent': 'Foundry-cost-history/1'})
                with urllib.request.urlopen(req, timeout=45) as response:
                    data = json.load(response)
            receipt['response'] = data
        except Exception as error:
            receipt['error'] = str(error)
            dump(path, receipt)
            raise
        dump(path, receipt)
        return data, str(path)

    def event(self, recipe, kind, version, stamp, url, basis, **extra):
        instant = parse_time(stamp)
        if not self.start <= instant < self.end:
            return None
        return {'package': recipe['package'], 'kind': kind, 'version': version,
                'timestamp': instant.isoformat(), 'day': instant.astimezone(self.tz).date().isoformat(),
                'url': url, 'date_basis': basis, **extra}

    def github(self, recipe):
        owner, name = recipe['source'].split('/')
        events, refs, receipts = [], {}, []
        stream = recipe['stream']
        curs = None
        for page in range(self.args.max_pages):
            after = ',after:' + json.dumps(curs) if curs else ''
            if stream == 'snapshot':
                inner = 'defaultBranchRef { target { ... on Commit { history(first:100' + after + ',since:' + json.dumps(self.start.isoformat()) + ',until:' + json.dumps(self.end.isoformat()) + ') { nodes { oid committedDate url } pageInfo { hasNextPage endCursor } } } } }'
            else:
                inner = 'releases(first:100' + after + ',orderBy:{field:CREATED_AT,direction:DESC}) { nodes { tagName publishedAt isPrerelease isDraft url } pageInfo { hasNextPage endCursor } }'
            query = 'query { repository(owner:' + json.dumps(owner) + ',name:' + json.dumps(name) + ') { ' + inner + ' } }'
            data, receipt = self.request('graphql', query); receipts.append(receipt)
            repo = data['data']['repository']
            conn = repo['defaultBranchRef']['target']['history'] if stream == 'snapshot' else repo['releases']
            stamps = []
            for node in conn['nodes']:
                if stream == 'snapshot':
                    stamp = node['committedDate']; version = node['oid']
                    event = self.event(recipe, 'snapshot', version, stamp, node['url'], 'default-branch commit timestamp')
                else:
                    refs[node['tagName']] = node
                    stamp = node['publishedAt']
                    if not stamp or node['isDraft'] or node['isPrerelease'] or not stable(node['tagName'], recipe['package']):
                        continue
                    version = node['tagName']
                    event = self.event(recipe, 'release', version, stamp, node['url'], 'release publishedAt')
                if stamp: stamps.append(parse_time(stamp))
                if event: events.append(event)
            if not conn['pageInfo']['hasNextPage'] or (stamps and max(stamps) < self.start):
                break
            curs = conn['pageInfo']['endCursor']
        else:
            raise RuntimeError('History pagination limit reached; increase --max-pages')
        # A release feed can omit tags. Capture matching tags separately with an
        # explicit proxy-date label; simulation selects whether to include them.
        if stream != 'snapshot':
            curs = None
            for page in range(self.args.max_pages):
                after = ',after:' + json.dumps(curs) if curs else ''
                query = 'query { repository(owner:' + json.dumps(owner) + ',name:' + json.dumps(name) + ') { refs(refPrefix:"refs/tags/",first:100' + after + ',orderBy:{field:TAG_COMMIT_DATE,direction:DESC}) { nodes { name target { ... on Commit { committedDate oid } ... on Tag { tagger { date } target { ... on Commit { committedDate oid } } } } } pageInfo { hasNextPage endCursor } } } }'
                data, receipt = self.request('graphql', query); receipts.append(receipt)
                conn = data['data']['repository']['refs']; stamps = []
                for node in conn['nodes']:
                    target = node['target']; commit = target.get('target', target)
                    stamp = (target.get('tagger') or {}).get('date') or commit.get('committedDate')
                    if stamp: stamps.append(parse_time(stamp))
                    if not stamp or node['name'] in refs or not stable(node['name'], recipe['package']):continue
                    event = self.event(recipe, 'tag_proxy', node['name'], stamp, f'https://github.com/{recipe["source"]}/releases/tag/{node["name"]}', 'annotated-tag date or target commit date; NOT publication date')
                    if event: events.append(event)
                if not conn['pageInfo']['hasNextPage'] or (stamps and max(stamps) < self.start):break
                curs = conn['pageInfo']['endCursor']
            else:raise RuntimeError('Tag pagination limit reached; increase --max-pages')
        return events, receipts

    def external(self, recipe):
        if recipe['provider'] == 'github':return self.github(recipe)
        if recipe['provider'] == 'pypi':
            url = f'https://pypi.org/pypi/{recipe["source"]}/json'
            data, receipt = self.request('http', url); events = []
            for version, files in data['releases'].items():
                stamps = [x['upload_time_iso_8601'] for x in files if not x.get('yanked')]
                if not stamps or not stable(version, recipe['package']):continue
                event = self.event(recipe, 'release', version, min(stamps), f'https://pypi.org/project/{recipe["source"]}/{version}/', 'first non-yanked file upload')
                if event:events.append(event)
            return events, [receipt]
        events, receipts = [], []
        for page in range(1, self.args.max_pages + 1):
            url = 'https://gitlab.com/api/v4/projects/' + urllib.parse.quote(recipe['source'], safe='') + f'/repository/tags?per_page=100&page={page}'
            data, receipt = self.request('http', url); receipts.append(receipt); stamps = []
            for node in data:
                stamp = node.get('created_at') or node['commit']['committed_date']; stamps.append(parse_time(stamp))
                if not stable(node['name'], recipe['package']):continue
                event = self.event(recipe, 'tag_proxy', node['name'], stamp, f'https://gitlab.com/{recipe["source"]}/-/tags/{node["name"]}', 'GitLab tag creation or target commit date; publication unverified')
                if event:events.append(event)
            if len(data) < 100 or (stamps and max(stamps) < self.start):break
        else:raise RuntimeError('GitLab pagination limit reached')
        return events, receipts

    def collect_one(self, recipe):
        coverage = {**recipe, 'status': 'ok', 'receipts': []}
        if recipe['provider'] == 'local':
            coverage['status'] = 'local-only'
            return [], coverage
        try:
            events, receipts = self.external(recipe)
            coverage.update(receipts=receipts, event_count=len(events))
            return events, coverage
        except Exception as error:
            coverage.update(status='unknown', error=str(error))
            return [], coverage

    def run(self):
        recipes = discover(Path(self.args.repo))
        events, coverage = [], []
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.args.workers) as pool:
            for ev, cov in pool.map(self.collect_one, recipes):
                events.extend(ev); coverage.append(cov)
                print(f'{cov["package"]}: {cov["status"]}, {len(ev)} events', file=sys.stderr, flush=True)
        git = ['git', '-C', self.args.repo]
        commits = subprocess.check_output(git + ['log', '--since=' + self.start.isoformat(), '--until=' + self.end.isoformat(), '--format=%H', '--', 'foundry-apt/packages'], text=True).splitlines()
        for oid in commits:
            stamp, subject = subprocess.check_output(git + ['show', '-s', '--format=%cI%n%s', oid], text=True).splitlines()[:2]
            files = subprocess.check_output(git + ['diff-tree', '--no-commit-id', '--name-only', '-r', oid], text=True).splitlines()
            packages = sorted({p.split('/')[2] for p in files if p.startswith('foundry-apt/packages/') and len(p.split('/')) > 3})
            for pkg in packages:
                event = self.event({'package': pkg}, 'repo_change', oid, stamp, f'https://github.com/foundry-linux/foundrylinux.org/commit/{oid}', 'git committer timestamp', subject=subject)
                if event:events.append(event)
        result = {'schema_version': VERSION, 'collected_at': utcnow(), 'window': {'start_date': self.start_day.isoformat(), 'end_date': self.end_day.isoformat(), 'days': self.args.days, 'timezone': self.args.timezone},
                  'repo_head': subprocess.check_output(git + ['rev-parse', 'HEAD'], text=True).strip(),
                  'policy': 'Stable releases preferred; unmatched dated tags retained as explicit proxies. Snapshot commits are separate. Coalescing is a simulation decision. Only Foundry overlay recipes, not all Ubuntu dependencies. Dates grouped into local calendar days; not a replay of 03:00 scheduling.',
                  'coverage': coverage, 'events': sorted(events, key=lambda e: (e['timestamp'], e['package'], e['kind']))}
        dump(self.args.output, result)
        print(f'Saved {len(events)} events to {self.args.output}')

def import_sizes(measured, repo, dist):
    """Attach source artifact sizes from verified local DSCs matching recipe versions."""
    source_artifacts, catalog = [], []
    for recipe in discover(repo):
        package = recipe['package']
        changelog = repo / 'foundry-apt/packages' / package / 'debian/changelog'
        version = re.search(r'^\S+ \(([^)]+)\)', changelog.read_text()).group(1)
        matches = []
        for dsc in dist.glob(package + '_*.dsc'):
            text = dsc.read_text()
            found = re.search(r'^Version: (.+)$', text, re.M)
            if found and found.group(1) == version:matches.append(dsc)
        record = {'package': package, 'version': version, 'status': 'unknown'}
        if len(matches) != 1:
            record['error'] = 'Expected exactly one matching DSC'
            catalog.append(record);continue
        dsc = matches[0]; text = dsc.read_text()
        section = re.search(r'^Checksums-Sha256:\n((?: .+\n)+)', text, re.M)
        if not section:
            record['error'] = 'DSC lacks SHA256 file inventory';catalog.append(record);continue
        files = [{'file': dsc.name, 'bytes': dsc.stat().st_size,
                  'sha256': hashlib.sha256(dsc.read_bytes()).hexdigest(), 'kind': 'dsc', 'package': package}]
        try:
            for line in section.group(1).splitlines():
                expected, size, name = line.split()
                if Path(name).name != name:raise ValueError('Unsafe source artifact path')
                path = dist / name
                with path.open('rb') as stream:actual = hashlib.file_digest(stream, 'sha256').hexdigest()
                if path.stat().st_size != int(size) or actual != expected:raise ValueError('Source artifact checksum/size mismatch: ' + name)
                files.append({'file': name, 'bytes': int(size), 'sha256': actual,
                              'kind': 'orig' if '.orig.tar.' in name else 'source', 'package': package})
        except Exception as error:
            record['error'] = str(error);catalog.append(record);continue
        record.update(status='ok', dsc=dsc.name, files=len(files))
        source_artifacts.extend(files);catalog.append(record)
    return {**measured, 'source_artifacts': source_artifacts, 'source_catalog': catalog,
            'source_size_provenance': {'collected_at': utcnow(), 'dist': str(dist), 'repo': str(repo),
              'method': 'Match current recipe changelog version to DSC; verify every SHA256 and byte size. Local files proxy future source sizes, not AWS-timed artifacts.'}}


def selected_events(history, scenario):
    kinds = {'release'}
    if scenario.get('include_tag_proxies', True):kinds.add('tag_proxy')
    if scenario.get('include_snapshots', True):kinds.add('snapshot')
    mode = scenario.get('activity', 'both')
    if mode == 'repo':kinds = {'repo_change'}
    elif mode == 'both':kinds.add('repo_change')
    elif mode != 'upstream':raise ValueError('activity must be upstream, repo, or both')
    prefixes = scenario.get('upstream_version_prefixes', {})
    return [e for e in history['events'] if e['kind'] in kinds and
            (e['kind'] == 'repo_change' or e['package'] not in prefixes or
             e['version'].startswith(prefixes[e['package']]))]

def validate_number(name, value, positive=False):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0 or (positive and value == 0):
        raise ValueError(f'{name} must be a finite ' + ('positive' if positive else 'nonnegative') + ' number')


def simulate(history, measured, scenario, projection_days=None):
    days = history['window']['days']
    validate_number('history days', days, positive=True)
    if not isinstance(days, int):raise ValueError('history days must be an integer')
    actual_days = (date.fromisoformat(history['window']['end_date']) - date.fromisoformat(history['window']['start_date'])).days + 1
    if actual_days != days:raise ValueError('history window dates do not match its day count')
    if projection_days is not None:
        validate_number('projection days', projection_days, positive=True)
        if not isinstance(projection_days, int):raise ValueError('projection days must be an integer')
    required = ['compute_per_hour', 'ipv4_per_hour', 'ebs_gib', 'ebs_per_gib_month', 'warm_workflow_seconds', 'discovery_seconds', 'startup_seconds', 'validation_seconds', 'publication_seconds', 'egress_per_gb', 'storage_per_gb_month']
    for key in required:validate_number(key, scenario[key])
    for key, default in [('billing_days_per_month',30), ('artifact_retention_days',30), ('egress_bytes_per_gb',1000000000), ('storage_bytes_per_gb',1000000000)]:validate_number(key, scenario.get(key,default), positive=True)
    for key, default in [('existing_storage_gb',0), ('package_upload_multiplier',1), ('available_egress_free_gb_per_calendar_month',0)]:validate_number(key, scenario.get(key,default))
    for item in measured['markers']:validate_number('recipe seconds', item['seconds'])
    for item in measured['artifacts']:validate_number('artifact bytes', item['bytes'])
    for item in measured.get('source_artifacts', []):validate_number('source artifact bytes', item['bytes'])
    validate_number('ISO bytes', measured['iso_sizes']['warm'])
    for value in scenario.get('recipe_seconds_override', {}).values():validate_number('recipe override seconds', value)
    for key, default in [('build_compute_per_hour',scenario['compute_per_hour']),('build_ipv4_per_hour',scenario['ipv4_per_hour'])]:validate_number(key, scenario.get(key,default))
    validate_number('build runtime multiplier', scenario.get('build_runtime_multiplier',1), positive=True)
    if scenario.get('iso_policy', 'daily') not in ('daily', 'changed'):
        raise ValueError('iso_policy must be daily or changed')
    if scenario.get('retention_policy', 'rolling') not in ('rolling', 'latest'):
        raise ValueError('retention_policy must be rolling or latest')
    timings = {x['package']: x['seconds'] for x in measured['markers']}
    timings.update(scenario.get('recipe_seconds_override', {}))
    bytes_by_pkg = {}
    for artifact in measured['artifacts']:
        # debug symbols belong to the same recipe; multi-binary mappings can be supplied.
        pkg = scenario.get('binary_to_recipe', {}).get(artifact['package'], artifact['package'].removesuffix('-dbgsym'))
        bytes_by_pkg[pkg] = bytes_by_pkg.get(pkg, 0) + artifact['bytes']
    source_bytes = {}; orig_bytes = {}
    for artifact in measured.get('source_artifacts', []):
        dest = orig_bytes if artifact['kind'] == 'orig' else source_bytes
        dest[artifact['package']] = dest.get(artifact['package'], 0) + artifact['bytes']
    source_known = {x['package'] for x in measured.get('source_catalog', []) if x['status'] == 'ok'}
    grouped = {}; upstream_groups = {}
    for event in selected_events(history, scenario):
        grouped.setdefault(event['day'], set()).add(event['package'])
        if event['kind'] != 'repo_change':upstream_groups.setdefault(event['day'], set()).add(event['package'])
    count = days if projection_days is None else projection_days
    if count < 1:raise ValueError('projection days must be positive')
    start = date.fromisoformat(history['window']['start_date'])
    output_start = date.fromisoformat(scenario.get('projection_start', history['window']['start_date']))
    validate_number('public download GB/day', scenario.get('public_download_gb_per_day', 0))
    validate_number('public download rate', scenario.get('public_download_per_gb', 0))
    cpu_rate = scenario['compute_per_hour'] + scenario['ipv4_per_hour']
    build_rate = scenario.get('build_compute_per_hour',scenario['compute_per_hour']) + scenario.get('build_ipv4_per_hour',scenario['ipv4_per_hour'])
    ebs_daily = scenario['ebs_gib'] * scenario['ebs_per_gib_month'] / scenario.get('billing_days_per_month', 30)
    iso_bytes = measured['iso_sizes']['warm']
    retained, rows, warnings = [], [], []
    missing = set(); missing_sizes = set(); missing_sources = set()
    allowances = {}
    for i in range(count):
        source_day = start + timedelta(days=i % days)
        day = output_start + timedelta(days=i)
        packages = sorted(grouped.get(source_day.isoformat(), set()))
        unknown = [p for p in packages if p not in timings]; missing.update(unknown)
        unknown_sizes = [p for p in packages if p not in bytes_by_pkg]; missing_sizes.update(unknown_sizes)
        unknown_sources = [p for p in packages if p not in source_known] if scenario.get('include_source_uploads', False) else []
        missing_sources.update(unknown_sources)
        build_seconds = sum(timings[p] for p in packages if p in timings)
        iso = scenario.get('iso_policy', 'daily') == 'daily' or bool(packages)
        baseline_build_seconds = (scenario['warm_workflow_seconds'] if iso else 0) + build_seconds
        modeled_build_seconds = baseline_build_seconds * scenario.get('build_runtime_multiplier',1)
        other_seconds = scenario['discovery_seconds']
        # Validation/control stages retain their separate host rate.
        if iso:other_seconds += scenario['startup_seconds'] + scenario['validation_seconds'] + scenario['publication_seconds']
        seconds = modeled_build_seconds + other_seconds
        package_bytes = sum(bytes_by_pkg.get(p, 0) for p in packages) * scenario.get('package_upload_multiplier', 1)
        source_upload = 0
        if scenario.get('include_source_uploads', False):
            upstream_packages = upstream_groups.get(source_day.isoformat(), set())
            source_upload = sum(source_bytes.get(p, 0) + (orig_bytes.get(p, 0) if p in upstream_packages else 0) for p in packages)
        upload_bytes = (iso_bytes if iso else 0) + package_bytes + source_upload
        upload_gb = upload_bytes / scenario.get('egress_bytes_per_gb', 1e9)
        month = day.strftime('%Y-%m')
        allowance = allowances.setdefault(month, scenario.get('available_egress_free_gb_per_calendar_month', 0))
        download_gb = scenario.get('public_download_gb_per_day', 0)
        free = min(allowance, download_gb); allowances[month] -= free
        transfer = upload_gb * scenario['egress_per_gb'] + (download_gb - free) * scenario.get('public_download_per_gb', 0)
        retained.append((i, upload_bytes))
        retained = [(j, size) for j, size in retained if i - j < scenario['artifact_retention_days']]
        retained_bytes = sum(size for _, size in retained)
        if scenario.get('retention_policy', 'rolling') == 'latest':
            # Current catalog sizes proxy replacements; unchanged recipes stay available.
            retained_bytes = iso_bytes + sum(bytes_by_pkg.values())
            if scenario.get('include_source_uploads', False):
                retained_bytes += sum(source_bytes.values()) + sum(orig_bytes.values())
        storage_gb = retained_bytes / scenario.get('storage_bytes_per_gb', 1e9) + scenario.get('existing_storage_gb', 0)
        storage_cost = storage_gb * scenario['storage_per_gb_month'] / scenario.get('billing_days_per_month', 30)
        build_compute = modeled_build_seconds / 3600 * build_rate
        other_compute = other_seconds / 3600 * cpu_rate
        compute = build_compute + other_compute
        rows.append({'date': day.isoformat(), 'history_day': source_day.isoformat(), 'packages': packages, 'package_build_seconds_proxy': build_seconds, 'unknown_timing_packages': unknown, 'unknown_size_packages': unknown_sizes, 'unknown_source_size_packages': unknown_sources,
                     'runner_seconds_model': seconds, 'build_compute_usd': build_compute, 'other_compute_usd': other_compute, 'compute_usd': compute, 'ebs_usd': ebs_daily, 'upload_gb_proxy': upload_gb, 'egress_usd': transfer, 'artifact_retained_gb_model': storage_gb, 'artifact_storage_usd': storage_cost,
                     'total_usd': compute + ebs_daily + transfer + storage_cost})
    gaps = [x['package'] for x in history['coverage'] if x['status'] == 'unknown']
    if gaps:warnings.append('Unknown upstream coverage (not zero activity): ' + ', '.join(gaps))
    if missing:warnings.append('Unpriced rebuilds due to missing timing: ' + ', '.join(sorted(missing)))
    if missing_sources:warnings.append('Unpriced source transfers/storage due to missing verified source sizes: ' + ', '.join(sorted(missing_sources)))
    if missing_sizes:warnings.append('Unpriced package transfers/storage due to missing artifact sizes: ' + ', '.join(sorted(missing_sizes)))
    warnings += ['Current package catalog is projected backward; deleted releases and historical promotion eligibility cannot be reconstructed from current feeds.', 'Per-recipe cold intervals are proxies for future changed-source builds, not measured historical runtimes.', 'Current artifact sizes proxy historical uploads. Verified local source sizes are included only when include_source_uploads is enabled; metadata indexes are not sized.', 'Ubuntu archive updates are outside overlay activity coverage; daily ISO still refreshes dependencies.', 'Validation/startup/publication/discovery inputs are explicit scenario allowances, not measured automated stage durations.', 'No automatic CPU-count timing scaling; hardware changes require explicit measured timing overrides.', 'Shared account free allowances are zero by default; rolling retention starts empty; latest retention includes one ISO and the current measured package/source catalog. existing_storage_gb adds other retained content. Requests, taxes, failures, signing/monitoring services and website operations are excluded.']
    totals = {key: sum(row[key] for row in rows) for key in ['build_compute_usd', 'other_compute_usd', 'compute_usd', 'ebs_usd', 'egress_usd', 'artifact_storage_usd', 'total_usd', 'upload_gb_proxy', 'package_build_seconds_proxy']}
    totals.update(days=count, average_daily_usd=totals['total_usd']/count, package_builds=sum(len(x['packages']) for x in rows), active_package_days=sum(bool(x['packages']) for x in rows))
    return {'schema_version': VERSION, 'is_partial': bool(gaps or missing or missing_sizes or missing_sources), 'generated_at': utcnow(), 'history_window': history['window'], 'projection': 'Repeat the observed daily sequence cyclically; not a release forecast' if count != days else 'Historical activity replay with current measured timing proxies', 'scenario': scenario, 'warnings': warnings, 'totals': totals, 'daily': rows}

def write_report(result, output):
    out = Path(output); out.mkdir(parents=True, exist_ok=True)
    dump(out / 'simulation.json', result)
    columns = list(result['daily'][0])
    with (out/'daily.csv').open('w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=columns, lineterminator="\n"); writer.writeheader()
        for row in result['daily']:
            writer.writerow({k: ', '.join(v) if isinstance(v,list) else v for k,v in row.items()})
    t = result['totals']
    total_label = 'Partial known-cost subtotal (unknown components unpriced)' if result['is_partial'] else 'Modeled total'
    lines = ['# Daily Foundry build cost simulation', '', f'**{total_label}: ${t["total_usd"]:.2f} over {t["days"]} days; average ${t["average_daily_usd"]:.3f}/day.**', '', result['projection'], '',
             '| Component | USD |', '|---|---:|']
    for key, name in [('compute_usd','Compute + IPv4'),('ebs_usd','Retained EBS'),('egress_usd','Publication/download transfer'),('artifact_storage_usd','S3 artifact storage')]:lines.append(f'| {name} | {t[key]:.3f} |')
    lines += ['', '## Assumptions and coverage', ''] + ['- '+w for w in result['warnings']]
    lines += ['', '## Per-day calculation', '', '| Date | Packages to rebuild (coalesced) | Build proxy min | Compute | EBS | Egress | S3 | Total USD |', '|---|---|---:|---:|---:|---:|---:|---:|']
    for row in result['daily']:
        packages=', '.join(row['packages']) or '—'
        lines.append(f'| {row["date"]} | {packages} | {row["package_build_seconds_proxy"]/60:.2f} | {row["compute_usd"]:.3f} | {row["ebs_usd"]:.3f} | {row["egress_usd"]:.3f} | {row["artifact_storage_usd"]:.3f} | **{row["total_usd"]:.3f}** |')
    (out/'report.md').write_text('\n'.join(lines)+'\n')

def main():
    p=argparse.ArgumentParser(description=__doc__)
    sub=p.add_subparsers(dest='command',required=True)
    c=sub.add_parser('collect',help='Read upstream APIs and local git; save reusable normalized JSON')
    c.add_argument('--repo',default=str(ROOT));c.add_argument('--days',type=int,default=30)
    c.add_argument('--end-date',required=True,help='Inclusive last local calendar day, YYYY-MM-DD')
    c.add_argument('--timezone',default='Asia/Bangkok');c.add_argument('--cache',required=True)
    c.add_argument('--output',required=True);c.add_argument('--refresh',action='store_true')
    c.add_argument('--workers',type=int,default=4);c.add_argument('--max-pages',type=int,default=20)
    c.add_argument('--cache-max-age-hours',type=float,default=24,help='Refresh cached API evidence older than this; default 24 hours')
    z=sub.add_parser('import-sizes',help='Attach verified local source artifact sizes to saved measurements')
    z.add_argument('--measurements',required=True);z.add_argument('--repo',default=str(ROOT));z.add_argument('--dist',required=True);z.add_argument('--output',required=True)
    s=sub.add_parser('simulate',aliases=['report'],help='Offline cost replay/projection; does not access APIs')
    s.add_argument('--history',required=True);s.add_argument('--measurements',required=True)
    s.add_argument('--scenario',required=True);s.add_argument('--output',required=True)
    s.add_argument('--projection-days',type=int)
    args=p.parse_args()
    if args.command=='collect':
        if args.days<1:p.error('--days must be positive')
        if args.workers<1 or args.max_pages<1:p.error('--workers and --max-pages must be positive')
        validate_number('cache max age hours', args.cache_max_age_hours)
        Collector(args).run()
    elif args.command == 'import-sizes':
        measured=json.loads(Path(args.measurements).read_text())
        result=import_sizes(measured, Path(args.repo), Path(args.dist))
        dump(args.output,result)
        print(json.dumps({'sources':len(result['source_catalog']), 'unknown':[x for x in result['source_catalog'] if x['status']!='ok']},indent=2))
    else:
        inputs=[json.loads(Path(getattr(args,k)).read_text()) for k in ['history','measurements','scenario']]
        result=simulate(*inputs,args.projection_days);write_report(result,args.output)
        print(json.dumps(result['totals'],indent=2))
if __name__=='__main__':main()
