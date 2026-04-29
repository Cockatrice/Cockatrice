FROM debian:12

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        ccache \
        clang-format \
        cmake \
        file \
        g++ \
        git \
        libmariadb-dev-compat \
        libprotobuf-dev \
        libqt6sql6-mysql \
        ninja-build \
        protobuf-compiler \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        qt6-websockets-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
