#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew update
  if (( QT4 )); then
    brew install qt protobuf libgcrypt
  else
    brew install qt5 protobuf libgcrypt
  fi
fi