FROM ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive

# Vendored packages use Build-Depends from universe (for example dh-python).
RUN sed -i 's/^Components: main restricted$/Components: main restricted universe multiverse/' /etc/apt/sources.list.d/ubuntu.sources \
    && apt-get update -qq \
    && apt-get install -qq -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        debhelper \
        devscripts \
        dpkg-dev \
        fakeroot \
        libcairo2-dev \
        libfontconfig-dev \
        libgif-dev \
        libjpeg-dev \
        libpango1.0-dev \
        librsvg2-dev \
        libsecret-1-dev \
        libx11-dev \
        libxkbfile-dev \
        lintian \
        nodejs \
        npm \
        pkg-config \
        python3 \
        qt6-base-dev \
        qt6-declarative-dev \
        sudo \
        zip \
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
