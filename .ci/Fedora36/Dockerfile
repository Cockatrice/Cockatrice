FROM fedora:36

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
        libxkbcommon-devel \
        mariadb-devel \
        protobuf-devel \
        qt6-{qttools,qtsvg,qtmultimedia,qtwebsockets,qt5compat}-devel \
        rpm-build \
        sqlite-devel \
        wget \
        xz-devel \
        zlib-devel \
    && dnf clean all
