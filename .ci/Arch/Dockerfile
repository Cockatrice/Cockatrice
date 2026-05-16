from archlinux:latest

RUN pacman --sync --refresh --sysupgrade --needed --noconfirm \
      ccache \
      cmake \
	  gcc \
    # gtest    Exclude gtest to confirm fallback of downloading source from google repo keeps working
      ninja \
      protobuf \
	  qt6-imageformats \
	  qt6-multimedia \
	  qt6-svg \
	  qt6-tools \
	  qt6-translations \
	  qt6-websockets \
    && pacman --sync --clean --clean --noconfirm
