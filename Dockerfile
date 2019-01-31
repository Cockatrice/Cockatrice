FROM ubuntu:bionic
MAINTAINER Zach Halpern <zahalpern+github@gmail.com>

RUN apt-get update && apt-get install -y software-properties-common
RUN apt-get update && apt-get install -y\
  build-essential g++\
  cmake\
  git\
  libprotobuf-dev\
  protobuf-compiler\
  qt5-default\
  qtbase5-dev\
  qttools5-dev-tools\
  qttools5-dev\
  libqt5sql5-mysql

COPY . /home/servatrice/code/
WORKDIR /home/servatrice/code

RUN mkdir oracle

WORKDIR build
RUN cmake .. -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0 &&\
  make &&\
  make install

WORKDIR /home/servatrice

EXPOSE 4747

ENTRYPOINT [ "servatrice" ]
