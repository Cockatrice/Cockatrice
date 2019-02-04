FROM ubuntu:bionic

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        clang-format \
        file \
        g++ \
        git \
        ccache \
        cmake \
        liblzma-dev \
        libprotobuf-dev \
        libqt5multimedia5-plugins \
        libqt5svg5-dev \
        libqt5sql5-mysql \
        libqt5websockets5-dev \
        protobuf-compiler \
        qt5-default \
        qttools5-dev \
        qttools5-dev-tools \
        qtmultimedia5-dev \
    && rm -rf /var/lib/apt/lists/*
