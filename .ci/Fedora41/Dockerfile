FROM fedora:41

RUN dnf install -y \
        ccache \
        cmake \
        gcc-c++ \
        git \
        mariadb-devel \
        ninja-build \
        protobuf-devel \
        qt6-{qttools,qtsvg,qtmultimedia,qtwebsockets}-devel \
        qt6-qtimageformats \
        rpm-build \
        xz-devel \
        zlib-devel \
    && dnf clean all
