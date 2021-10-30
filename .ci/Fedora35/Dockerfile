FROM fedora:35

RUN dnf install -y \
        @development-tools \
        ccache \
        cmake \
        desktop-file-utils \
        file \
        gcc-c++ \
        git \
        hicolor-icon-theme \
        libappstream-glib \
        mariadb-devel \
        protobuf-devel \
        qt5-{qttools,qtsvg,qtmultimedia,qtwebsockets}-devel \
        rpm-build \
        sqlite-devel \
        wget \
        xz-devel \
        zlib-devel \
    && dnf clean all
