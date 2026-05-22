#!/bin/bash
set -euo pipefail
# Drop-in genisoimage wrapper using xorriso.
# Ubuntu 26.04 live ISOs include a squashfs > 4 GiB; genisoimage aborts on files
# larger than 4 GiB-1. xorriso handles large files natively via its -as mkisofs
# compatibility layer.
exec xorriso -as mkisofs -iso-level 3 "$@"
