# -------- Build Stage --------
FROM ubuntu:26.04 AS build

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
      build-essential \
      ccache \
      cmake \
      ninja-build \
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

ENV CCACHE_DIR=/root/.ccache
ENV CCACHE_MAXSIZE=550M
WORKDIR /src
COPY . .
RUN --mount=type=cache,id=ccache,target=/root/.ccache \
    cmake \
	  -S . \
	  -B build \
      -G Ninja \
      -DCMAKE_C_COMPILER_LAUNCHER=ccache \
      -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
      -DWITH_SERVER=1 \
      -DWITH_CLIENT=0 \
      -DWITH_ORACLE=0 \
    && cmake --build build \
    && cmake --install build \
	&& ccache --show-config \
    && ccache --show-stats


# -------- Runtime Stage (clean) --------
FROM ubuntu:26.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
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
