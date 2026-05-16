FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
      ccache \
      cmake \
      g++ \
      git \
      libgl-dev \
      liblzma-dev \
      libprotobuf-dev \
      libqt6multimedia6 \
      ninja-build \
      protobuf-compiler \
      qt6-image-formats-plugins \
      qt6-l10n-tools \
      qt6-multimedia-dev \
      qt6-svg-dev \
      qt6-tools-dev \
      qt6-tools-dev-tools \
      qt6-websockets-dev \
    && rm -rf /var/lib/apt/lists/*
