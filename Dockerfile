FROM ubuntu:trusty
MAINTAINER Gavin Bisesi <Daenyth@gmail.com>

RUN apt-get update && apt-get install -y software-properties-common
RUN apt-add-repository ppa:ubuntu-sdk-team/ppa
RUN apt-get update && apt-get install -y\
  cmake\
  git\
  libprotobuf-dev\
  libqt5svg5-dev\
  libqt5webkit5-dev\
  libsqlite3-dev\
  protobuf-compiler\
  qt5-default\
  qtbase5-dev\
  qtdeclarative5-dev\
  qtmultimedia5-dev\
  qttools5-dev-tools\
  qttools5-dev

ENV dir /home/servatrice/code
WORKDIR $dir
RUN mkdir oracle
COPY COPYING COPYING
COPY CMakeLists.txt CMakeLists.txt
COPY cmake/ cmake/
COPY common/ common/
COPY servatrice/ servatrice/
COPY README.md README.md

WORKDIR build
RUN cmake .. -DWITH_QT4=0 -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0 &&\
  make &&\
  make install

WORKDIR /home/servatrice

EXPOSE 4747

CMD servatrice
