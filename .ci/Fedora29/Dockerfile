FROM fedora:29

RUN dnf install -y \
        ccache \
        cmake \
        desktop-file-utils \
        gcc-c++ \
        hicolor-icon-theme \
        libappstream-glib \
        protobuf-devel \
        qt5-{qttools,qtsvg,qtmultimedia,qtwebsockets}-devel \
        wget \
        {zlib,sqlite}-devel \
    && dnf clean all
