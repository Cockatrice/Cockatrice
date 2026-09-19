FROM fedora:43

RUN dnf install -y \
        ccache \
        cmake \
        gcc-c++ \
        git \
        mariadb-devel \
        ninja-build \
        openssl-devel \
        protobuf-devel \
        qt6-{qtdeclarative,qtshadertools,qttools,qtsvg,qtmultimedia,qtwebsockets}-devel \
        qt6-qtimageformats \
        rpm-build \
        xz-devel \
        zlib-devel \
    && dnf clean all
