FROM fedora:38

RUN dnf install -y \
        ccache \
        cmake \
        gcc-c++ \
        git \
        mariadb-devel \
        protobuf-devel \
        qt6-{qttools,qtsvg,qtmultimedia,qtwebsockets}-devel \
        rpm-build \
        xz-devel \
        zlib-devel \
    && dnf clean all
