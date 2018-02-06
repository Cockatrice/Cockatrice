#!/bin/bash

set -e

if [[ $TRAVIS == true ]]; then

  ./servatrice/check_schema_version.sh
  
  mkdir -p build
  cd build
  prefix=""
  
  if [[ $TRAVIS_OS_NAME == "linux" ]]; then
    prefix="-DCMAKE_PREFIX_PATH=$(echo /opt/qt5*/lib/cmake/)"
  fi
  if [[ $TRAVIS_OS_NAME == "osx" ]]; then
    prefix="-DCMAKE_PREFIX_PATH=$(echo /usr/local/opt/qt*/)"
    export PATH="/usr/local/opt/ccache/bin:$PATH"
  fi
  
  if [[ $BUILDTYPE == "Debug" ]]; then
    cmake .. -DWITH_SERVER=1 -DCMAKE_BUILD_TYPE=$BUILDTYPE $prefix -DTEST=1
    make -j2
    make test
  fi
  if [[ $BUILDTYPE == "Release" ]]; then
    cmake .. -DWITH_SERVER=1 -DCMAKE_BUILD_TYPE=$BUILDTYPE $prefix
    make package -j2
  fi

else
  echo "Doing nothing. This script is supposed to be run by Travis CI only!"
fi
