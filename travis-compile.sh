#!/bin/bash

set -e

mkdir build
cd build
prefix=""
if [[ $TRAVIS_OS_NAME == "osx" && $QT4 == 0 ]]; then
  prefix="-DCMAKE_PREFIX_PATH=/usr/local/Cellar/qt5/5.3.2/"
fi
cmake .. -DWITH_SERVER=1 -DWITH_QT4=$QT4 $prefix
make
