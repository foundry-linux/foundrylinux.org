// /packages — full catalogue, generated against the live apt indexes.
// See docs/plans/2026-05-21-packages-page.md §3.
import React from 'react';
import {
  Topbar, Foot, formatSize, findCategory, ICONS,
} from './sections.jsx';
import { ArrowRightIcon, SparksIcon } from './icons.jsx';
import packagesData from './packages-data.json';

function originBadge(origin) {
  // Short, kebab-case origin labels with semantic colour via data-origin.
  const label = {
    'foundry':            'foundry',
    'worldfoundry':       'worldfoundry',
    'ubuntu-universe':    'universe',
    'ubuntu-multiverse':  'multiverse',
    'ubuntu-main':        'main',
    'unknown':            '?',
  }[origin] || origin;
  return <span className="pkg-origin-badge" data-origin={origin}>{label}</span>;
}

function upstreamBadge(upstream) {
  const status = upstream?.status || 'unknown';
  const labels = {
    'vendored':              'vendored',
    'debian-itp':            upstream.bug ? `ITP #${upstream.bug}` : 'ITP',
    'in-debian-unstable':    'in Debian unstable',
    'in-ubuntu-universe':    'universe',
    'in-ubuntu-multiverse':  'multiverse',
    'in-ubuntu-main':        'main',
    'n/a-firstparty':        'first-party',
    'n/a-proprietary':       'proprietary',
    'unknown':               '?',
  };
  return <span className="pkg-upstream-badge" data-upstream={status}>{labels[status] || status}</span>;
}

function EditionsLadder() {
  return (
    <section className="section" id="editions-ladder">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">Editions</span>
            <h2 className="section-title">Three nested casts.</h2>
          </div>
          <p className="section-blurb">
            Atelier pulls in Sprite. Sprite pulls in Anvil. Pick the largest
            one you want struck in from first boot.
          </p>
        </div>

        <div className="editions">
          {packagesData.editions.map((ed) => (
            <div className="edition" key={ed.slug} data-flavor={ed.slug}>
              <span className="edition-tag">Edition · 26.04</span>
              <h3 className="edition-name">{ed.title}</h3>
              <p className="edition-desc">{ed.blurb}</p>
              <pre className="edition-install"><code>{ed.install_command}</code></pre>
              <div className="edition-pkg">
                <span>{ed.metapackage}</span>
                <span className="size">
                  {ed.package_count}&nbsp;pkgs · {formatSize(ed.installed_size_kb)}
                </span>
              </div>
            </div>
          ))}
        </div>
      </div>
    </section>
  );
}

