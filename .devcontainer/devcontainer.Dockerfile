# Use an official Ubuntu as a parent image
FROM ubuntu:22.04

# Set the environment variable to avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update package lists and install dependencies
RUN apt-get update && \
    apt-get install --yes --no-install-recommends \
        build-essential \
        ccache \
        clang-format \
        cmake \
        file \
        g++ \
        git \
        libgl-dev \
        liblzma-dev \
        libmariadb-dev-compat \
        libprotobuf-dev \
        libqt6multimedia6 \
        libqt6sql6-mysql \
        libqt6svg6-dev \
        libqt6websockets6-dev \
        protobuf-compiler \
        qt6-l10n-tools \
        qt6-multimedia-dev \
        qt6-tools-dev \
        qt6-tools-dev-tools \
    && rm -rf /var/lib/apt/lists/*


SHELL ["/bin/bash", "-c"]

# Set the working directory
WORKDIR /workspace

# Copy the local Cockatrice repository into the Docker image
COPY Cockatrice /workspace/Cockatrice

#  Create a build directory and configure the build with CMake
RUN cd Cockatrice && \
    mkdir build && \
    cd build && \
    cmake ..

# Build the Cockatrice project
RUN cd Cockatrice/build && \
    cmake --build . --parallel $(nproc)

# Expose ports if needed (adjust according to your application requirements)
EXPOSE 6080

# Run the application (adjust according to your application requirements)
CMD ["/workspace/Cockatrice/build/cockatrice"]
