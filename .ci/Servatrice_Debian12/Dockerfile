FROM debian:12

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
      ccache \
      cmake \
      g++ \
      git \
      libmariadb-dev-compat \
      libprotobuf-dev \
      libqt6sql6-mysql \
      ninja-build \
      protobuf-compiler \
      qt6-tools-dev \
      qt6-tools-dev-tools \
      qt6-websockets-dev \
    && rm -rf /var/lib/apt/lists/*
