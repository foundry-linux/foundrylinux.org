# Depot & Blacksmith CI — investigation

*2026-05-23 · context: foundry-linux ISO build workload, current wbniv budget at 82%*

---

## Background

`wbniv` personal account has **hit the 2,000 min/month cap** this cycle. The
[2026-05-22-gh-actions-usage.md](2026-05-22-gh-actions-usage.md) report showed 1,643 min
in wall-clock time (82%), but that doc explicitly undercounts: parallel/matrix jobs each
contribute only one wall-time figure regardless of job fan-out, and macOS/Windows
multipliers (×10/×2) are ignored. Actual billed minutes exceeded 2,000.
The foundry-linux ISO build (live-build, 5–20 min per run inside a `ubuntu:26.04`
container) will add more minutes — the question is which account they land on and whether
extra capacity is worth paying for.

**First clarification needed:** are the `foundry-linux` org repos **public or private?**

- **Public repos → GitHub Actions is free, no caps.** Both Depot and Blacksmith are
  irrelevant for cost; the only value would be faster wall-clock time.
- **Private repos → free tier is 2,000 min/month** (shared with `wbniv` if under the
  same GitHub user plan; separate if under a paid GitHub org plan). The current 82%
  burn rate leaves ~360 min/cycle before hitting the cap.

---

## GitHub Actions pricing changes (March 2026)

GitHub introduced a **$0.002/min control-plane fee** for self-hosted runners on
March 1, 2026. Previously self-hosted compute was free to orchestrate; now GitHub
charges this fee regardless of where the job executes. Key points:

- **Public repos: unchanged — still completely free.**
- **GitHub-hosted runners:** per-minute rates dropped by up to 39%, offsetting the
  new platform fee. Net cost for most users is flat or lower.
- **Third-party runners (Blacksmith, Depot, etc.):** now incur the $0.002/min GitHub
  fee on top of the provider's rate. Providers have updated their pricing to absorb
  or reflect this.
- GitHub says 96% of customers see no bill change; of the 4% affected, 85% actually
  pay less.

---

## Depot

**What it is:** Two products bundled under one brand:

1. **Docker Build Cloud** — remote BuildKit builders with persistent layer cache.
   Accelerates `docker build` (and `docker run -v ... ubuntu:26.04 bash -c '...'`-style
   builds) by 10–40× for repeated builds via shared layer cache.
2. **GitHub Actions Runners** — managed x64/ARM cloud runners, billed by the second,
   ~3× faster than GitHub-hosted runners, with Docker layer cache baked in.

**Pricing (2026):**

| Plan | Monthly | GHA minutes included | Docker build min | Cache storage |
|---|---:|---:|---:|---:|
| Developer | $20 | 2,000 | 500 | 25 GB |
| Startup | $200 | 20,000 | 5,000 | 250 GB |
| Free trial | — | 7 days, no card | — | — |

Overage: $0.004/min for GHA runners · $0.04/min for Docker builds · $0.20/GB/month
cache. No one-minute billing minimum — tracked per second.

**No ongoing free tier.** Cheapest entry is $20/month.

**Integration:** Change `runs-on: ubuntu-latest` → `runs-on: depot-ubuntu-22.04`
(or `depot-ubuntu-24.04`). One-line change. Requires Depot project token in secrets.

**Relevance for foundry-linux:**

- The `.deb` packaging workflow wraps `dpkg-buildpackage` inside
  `docker run ubuntu:26.04 …`. Depot's Docker Build Cloud would *not* help here
  because we're running a container for isolation, not building a Docker image — the
  inner `dpkg-buildpackage` outputs `.deb` files, not image layers. Layer caching only
  helps `docker build` Dockerfile steps.
- The GHA runner upgrade (3× faster) would reduce wall-clock time for CPU-bound
  ISO builds, but at $20/month minimum with no free tier it's hard to justify for
  an early-stage distro project.
- **Verdict: not a good fit.** Docker Build Cloud doesn't accelerate our pattern.
  GHA runners cost $20/month minimum.

---

## Blacksmith

**What it is:** Managed GitHub Actions runner provider. Drop-in replacement for
`ubuntu-latest` runners. Faster hardware, persistent Docker layer cache optional add-on.
Positions itself as half the cost of GitHub for the same workload.

**Pricing (2026):**

| Runner type | Free/month | Rate after | vs GitHub |
|---|---:|---|---|
| Ubuntu x64 | 3,000 min | $0.004/min | −67% |
| Ubuntu ARM | 3,000 min | $0.0025/min | −75% |
| Windows x64 | 3,000 min | $0.008/min | −60% |
| macOS M4 | 3,000 min | $0.08/min | −35% |

