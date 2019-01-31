FROM ubuntu:bionic
MAINTAINER Zach Halpern <zahalpern+github@gmail.com>

RUN apt-get update && apt-get install -y\
  build-essential\
  cmake\
  git\
  libprotobuf-dev\
  libqt5sql5-mysql\
  protobuf-compiler\
  qt5-default\
  qtbase5-dev\
  qttools5-dev-tools\
  qttools5-dev\
  software-properties-common

COPY . /home/servatrice/code/
WORKDIR /home/servatrice/code

<<<<<<< HEAD
=======
COPY . /home/servatrice/code/
WORKDIR /home/servatrice/code

>>>>>>> 9565b8fa01261d75c26dfae313b05f775d076483
RUN mkdir oracle

WORKDIR build
RUN cmake .. -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0 &&\
  make &&\
  make install

WORKDIR /home/servatrice

EXPOSE 4747

ENTRYPOINT [ "servatrice" ]
