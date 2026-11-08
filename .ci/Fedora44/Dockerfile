FROM fedora:44

RUN dnf install -y --nodocs \
        ca-certificates \
        ccache \
        cmake \
        gcc-c++ \
        git \
        gtest-devel \
        libcurl-devel \
        mariadb-devel \
        ninja-build \
        openssl-devel \
        protobuf-devel \
        qt6-{qtdeclarative,qtmultimedia,qtshadertools,qtsvg,qttools,qtwebsockets}-devel \
        qt6-qtimageformats \
        rpm-build \
        xz-devel \
        zlib-devel \
    && dnf clean all
