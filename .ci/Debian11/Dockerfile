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
        liblzma-dev \
        libmariadb-dev-compat \
        libprotobuf-dev \
        libqt5multimedia5-plugins \
        libqt5sql5-mysql \
        libqt5svg5-dev \
        libqt5websockets5-dev \
        ninja-build \
        protobuf-compiler \
        qt5-image-formats-plugins \
        qtmultimedia5-dev \
        qttools5-dev \
        qttools5-dev-tools \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
