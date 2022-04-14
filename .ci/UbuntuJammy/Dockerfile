FROM ubuntu:jammy

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        ccache \
        clang-format \
        cmake \
        file \
        g++ \
        git \
        libgl1-mesa-dev \
        liblzma-dev \
        libmariadb-dev-compat \
        libprotobuf-dev \
        libxkbcommon-dev \
        libvulkan-dev \
        libqt6core5compat6-dev \
        libqt6multimedia6 \
        libqt6sql6-mysql \
        libqt6svg6-dev \
        libqt6websockets6-dev \
        protobuf-compiler \
        qt6-multimedia-dev \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        qt6-l10n-tools \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
