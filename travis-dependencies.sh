#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew update
  if (( QT4 )); then
    brew install qt protobuf libgcrypt
  else
    brew install qt5 protobuf libgcrypt
  fi
  brew unlink cmake
  brew upgrade cmake
else
  if (( QT4 )); then
    sudo apt-get update -qq
    sudo apt-get install -y qtmobility-dev libqt4-dev
  else
    sudo add-apt-repository -y ppa:george-edison55/precise-backports
    sudo add-apt-repository -y ppa:beineri/opt-qt521
    sudo apt-get update -qq
    sudo apt-get install -y libsqlite3-dev\
      qt52base qt52webkit qt52tools qt52svg qt52multimedia
  fi
  sudo apt-get install -y cmake libgtest-dev libprotobuf-dev protobuf-compiler
  cd /usr/src/gtest && sudo cmake . && sudo cmake --build . && sudo mv libg* /usr/local/lib/ ; cd -
fi
