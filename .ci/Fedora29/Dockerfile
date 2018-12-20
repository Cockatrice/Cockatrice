FROM fedora:29

RUN dnf install -y \
        @development-tools \
        ccache \
        cmake \
        desktop-file-utils \
        file \
        gcc-c++ \
        hicolor-icon-theme \
        libappstream-glib \
        protobuf-devel \
        qt5-{qttools,qtsvg,qtmultimedia,qtwebsockets}-devel \
        rpm-build \
        sqlite-devel \
        wget \
        zlib-devel \
        xz-devel \
    && dnf clean all
