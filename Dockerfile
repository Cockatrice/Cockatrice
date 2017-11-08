FROM ubuntu:trusty
MAINTAINER Gavin Bisesi <Daenyth@gmail.com>

RUN apt-get update && apt-get install -y software-properties-common
RUN apt-add-repository ppa:ubuntu-sdk-team/ppa
RUN add-apt-repository -y ppa:smspillaz/cmake-master
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
RUN cmake .. -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0 &&\
  make &&\
  make install

WORKDIR /home/servatrice

EXPOSE 4747

ENTRYPOINT [ "servatrice" ]
