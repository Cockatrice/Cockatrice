#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew update
  if (( QT4 )); then
    brew install qt protobuf libgcrypt
  else
    brew install qt5 protobuf libgcrypt
  fi
else
  if (( QT4 )); then
    sudo apt-get update -qq
    sudo apt-get install -y qtmobility-dev libprotobuf-dev protobuf-compiler libqt4-dev
  else
    sudo add-apt-repository -y ppa:beineri/opt-qt521
    sudo add-apt-repository -y ppa:kalakris/cmake
    sudo apt-get update -qq
    sudo apt-get install -y libprotobuf-dev protobuf-compiler cmake libsqlite3-dev\
      qt52base qt52webkit qt52tools qt52svg qt52multimedia
  fi
fi
