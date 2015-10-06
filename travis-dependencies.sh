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
  sudo apt-get update -qq
  sudo apt-get install -y libprotobuf-dev protobuf-compiler cmake

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
    if [[ $DIST == "precise" ]]; then
      sudo add-apt-repository -y ppa:george-edison55/precise-backports
      sudo apt-get update -qq
      sudo apt-get install -y cmake cmake-data libgtest-dev
    else
      sudo add-apt-repository -y ppa:george-edison55/cmake-3.x
      sudo apt-get update -qq
      sudo apt-get install -y cmake cmake-extras libgtest-dev
    fi

    sudo mkdir /usr/src/gtest/build
    cd /usr/src/gtest/build
    sudo cmake .. -DBUILD_SHARED_LIBS=1
    sudo make -j2
    sudo ln -s /usr/src/gtest/build/libgtest.so /usr/lib/libgtest.so
    sudo ln -s /usr/src/gtest/build/libgtest_main.so /usr/lib/libgtest_main.so
    cd -
  fi
fi
