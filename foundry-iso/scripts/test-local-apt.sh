#!/usr/bin/env bash
# Run inside the disposable ISO tools container with the repository at /work.
set -euo pipefail
mode=${1:-normal}
mkdir -p /tmp/public /tmp/local
package() {
    local dest=$1 name=$2 version=$3 payload=$4 depends=${5:-}
    local dir=/tmp/build-$name-$version
    mkdir -p "$dir/DEBIAN" "$dir/usr/share/foundry-local-test"
    printf 'Package: %s\nVersion: %s\nArchitecture: all\nMaintainer: Test <test@example.invalid>\nDescription: fixture\n' "$name" "$version" > "$dir/DEBIAN/control"
    [[ -z "$depends" ]] || printf 'Depends: %s\n' "$depends" >> "$dir/DEBIAN/control"
    printf '%s\n' "$payload" > "$dir/usr/share/foundry-local-test/$name"
    (cd "$dir" && md5sum "usr/share/foundry-local-test/$name" > DEBIAN/md5sums)
    dpkg-deb --build "$dir" "$dest/$name.deb" >/dev/null
    rm -rf "$dir"
}
package /tmp/public foundry-anvil 1 public 'fixture-app, fixture-newer'
package /tmp/public fixture-app 1 public
package /tmp/public fixture-newer 2 public-new
package /tmp/local foundry-anvil 1 local 'fixture-app, fixture-newer'
package /tmp/local fixture-app 1 local
package /tmp/local fixture-newer 1 local-old
package /tmp/local fixture-atelier 1 unwanted
rm -f /etc/apt/sources.list /etc/apt/sources.list.d/*
printf 'deb [trusted=yes] file:/tmp/public ./\n' > /etc/apt/sources.list.d/public.list
(cd /tmp/public && dpkg-scanpackages . /dev/null > Packages)
apt-get update -qq
options=()
[[ "$mode" != older ]] || options+=(--allow-older)
python3 /work/scripts/prepare-local-apt.py --root / --archives /tmp/local \
    --edition anvil --package-list /tmp/local.list "${options[@]}"
mapfile -t selected < /tmp/local.list
apt-get install -y foundry-anvil "${selected[@]}"
test "$(cat /usr/share/foundry-local-test/fixture-app)" = local
test "$(cat /usr/share/foundry-local-test/foundry-anvil)" = local
test ! -e /usr/share/foundry-local-test/fixture-atelier
if [[ "$mode" == older ]]; then
    test "$(dpkg-query -W -f='${Version}' fixture-newer)" = 1
else
    test "$(dpkg-query -W -f='${Version}' fixture-newer)" = 2
fi
bash /work/config/hooks/1000-install-local-debs.hook.chroot
# The receipt must reject a different payload with the same package version.
cp /var/lib/dpkg/info/fixture-app.md5sums /tmp/original-md5sums
printf 'bad\n' > /var/lib/dpkg/info/fixture-app.md5sums
if bash /work/config/hooks/1000-install-local-debs.hook.chroot; then
    echo 'ERROR: mismatching same-version payload accepted' >&2
    exit 1
fi
cp /tmp/original-md5sums /var/lib/dpkg/info/fixture-app.md5sums
bash /work/config/hooks/1290-remove-local-build-repository.hook.chroot
test ! -e /opt/foundry-local-apt
test ! -e /etc/apt/sources.list.d/00-foundry-local.list
test -s /usr/share/foundry/build/local-packages.json
echo "PASS: local APT $mode selection, payload verification and cleanup"
