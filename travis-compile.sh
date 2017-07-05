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
	cmake .. -DWITH_SERVER=1 -DTEST=1 -DCMAKE_BUILD_TYPE=$BUILDTYPE $prefix
	make -j2
	make test
else
	cmake .. -DWITH_SERVER=1 -DCMAKE_BUILD_TYPE=$BUILDTYPE $prefix
	make package -j2
fi
