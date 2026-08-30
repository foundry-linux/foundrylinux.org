# Signal Desktop vs shared Electron container builds

**Date:** 2026-08-30
**Scope:** Signal Desktop's public Linux reproducible-build harness compared
with the current Foundry APT pipeline and the proposed shared Electron pipeline.

## Conclusion

The systems optimize for different primary outcomes. Signal tries to reproduce
one application package byte-for-byte from source. The current Foundry pipeline
tries to produce ABI-correct Ubuntu 26.04 packages and publish a complete signed
multi-package repository safely. The shared Electron project needs both: adopt
Signal's deterministic build capsule without discarding Foundry's repository
transaction, completeness checks, signing separation, or live verification.

“Built in a container” is not equivalent to “reproducible.” Our current
`ubuntu:26.04` tag, live `apt update`, missing `SOURCE_DATE_EPOCH`, and build-time
`npx` resolution make the output environment-controlled but time-variant.

## Goal-by-goal comparison

| Stated or implied goal | Signal implementation | Current/proposed shared-runtime implementation | Assessment and required action |
|---|---|---|---|
| Build on any Docker-capable host | Repository-local Dockerfile and wrapper; source tree bind-mounted into the container | Thin wrapper runs `ubuntu:26.04` and bind-mounts the repository | Both meet the portability goal. Keep the one-command local/CI path. |
| Reproduce the official application from public source | Full Signal source build, with an experimental procedure to compare the resulting `.deb` SHA-256 with Signal's official package | LosslessCut currently repacks a pinned official AppImage and modifies its ASAR; Electron repacks a pinned official binary zip | Different provenance. Describe ours as verified binary repackaging, not a source-reproducible application build. Add a later LosslessCut source-build track. |
| Keep the base operating system identical between runs | Ubuntu Jammy base pinned by dated tag and image digest | Mutable `ubuntu:26.04` tag | Signal is stronger. Pin the Resolute image digest and update it through reviewed automation. |
| Build against the actual target distribution ABI | Jammy-based capsule plus a declared minimum glibc for Signal's vendor package | Resolute container intentionally drives `dpkg-shlibdeps` against Ubuntu 26.04 sonames | Ours is stronger for a Resolute-native APT repository. Preserve Resolute as the build base when adding digest pinning. |
| Keep operating-system build dependencies stable | Purpose-built image; base digest fixed, although image construction still obtains packages from configured APT sources | Runs `apt update` and installs current archive packages during every build | Neither is a complete snapshot, but ours varies on every invocation. Build a versioned toolchain image from a dated/snapshotted manifest and publish its digest. |
| Keep JavaScript dependencies stable | Exact pnpm version, `pnpm install --frozen-lockfile`, committed lockfile and patch hashes | LosslessCut proof invokes `npx @electron/asar@4.0.1`; top-level version is pinned but its resolved graph/artifacts are not committed | Adopt a tiny committed workspace/lockfile for ASAR tooling, run offline from the toolchain image, and verify the package-manager store inputs. |
| Make timestamps deterministic | Sets `SOURCE_DATE_EPOCH` from the selected Git commit, with an override | No pipeline-wide timestamp normalization | Adopt Signal's model: derive it from the packaging commit and pass it through Docker, `dpkg-buildpackage`, archive generation, SBOM, and provenance. |
| Avoid root-owned or host-specific build artifacts | Runs the container as the invoking UID/GID and places caches in writable temporary paths | Builds as root and repairs ownership after the build | Prefer Signal's user mapping. Retain fakeroot only where Debian packaging requires simulated ownership; test the SUID sandbox in an install container. |
| Rebuild native Electron modules for the selected ABI | `electron-builder install-app-deps` plus frozen application dependencies | Application contract requires ABI checking; LosslessCut currently has no native `.node` modules | Turn the contract into a gate: inventory `.node` files, rebuild where necessary, record Electron module ABI, and reject mismatches. |
| Prove two independent builds are identical | Documented comparison against the official `.deb`; Linux reproducibility is explicitly experimental | No double-build/hash-equality gate | Build runtime and app twice in fresh containers with isolated caches and compare hashes. On mismatch, run `diffoscope` and block stable promotion. |
| Verify downloaded upstream inputs | Dependencies are lockfile-controlled; official package comparison supplies a strong end-to-end oracle | Electron zip and LosslessCut AppImage have committed SHA-256 pins | Ours is strong for primary binary inputs. Also verify Electron's published checksum manifest/signature and record immutable URLs in provenance. |
| Make the build independent of transient networks | Container build and pnpm install still download dependencies | APT, GitHub, AppImage, Electron zip, and npm are all fetched during the build | Neither fully meets it. Add a fetch stage that creates a content-addressed, checksummed input bundle; make stable builds consume only that bundle. |
| Produce Debian-policy-aware dependency metadata | `electron-builder` emits Signal's vendor `.deb` from application configuration | Canonical `debian/` trees and `dpkg-shlibdeps` generate target archive dependencies | Ours is stronger for repository integration. Keep canonical Debian source-package layout and add policy/lintian gates to the dedicated runtime pipeline. |
| Share one runtime safely across unrelated apps | Not a Signal goal; Signal deliberately ships a complete private application bundle | Versioned `electron-runtime-<major>` packages, exact-major app dependencies, resource-path/native-module contract | Unique to our project. Add cross-app install/remove/upgrade tests and prevent app packages from owning runtime files. |
| Preserve the Electron renderer sandbox | Signal's bundled runtime controls its own helper and packaging | Shared runtime owns `chrome-sandbox` as `4755 root:root`; apps must not use `--no-sandbox` | Correct design, but add a runtime process inspection test proving the sandbox is selected after installation. |
| Preserve application-private helpers | Signal bundles everything as one application unit | LosslessCut retains its private FFmpeg while generic Electron moves out | Correct boundary. Test helper discovery, RPATH isolation, and absence of collisions across apps. |
| Produce auditable provenance | Reproducible source-to-official-package equality is the principal evidence | Plan calls for SBOM and SLSA/in-toto-style provenance; current proof has not implemented them | Combine both: publish build-image digest, input hashes, source commit, packaging commit, `SOURCE_DATE_EPOCH`, output hashes, SBOM, and test result. Do not claim this until emitted and verified. |
| Prevent a partial multi-package repository release | Outside the public reproducible-build harness's scope | Durable R2 hydration, complete-dist gate, targeted-build refusal, and retained secondary cache | Ours is stronger. Preserve unchanged in the neutral runtime repository. |
| Keep signed repository metadata internally consistent during deployment | Outside the reviewed Signal build harness's scope | Build and signing separated; package indexes uploaded before `Release`; live hash comparison follows | Ours is stronger. Preserve staged metadata publication and live consistency checks. |
| Support rollback and artifact retention | Official Signal releases remain available through Signal's release system, but not specified by the reviewed harness | Durable R2 artifact mirror and planned immutable runtime snapshots/rollback | Formalize retention periods, snapshot identifiers, candidate-to-stable promotion, and emergency yanking without deleting audit history. |
| Separate build authority from publishing authority | Not established by the public local reproduction script | CI builds unsigned packages, then imports a release key only for repository signing | Good boundary, though the same job currently performs both. Move signing/promotion to a protected environment or separate job with artifact attestations. |
| Let outsiders verify the released artifact | Anyone can build the tagged Signal source and compare the `.deb` hash | Users can verify APT signatures and input pins, but cannot yet reproduce and compare our output reliably | Add documented third-party rebuild instructions and publish expected hashes plus `diffoscope` guidance. |
| Avoid overstating security guarantees | Signal labels Linux reproducibility experimental | Plan describes future provenance features alongside current proof work | Mark every control as implemented, experimental, or planned. “Containerized” means ABI-isolated today; it must not be presented as reproducible yet. |

