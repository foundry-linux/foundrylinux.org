# Upstream bug: live-build defaults to sysvinit on Ubuntu 26.04

**Where to file:** https://bugs.launchpad.net/ubuntu/+source/live-build/+filebug

## Summary

`live-build 3.0~a57-1ubuntu54` (Ubuntu 26.04) defaults `LB_INITSYSTEM=sysvinit`, but
`live-config-sysvinit` was removed from Ubuntu 26.04. Any live-build run that does not
explicitly set `--initsystem systemd` fails hard during `lb_chroot_live-packages`:

```
E: Package 'live-config-sysvinit' has no installation candidate
```

## Steps to reproduce

```bash
docker run --rm --privileged ubuntu:26.04 bash -c '
  apt-get update -qq
  apt-get install -y live-build debootstrap
  lb config --mode ubuntu --distribution resolute
  lb bootstrap
  lb chroot   # fails here
'
```

## Expected behaviour

Ubuntu 26.04 uses systemd. `lb_chroot_live-packages` should install
`live-config-systemd` (which exists) rather than `live-config-sysvinit` (which does not).

## Actual behaviour

`lb_chroot_live-packages` runs:
```
apt-get install live-boot live-config live-config-sysvinit
```
`live-config-sysvinit` has no installation candidate → build aborts.

## Root cause

`/usr/lib/live/build/lb_config` sets `LB_INITSYSTEM` to `sysvinit` by default.
`lb_chroot_live-packages` constructs the package name as `live-config-${LB_INITSYSTEM}`.
`live-config-sysvinit` was dropped from Ubuntu 26.04; `live-config-systemd` is the
correct package and is available.

## Fix

In `lb_config`, change the default for `LB_INITSYSTEM` from `sysvinit` to `systemd`
when running under `--mode ubuntu` on Ubuntu >= 20.04 (all of which are systemd-only).

One-line workaround for users until this is fixed:

```bash
lb config --initsystem systemd ...
```

## Environment

- Ubuntu 26.04 LTS (Resolute Raccoon)
- `live-build` 3.0~a57-1ubuntu54
- `live-config-systemd` 11.0.5build1 — available and working
- `live-config-sysvinit` — no installation candidate
