#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew update
  if (( QT4 )); then
    brew install qt protobuf libgcrypt
  else
    brew install qt qt5 protobuf libgcrypt
  fi
else
  if (( QT4 )); then
    sudo apt-get update -qq
    sudo apt-get install -y qtmobility-dev libprotobuf-dev protobuf-compiler libqt4-dev
  else
    sudo add-apt-repository --yes ppa:ubuntu-sdk-team/ppa
    sudo apt-get update -qq
    sudo apt-get install -y libprotobuf-dev protobuf-compiler qtbase5-dev cmake\
      qtdeclarative5-dev libqt5webkit5-dev libsqlite3-dev qt5-default qttools5-dev-tools\
      qttools5-dev qtmultimedia5-dev libqt5svg5-dev
  fi
fi
