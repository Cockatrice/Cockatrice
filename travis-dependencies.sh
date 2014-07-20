#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew update
  brew install qt protobuf libgcrypt
else
  sudo apt-get update -qq
  sudo apt-get install -y qtmobility-dev libprotobuf-dev protobuf-compiler libqt4-dev
fi
