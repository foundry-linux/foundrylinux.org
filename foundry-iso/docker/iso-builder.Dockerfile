# Refresh the digest/revision deliberately when validating toolchain updates.
ARG BASE=ubuntu:26.04@sha256:6a06111c3b0a8600feadb270aec771dc239bf407b25625f93f44ab00e11f4d3c
FROM ${BASE}
ARG TOOLCHAIN_REVISION=2026-11-15
ENV DEBIAN_FRONTEND=noninteractive
RUN test -n "$TOOLCHAIN_REVISION" \
    && apt-get update -qq -o APT::Update::Error-Mode=any \
    && apt-get install -y --no-install-recommends \
       live-build curl gpg apt-utils ca-certificates dpkg-dev python3 \
       xorriso genisoimage isolinux grub-efi-amd64-bin grub-pc-bin \
       mtools dosfstools e2fsprogs squashfs-tools \
    && dpkg-query -W > /usr/local/share/foundry-iso-toolchain.tsv \
    && rm -rf /var/lib/apt/lists/*
