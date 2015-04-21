#!/bin/bash

set -e

mkdir build
cd build
prefix=""
if [[ $TRAVIS_OS_NAME == "osx" && $QT4 == 0 ]]; then
  prefix="-DCMAKE_PREFIX_PATH=/usr/local/Cellar/qt5/5.4.1/"
fi
if [[ $TRAVIS_OS_NAME == "linux" && $QT4 == 0 ]]; then
  prefix="-DCMAKE_PREFIX_PATH=/opt/qt52/lib/cmake/"
fi
cmake .. -DWITH_SERVER=1 -DCMAKE_BUILD_TYPE=Debug -DWITH_QT4=$QT4 $prefix
make -j2
