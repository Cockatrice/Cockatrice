#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew update
  brew install qt qt5 protobuf libgcrypt
else
  sudo apt-get update -qq
  sudo apt-get install -y qtmobility-dev libprotobuf-dev protobuf-compiler libqt4-dev qt5-default qttools5-dev-tools
fi
