#!/usr/bin/env bash
set -Eeuo pipefail
source /source/scripts/build-resources.sh
ccache --zero-stats >/dev/null
for attempt in 1 2; do
    directory=$(mktemp -d /tmp/foundry-build/hello.XXXXXX)
    printf 'int answer(void) { return 42; }\n' > "$directory/hello.c"
    (
        cd "$directory"
        gcc -g -ffile-prefix-map="$PWD"=/usr/src/hello -c "$PWD/hello.c" -o hello.o
    )
done
ccache --show-stats
hits=$(ccache --print-stats | awk '$1 == "direct_cache_hit" || $1 == "preprocessed_cache_hit" { total += $2 } END { print total+0 }')
test "$hits" -ge 1
echo 'PASS: bounded compiler cache reused a compilation across distinct temporary directories'
