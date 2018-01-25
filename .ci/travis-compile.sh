#!/bin/bash

set -e

./servatrice/check_schema_version.sh

mkdir -p build
cd build
prefix=""

if [[ $TRAVIS_OS_NAME == "osx" ]]; then
  export PATH="/usr/local/opt/ccache/libexec:$PATH"
  prefix="-DCMAKE_PREFIX_PATH=$(echo /usr/local/opt/qt*/)"
fi
if [[ $TRAVIS_OS_NAME == "linux" ]]; then
  prefix="-DCMAKE_PREFIX_PATH=$(echo /opt/qt5*/lib/cmake/)"
  export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$(echo /opt/qt5*/lib/)"
fi

if [[ $BUILDTYPE == "Debug" ]]; then
  cmake .. -DWITH_SERVER=1 -DCMAKE_BUILD_TYPE=$BUILDTYPE $prefix -DTEST=1
  make -j2
  make test

  if [[ $TRAVIS_OS_NAME == "linux" ]]; then
    cd ..
    clang-format -i \
      common/*.h \
      common/*.cpp \
      cockatrice/src/*.h \
      cockatrice/src/*.cpp \
      oracle/src/*.h \
      oracle/src/*.cpp \
      servatrice/src/*.h \
      servatrice/src/*.cpp
    
    git clean -f
    git diff --quiet || (
      echo "*****************************************************";
      echo "***  This PR is not clean against our code style  ***";
      echo "***  Run clang-format and fix up any differences  ***";
      echo "***  Check our CONTRIBUTING.md file for details!  ***";
      echo "***                  Thank you ♥                  ***";
      echo "*****************************************************";
    )
    git diff --exit-code
  fi
fi

if [[ $BUILDTYPE == "Release" ]]; then
  cmake .. -DWITH_SERVER=1 -DCMAKE_BUILD_TYPE=$BUILDTYPE $prefix
  make package -j2
fi
