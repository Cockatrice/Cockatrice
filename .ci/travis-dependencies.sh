#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew update
  brew install ccache clang-format protobuf qt@5.7
fi
if [[ $TRAVIS_OS_NAME == "linux" ]] ; then
  echo Skipping... packages are installed with the Travis apt addon for sudo disabled container builds
fi