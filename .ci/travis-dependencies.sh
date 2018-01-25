#!/bin/bash

if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
  brew install ccache   # enable caching on mac (PATH only set in travis-compile.sh)
  brew install --force qt@5.7
  brew install protobuf
fi
if [[ $TRAVIS_OS_NAME == "linux" ]] ; then
  echo Skipping... packages are installed with the Travis apt addon for sudo disabled container builds
fi