## Design to carry forward

The durable shared-runtime release flow should be:

```text
reviewed source + packaging commit
             │
             ▼
content-addressed fetch stage ── verifies upstream signatures and hashes
             │
             ▼
digest-pinned Ubuntu 26.04 toolchain image
frozen APT/tool manifests + frozen JS lockfile + SOURCE_DATE_EPOCH
             │
       ┌─────┴─────┐
       ▼           ▼
 clean build A  clean build B
       └─────┬─────┘
             ▼
 hash equality or diffoscope failure
             │
             ▼
 ABI, sandbox, application and cross-app tests
             │
             ▼
 unsigned .debs + SBOM + provenance + test evidence
             │
             ▼
 protected signing/promotion job
             │
             ▼
 complete repository gate → staged metadata upload → live consistency check
```

For an intentionally transformed application such as shared-runtime Signal,
first reproduce Signal's untouched official `.deb`. That establishes the source
baseline. The transformed package cannot equal the official hash; instead,
compare its declared file split, rebuilt native modules, runtime dependency,
functional tests, and deterministic output across two independent builds.

## Sources inspected

- [Signal Linux reproducible-build guide](https://github.com/signalapp/Signal-Desktop/blob/main/reproducible-builds/README.md)
- [Signal reproducible build wrapper](https://github.com/signalapp/Signal-Desktop/blob/main/reproducible-builds/build.sh)
- [Signal reproducible-build Dockerfile](https://github.com/signalapp/Signal-Desktop/blob/main/reproducible-builds/Dockerfile)
- [Signal container entrypoint](https://github.com/signalapp/Signal-Desktop/blob/main/reproducible-builds/docker-entrypoint.sh)
- [Signal dependency lockfile](https://github.com/signalapp/Signal-Desktop/blob/main/pnpm-lock.yaml)
- `foundry-apt/scripts/build-all-in-docker.sh`
- `foundry-apt/.github/workflows/publish.yml`
- `foundry-apt/packages/electron-runtime-42/build.sh`
- `foundry-apt/packages/losslesscut/build.sh`
