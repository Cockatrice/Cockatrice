#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew update > /dev/null
  if (( QT4 )); then
    brew install qt protobuf libgcrypt > /dev/null
  else
    brew install qt5 protobuf libgcrypt > /dev/null
  fi
  brew unlink cmake
  brew upgrade cmake
else
  sudo add-apt-repository -y ppa:george-edison55/precise-backports
  if (( QT4 )); then
    sudo apt-get update -qq
    sudo apt-get install -y qtmobility-dev libqt4-dev
  else
    sudo add-apt-repository -y ppa:beineri/opt-qt541
    sudo apt-get update -qq
    sudo apt-get install -y libsqlite3-dev\
      qt54base qt54webkit qt54tools qt54svg qt54multimedia
  fi
  sudo apt-get install -y cmake cmake-data libgtest-dev libprotobuf-dev protobuf-compiler
  sudo mkdir /usr/src/gtest/build
  cd /usr/src/gtest/build
  sudo cmake .. -DBUILD_SHARED_LIBS=1
  sudo make -j2
  sudo ln -s /usr/src/gtest/build/libgtest.so /usr/lib/libgtest.so
  sudo ln -s /usr/src/gtest/build/libgtest_main.so /usr/lib/libgtest_main.so
  cd -
fi
