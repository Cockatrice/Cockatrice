FROM debian:11

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
        libqt5sql5-mysql \
        libqt5websockets5-dev \
        ninja-build \
        protobuf-compiler \
        qttools5-dev \
        qttools5-dev-tools \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
