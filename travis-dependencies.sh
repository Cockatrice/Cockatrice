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
  # common prerequisites
  sudo add-apt-repository -y ppa:smspillaz/cmake-master
  sudo apt-get update -qq
  sudo apt-get -y purge cmake
  sudo apt-get install -y libprotobuf-dev protobuf-compiler cmake bc

  if (( QT4 )); then
    # qt4 prerequisites
    sudo apt-get install -y qtmobility-dev libqt4-dev
  else
    # qt5 prerequisites
    sudo apt-get install -y libprotobuf-dev protobuf-compiler \
      qt5-default qttools5-dev qttools5-dev-tools \
      qtmultimedia5-dev libqt5multimedia5-plugins libqt5svg5-dev libqt5sql5-mysql
  fi

  # prerequisites for tests
  if [[ $BUILDTYPE == "Debug" ]]; then
    sudo apt-get install -y libgtest-dev

    sudo mkdir /usr/src/gtest/build
    cd /usr/src/gtest/build
    sudo cmake .. -DBUILD_SHARED_LIBS=1
    sudo make -j2
    sudo ln -s /usr/src/gtest/build/libgtest.so /usr/lib/libgtest.so
    sudo ln -s /usr/src/gtest/build/libgtest_main.so /usr/lib/libgtest_main.so
    cd -
  fi
fi
