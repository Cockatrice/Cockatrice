# -------- Build Stage --------
FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y \
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
    cmake .. -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0 -DWITH_DBCONVERTER=0 && \
    make -j$(nproc) && \
    make install


# -------- Runtime Stage (clean) --------
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
  libmariadb-dev-compat \
  libprotobuf-dev \
  libqt6sql6-mysql \
  qt6-websockets-dev

# Only copy installed binaries, not source
COPY --from=build /usr/local /usr/local

EXPOSE 4748
ENTRYPOINT [ "servatrice", "--log-to-console" ]
