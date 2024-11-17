FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y\
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
  qt6-svg-dev \
  qt6-websockets-dev \
  protobuf-compiler \
  qt6-l10n-tools \
  qt6-multimedia-dev \
  qt6-tools-dev \
  qt6-tools-dev-tools

COPY . /home/servatrice/code/
WORKDIR /home/servatrice/code

WORKDIR build
RUN cmake .. -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0 -DWITH_DBCONVERTER=0 &&\
  make &&\
  make install

WORKDIR /home/servatrice

EXPOSE 4747 4748

ENTRYPOINT [ "servatrice", "--log-to-console" ]
