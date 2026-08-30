#!/usr/bin/env bash
# Initialise a local aptly repo named 'foundry' for the 'resolute' suite
# (Kubuntu 26.04 codename: 'resolute raccoon'). Idempotent.
#
# Prereqs: aptly installed (apt install aptly).

set -euo pipefail
cd "$(dirname "$0")/.."

# Generate a runtime config with the absolute path to ./public/ baked in,
# since aptly requires FileSystemPublishEndpoints.rootDir to be absolute.
REPO_NAME="${REPO_NAME:-foundry}"
RUNTIME_CONFIG="/tmp/aptly-${REPO_NAME}.conf"
PUBLIC_DIR="${PUBLIC_DIR:-$(pwd)/public}"
jq --arg pub "$PUBLIC_DIR" \
    '.FileSystemPublishEndpoints = {"public": {"rootDir": $pub, "linkMethod": "copy", "verifyMethod": "md5"}}' \
    aptly/aptly.conf > "$RUNTIME_CONFIG"
export APTLY_CONFIG="${APTLY_CONFIG:-$RUNTIME_CONFIG}"

if ! command -v aptly &>/dev/null; then
    echo "ERROR: aptly not installed. Run: sudo apt install aptly" >&2
    exit 1
fi

# Create the repo if it doesn't exist
if ! aptly -config="$APTLY_CONFIG" repo show "$REPO_NAME" &>/dev/null; then
    echo "Creating aptly repo '$REPO_NAME'..."
    aptly -config="$APTLY_CONFIG" repo create \
        -distribution=resolute \
        -component=main \
        -architectures=amd64,arm64,all \
        "$REPO_NAME"
else
    echo "Repo '$REPO_NAME' already exists."
fi

aptly -config="$APTLY_CONFIG" repo show "$REPO_NAME"
