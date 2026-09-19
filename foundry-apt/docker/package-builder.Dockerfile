FROM ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qq \
    && apt-get install -qq -y --no-install-recommends \
        build-essential debhelper dpkg-dev devscripts fakeroot lintian \
        curl ca-certificates pkg-config sudo \
        zip python3 nodejs npm \
        cmake qt6-base-dev qt6-declarative-dev \
        libcairo2-dev libpango1.0-dev libjpeg-dev libgif-dev librsvg2-dev \
        libx11-dev libxkbfile-dev libsecret-1-dev libfontconfig-dev \
    && rm -rf /var/lib/apt/lists/*

ARG NODE_VERSION=24.20.0
ARG NODE_SHA256=2f2c0da162318f0de47665410c7c8c2ed3d36c8f3105de4bbc61176c70a7cbf2
RUN curl -fsSL -o /tmp/node.tar.xz \
        "https://nodejs.org/dist/v${NODE_VERSION}/node-v${NODE_VERSION}-linux-x64.tar.xz" \
    && echo "${NODE_SHA256}  /tmp/node.tar.xz" | sha256sum -c - \
    && tar -xJf /tmp/node.tar.xz -C /usr/local --strip-components=1 \
    && rm -f /tmp/node.tar.xz \
    && node --version

# npm 10 accepts the lockfile-v3 optional-dependency override semantics used by
# current Electron applications; Ubuntu's npm 9 rejects them and npm 11 applies
# a stricter manifest/lock consistency rule.
RUN npm install --global --no-audit --no-fund npm@10.9.4
