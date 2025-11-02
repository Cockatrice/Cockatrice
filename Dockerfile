FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
  apt-get install -y \
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
  qt6-tools-dev-tools && \
  rm -rf /var/lib/apt/lists/*

# Order from least changing at the top to more frequently changing at the bottom
COPY ./libcockatrice_network /home/servatrice/code/libcockatrice_network
COPY ./libcockatrice_protocol /home/servatrice/code/libcockatrice_protocol
COPY ./libcockatrice_rng /home/servatrice/code/libcockatrice_rng
COPY ./servatrice /home/servatrice/code/servatrice
COPY ./cmake /home/servatrice/code/cmake
COPY ./CMakeLists.txt ./LICENSE ./README.md /home/servatrice/code/
COPY ./libcockatrice_utility /home/servatrice/code/libcockatrice_utility
COPY ./libcockatrice_settings /home/servatrice/code/libcockatrice_settings
COPY ./libcockatrice_deck_list /home/servatrice/code/libcockatrice_deck_list
COPY ./libcockatrice_card /home/servatrice/code/libcockatrice_card

WORKDIR /home/servatrice/code

WORKDIR build

RUN cmake .. -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0 -DWITH_DBCONVERTER=0 && \
  make && \
  make install

WORKDIR /home/servatrice

EXPOSE 4748

ENTRYPOINT [ "servatrice", "--log-to-console" ]
