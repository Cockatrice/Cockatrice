from archlinux:latest

RUN pacman --sync --refresh --sysupgrade --needed --noconfirm \
        base-devel \
        ccache \
        cmake \
        git \
        gtest \
        mariadb-libs \
        protobuf \
        qt5-base \
        qt5-multimedia \
        qt5-svg \
        qt5-tools \
        qt5-websockets \
    && pacman --sync --clean --clean --noconfirm