Add-ons: Docker layer caching $0.50/GB/month · Sticky disks $0.50/GB/month ·
Static IPs $100/IP/month (irrelevant for builds).

**Open source program:** Blacksmith offers **free access for qualifying open-source
projects.** Foundry Linux (public repos, non-commercial distro) would almost certainly
qualify. Worth applying.

**Performance:** 50%+ higher single-core performance vs GitHub-hosted runners.
Ongoing fleet upgrades add another 15–25%. For CPU-bound workloads like
`dpkg-buildpackage` or `lb build`, this directly cuts wall-clock time.

**Integration:** Change `runs-on: ubuntu-latest` → `runs-on: blacksmith-2vcpu-ubuntu-22.04`
(or `…-ubuntu-24.04`; check if 26.04 image is available before adopting). One-line
change per workflow. No SDK, no project token — auth is via GitHub App install.

**Relevance for foundry-linux:**

- 3,000 free minutes/month covers ~150–600 ISO builds per month (at 5–20 min each)
  before any charges — plenty of headroom.
- Faster hardware cuts the ISO build from wherever it lands to ~60% of that time,
  meaning fewer minutes consumed per build even on the free tier.
- Docker layer caching add-on could accelerate the `docker run ubuntu:26.04` packaging
  step if we structure the container invocation to reuse layers, but the current pattern
  (running a pre-pulled image, not building one) doesn't benefit without rework.
- **Open source program = potentially $0/month with better performance.** This is the
  angle worth pursuing.
- **Caveat:** the new GitHub $0.002/min control-plane fee applies on top of Blacksmith's
  rate for *private* repos. For public repos it doesn't apply (free). This makes public
  repos even more clearly the right choice for the foundry-linux org.

---

## Other runner providers (brief)

| Provider | Free tier | Rate | Notes |
|---|---|---|---|
| **WarpBuild** | 3,000 min/mo | $0.004/min x64 | Similar to Blacksmith; ARM available |
| **Namespace.so** | 2,000 min/mo | $0.004/min | Strong caching story; newer |
| **Buildjet** | None | $0.0036/min | Cheaper rate, no free tier |
| **runs-on** | Self-hosted on your AWS | EC2 spot pricing | Full control; requires AWS account |

None of these have an open-source program as prominent as Blacksmith's.

---

## Recommendation

| Scenario | Recommendation |
|---|---|
| foundry-linux repos are **public** | Stay on GitHub Actions free tier. Apply to Blacksmith OSS program for faster hardware at no cost — one-line change. |
| foundry-linux repos are **private** | Blacksmith free tier (3,000 min/mo) covers the ISO workload. Apply to OSS program if eligible. |
| wbniv personal account is at cap | Fix the flagged wasted-minute workflows first (Lightsail rebake 52% fail rate, worldfoundry.org Deploy 90% fail rate) — that recovers ~400 wall-clock min/cycle before spending anything. |

**Depot is not the right tool here.** Its Docker Build Cloud accelerates Dockerfile
layer rebuilds; our pattern is running a pre-pulled `ubuntu:26.04` container for
build isolation, not building an image. The GHA runner product overlaps with Blacksmith
but costs $20/month minimum vs Blacksmith's free tier.

**Blacksmith is worth piloting** — specifically:

1. Check if Ubuntu 26.04 image is available (`blacksmith-2vcpu-ubuntu-26.04` or via
   custom image); if not, verify whether our builds work on 24.04 runners with the
   container wrapping the 26.04 step.
2. Apply to the [Blacksmith open-source program](https://www.blacksmith.sh/).
3. If accepted, swap `runs-on: ubuntu-latest` in `foundry-apt/.github/workflows/publish.yml`
   and `foundry-iso/.github/workflows/` (when it exists) — one-line change per workflow.

---

## Sources

- [Depot pricing](https://depot.dev/pricing)
- [Depot GitHub Actions runners](https://depot.dev/products/github-actions)
- [Blacksmith pricing](https://www.blacksmith.sh/pricing)
- [Blacksmith: GitHub Actions control plane pricing](https://www.blacksmith.sh/blog/actions-pricing)
- [GitHub Actions 2026 pricing changes](https://github.com/resources/insights/2026-pricing-changes-for-github-actions)
- [2026-05-22-gh-actions-usage.md](2026-05-22-gh-actions-usage.md) — current wbniv burn rate
