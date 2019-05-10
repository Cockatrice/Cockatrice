FROM centos:7

RUN yum install -y \
        "@development tools" \
        ccache \
        cmake3 \
        desktop-file-utils \
        epel-release \
        file \
        gcc-c++ \
        protobuf-devel \
        qt5-{qttools,qtsvg,qtmultimedia,qtwebsockets}-devel \
        rpm-build \
        sqlite-devel \
        wget \
        xz-devel \
        zlib-devel \
    && yum clean all \
    && ln -s /usr/bin/cmake3 /usr/bin/cmake \
    && source /opt/rh/devtoolset-7/enable
