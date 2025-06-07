from archlinux:latest

RUN pacman --sync --refresh --sysupgrade --needed --noconfirm \
        base-devel \
        ccache \
        cmake \
        git \
        gtest \
        mariadb-libs \
        ninja \
        protobuf \
        qt6-base \
        qt6-imageformats \
        qt6-multimedia \
        qt6-svg \
        qt6-tools \
        qt6-translations \
        qt6-websockets \
    && pacman --sync --clean --clean --noconfirm