function CategorySection({ cat, index }) {
  const Icon = ICONS[cat.icon] || SparksIcon;
  return (
    <section className="section pkg-cat" id={cat.slug}>
      <div className="shell">
        <div className="pkg-cat-head">
          <div className="pkg-cat-icon"><Icon size={56} /></div>
          <div className="pkg-cat-headtext">
            <span className="section-num">№ {String(index).padStart(2, '0')} · {cat.role}</span>
            <h2 className="section-title">{cat.title}</h2>
            <p className="pkg-cat-blurb">{cat.blurb}</p>
            <div className="pkg-cat-meta">
              <code>sudo apt install {cat.metapackages.join(' ')}</code>
              <span>{cat.package_count}&nbsp;pkgs · {formatSize(cat.installed_size_kb)} · {cat.in_edition_tier}</span>
            </div>
          </div>
        </div>

        <table className="pkg-table">
          <thead>
            <tr>
              <th>Package</th>
              <th>Origin</th>
              <th className="num">Installed size</th>
              <th>Upstream</th>
              <th>Summary</th>
            </tr>
          </thead>
          <tbody>
            {cat.packages.map((p) => (
              <tr key={p.name}>
                <td className="pkg-name"><code>{p.name}</code></td>
                <td>{originBadge(p.origin)}</td>
                <td className="num">{formatSize(p.installed_size_kb)}</td>
                <td>{upstreamBadge(p.upstream)}</td>
                <td className="pkg-summary">{p.summary}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </section>
  );
}

function VendoredStandalones() {
  return (
    <section className="section pkg-cat" id="vendored">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">Vendored standalones</span>
            <h2 className="section-title">What we build ourselves.</h2>
          </div>
          <p className="section-blurb">
            Packages we ship because Ubuntu doesn&rsquo;t. Each one is pulled
            in transitively by one of the categories above; this list is the
            at-a-glance upstreaming surface.
          </p>
        </div>

        <ul className="vendored-list">
          {packagesData.vendored_standalones.map((v) => (
            <li key={v.name}>
              <code>{v.name}</code>
              <span className="vendored-pulled-by">
                pulled by {v.pulled_by.map(p => <code key={p}>{p}</code>).reduce((acc, x, i) => i === 0 ? [x] : [...acc, ', ', x], [])}
              </span>
              {upstreamBadge(v.upstream)}
            </li>
          ))}
        </ul>
      </div>
    </section>
  );
}

function Upstreaming() {
  const s = packagesData.upstream_summary;
  const itps   = packagesData.vendored_standalones.filter(v => v.upstream?.status === 'debian-itp');
  const vendoredNoItp = packagesData.vendored_standalones.filter(v => v.upstream?.status === 'vendored');

  return (
    <section className="section pkg-cat" id="upstreaming">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">Upstreaming</span>
            <h2 className="section-title">Push it to Debian.</h2>
          </div>
          <p className="section-blurb">
            Where the vendored tools end up. We file ITPs for the ones with
            a reasonable path to Debian inclusion; the rest stay in our
            archive until upstream stabilises.
          </p>
        </div>

        <div className="upstreaming-grid">
          <div className="upstreaming-block">
            <h3>Open Debian ITPs ({itps.length})</h3>
            {itps.length === 0 ? (
              <p className="dim">(none yet)</p>
            ) : (
              <ul>
                {itps.map(v => (
                  <li key={v.name}>
                    <code>{v.name}</code> — {upstreamBadge(v.upstream)}
                    {v.upstream.note ? ` — ${v.upstream.note}` : ''}
                  </li>
                ))}
              </ul>
            )}
          </div>

          <div className="upstreaming-block">
            <h3>Vendored, no ITP yet ({vendoredNoItp.length})</h3>
            <ul>
              {vendoredNoItp.map(v => (
                <li key={v.name}>
                  <code>{v.name}</code>
                  {v.upstream.note ? <> &mdash; <em>{v.upstream.note}</em></> : null}
                </li>
              ))}
            </ul>
          </div>

          <div className="upstreaming-block">
            <h3>Already in Ubuntu ({(s['in-ubuntu-universe'] || 0) + (s['in-ubuntu-multiverse'] || 0) + (s['in-ubuntu-main'] || 0)})</h3>
            <p className="dim">
              <b>{s['in-ubuntu-universe'] || 0}</b> universe ·{' '}
              <b>{s['in-ubuntu-multiverse'] || 0}</b> multiverse ·{' '}
              <b>{s['in-ubuntu-main'] || 0}</b> main
            </p>
          </div>
        </div>
      </div>
    </section>
  );
}

function Alignment() {
  const a = packagesData.audit;
  const empty = a.unowned_packages.length === 0 && a.missing_metapackages.length === 0;
  return (
    <section className="section pkg-cat" id="alignment">
      <div className="shell">
        <div className="section-head">
          <div>
            <span className="section-num">Alignment</span>
            <h2 className="section-title">Catalogue audit.</h2>
          </div>
          <p className="section-blurb">
            Self-check on the catalogue: every package in the apt index
            should be reached by exactly one category, every category should
            point at a metapackage that exists. {empty ? 'All clean.' : 'See below.'}
          </p>
        </div>

        <dl className="alignment-list">
          <dt>Unowned packages</dt>
          <dd>{a.unowned_packages.length === 0 ? <span className="dim">(none — every package is reached)</span> : a.unowned_packages.map(n => <code key={n}>{n}</code>)}</dd>

          <dt>Missing metapackages</dt>
          <dd>{a.missing_metapackages.length === 0 ? <span className="dim">(none — all declared metapackages exist in apt)</span> : a.missing_metapackages.map(n => <code key={n}>{n}</code>)}</dd>

          <dt>Out of catalogue</dt>
          <dd>
            <span className="dim">Intentionally not listed above (build-internal): </span>
            {a.out_of_catalogue.map(n => <code key={n}>{n}</code>)}
          </dd>
        </dl>
      </div>
    </section>
  );
}

function PackagesHero() {
  return (
    <section className="hero hero-pkg">
      <div className="shell">
        <div className="hero-eyebrow">
          /packages · regenerated {packagesData.generated_at}
        </div>
        <h1 className="wordmark wordmark-pkg" data-material="chrome" data-font="big-shoulders">
          <span className="wordmark-line">PACKAGES</span>
        </h1>
        <p className="hero-tagline">
          Everything Foundry&nbsp;Linux installs on top of Kubuntu&nbsp;26.04 —
          three nested editions, nine domain categories, six vendored
          standalones. Grand total{' '}
          <b>{formatSize(packagesData.grand_total_installed_size_kb)}</b>.
        </p>
        <p className="hero-tagline" style={{ marginTop: 16 }}>
          <a className="cta cta-secondary" href="/">
            <ArrowRightIcon /> Back to foundrylinux.org
          </a>
        </p>
      </div>
    </section>
  );
}

export function PackagesApp() {
  return (
    <>
      <Topbar />
      <main>
        <PackagesHero />
        <EditionsLadder />
        {packagesData.categories.map((cat, i) => (
          <CategorySection key={cat.slug} cat={cat} index={i + 1} />
        ))}
        <VendoredStandalones />
        <Upstreaming />
        <Alignment />
      </main>
      <Foot />
    </>
  );
}
