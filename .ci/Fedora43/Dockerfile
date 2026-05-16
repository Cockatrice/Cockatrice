FROM fedora:43

RUN dnf install -y --setopt=install_weak_deps=False \
      ccache \
      cmake \
      gcc-c++ \
      git \
	  gtest-devel \
      ninja-build \
      protobuf-devel \
      qt6-qttools-devel \
      qt6-qtsvg-devel \
      qt6-qtmultimedia-devel \
      qt6-qtwebsockets-devel \
      qt6-qtimageformats \
      rpm-build \
      xz-devel \
      zlib-devel \
    && dnf clean all \
	&& rm -rf /var/cache/dnf
