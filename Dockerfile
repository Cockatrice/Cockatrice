FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y\
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

COPY ./CMakeLists.txt ./LICENSE ./README.md /home/servatrice/code/
COPY ./cmake /home/servatrice/code/cmake
COPY ./libcockatrice_network/libcockatrice/network/server /home/servatrice/code/libcockatrice_network/libcockatrice/network/server
COPY ./libcockatrice_protocol/libcockatrice/protocol /home/servatrice/code/libcockatrice_protocol/libcockatrice/protocol
COPY ./servatrice /home/servatrice/code/servatrice

WORKDIR /home/servatrice/code

WORKDIR build
RUN cmake .. -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0 -DWITH_DBCONVERTER=0 &&\
  make &&\
  make install

WORKDIR /home/servatrice

EXPOSE 4748

ENTRYPOINT [ "servatrice", "--log-to-console" ]
