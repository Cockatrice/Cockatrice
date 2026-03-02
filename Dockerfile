# -------- Build Stage --------
FROM ubuntu:24.04 AS build

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
  build-essential \
  cmake \
  file \
  g++ \
  git \
  libmariadb-dev-compat \
  libprotobuf-dev \
  libqt6sql6-mysql \
  qt6-websockets-dev \
  protobuf-compiler \
  qt6-tools-dev \
  qt6-tools-dev-tools

WORKDIR /src
COPY . .
RUN mkdir build && cd build && \
    cmake .. -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0 && \
    make -j$(nproc) && \
    make install


# -------- Runtime Stage (clean) --------
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
  libprotobuf32t64 \
  libqt6sql6-mysql \
  libqt6websockets6 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Only copy installed binaries, not source
COPY --from=build /usr/local /usr/local

WORKDIR /home/servatrice

EXPOSE 4748
ENTRYPOINT [ "servatrice", "--log-to-console" ]
