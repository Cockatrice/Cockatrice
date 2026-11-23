FROM debian:12-slim

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        ca-certificates \
        ccache \
        cmake \
        dpkg-dev \
        file \
        g++ \
        git \
        libcurl4-openssl-dev \
        libgl-dev \
        libgtest-dev \
        liblzma-dev \
        libmariadb-dev-compat \
        libprotobuf-dev \
        libqt6multimedia6 \
        libqt6sql6-mysql \
        libssl-dev \
        ninja-build \
        protobuf-compiler \
        qt6-declarative-dev \
        qt6-image-formats-plugins \
        qt6-l10n-tools \
        qt6-multimedia-dev \
        qt6-shadertools-dev \
        qt6-svg-dev \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        qt6-websockets-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
