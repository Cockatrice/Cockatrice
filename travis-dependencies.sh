#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew update
  brew install qt cmake protobuf
else
  sudo apt-get update -qq
  sudo apt-get install -y qtmobility-dev libprotobuf-dev protobuf-compiler libqt4-dev qt4-dev-tools
fi
