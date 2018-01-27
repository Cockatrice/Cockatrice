#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew install --force qt@5.7
  brew install protobuf clang-format ccache
fi
if [[ $TRAVIS_OS_NAME == "linux" ]] ; then
  echo Skipping... packages are installed with the Travis apt addon for sudo disabled container builds
fi