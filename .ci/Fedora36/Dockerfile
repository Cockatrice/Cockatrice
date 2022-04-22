FROM fedora:36

RUN dnf install -y \
        ccache \
        cmake \
        gcc-c++ \
        git \
        mariadb-devel \
        protobuf-devel \
        qt6-{qttools,qtsvg,qtmultimedia,qtwebsockets,qt5compat}-devel \
        rpm-build \
        xz-devel \
        zlib-devel \
    && dnf clean all
