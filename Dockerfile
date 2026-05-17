# -------- Build Stage --------
FROM debian:13-slim AS build

ARG DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
      cmake \
      g++ \
      libmariadb-dev-compat \
      libprotobuf-dev \
      libqt6sql6-mysql \
      ninja-build \
      protobuf-compiler \
      qt6-tools-dev \
      qt6-tools-dev-tools \
      qt6-websockets-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy repo source not excluded in .dockerignore
WORKDIR /src
COPY . .

# Configure CMake
RUN cmake -S . -B build -G Ninja \
    -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0

# Build and install Servatrice
RUN cmake --build build --parallel $(nproc) \
    && cmake --install build


# -------- Runtime Stage (clean) --------
FROM debian:13-slim

# Install runtime dependencies
# Is libprotobuf-lite32t64 enough?
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
      libprotobuf32t64 \
      libqt6sql6-mysql \
      libqt6websockets6 \
    && rm -rf /var/lib/apt/lists/*

# Copy only resulting binaries from Build Stage
COPY --from=build /usr/local/bin/servatrice /usr/local/bin/servatrice

# Create and run as non-root user
RUN useradd -m servatrice
USER servatrice
WORKDIR /home/servatrice

EXPOSE 4748
ENTRYPOINT [ "servatrice", "--log-to-console" ]
