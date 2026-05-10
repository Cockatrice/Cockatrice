# -------- Build Stage --------
FROM ubuntu:26.04 AS build

ARG DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
  build-essential \
  cmake \
  file \
  g++ \
  git \
  libmariadb-dev-compat \
  libprotobuf-dev \
  libqt6sql6-mysql \
  protobuf-compiler \
  qt6-tools-dev \
  qt6-tools-dev-tools \
  qt6-websockets-dev

WORKDIR /src

# Configure CMake
COPY CMakeLists.txt ./
COPY cmake/ ./cmake/
RUN mkdir build && cd build && \
  cmake .. -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0

# Build Servatrice
COPY . .
RUN cd build && \
  make -j$(nproc) && make install


# -------- Runtime Stage (clean) --------
FROM ubuntu:26.04

# Install only dependencies needed at runtime
RUN apt-get update && apt-get install -y --no-install-recommends \
  libprotobuf32t64 \
  libqt6sql6-mysql \
  libqt6websockets6 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Copy only resulting binaries from Build Stage
COPY --from=build /usr/local /usr/local

WORKDIR /home/servatrice

EXPOSE 4748
ENTRYPOINT [ "servatrice", "--log-to-console" ]
