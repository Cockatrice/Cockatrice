from archlinux:latest

RUN pacman --sync --refresh --sysupgrade --needed --noconfirm \
      ccache \
      cmake \
	  gcc \
	  git \
      gtest \
      ninja \
      protobuf \
	  qt6-imageformats \
	  qt6-multimedia \
	  qt6-svg \
	  qt6-tools \
	  qt6-translations \
	  qt6-websockets \
    && pacman --sync --clean --clean --noconfirm
