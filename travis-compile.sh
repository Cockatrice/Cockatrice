#!/bin/bash

set -e

mkdir build
cd build
prefix=""
if [[ $TRAVIS_OS_NAME == "osx" && $QT4 == 0 ]]; then
  prefix="-DCMAKE_PREFIX_PATH=`echo /usr/local/Cellar/qt5/5.*/`"
fi
if [[ $TRAVIS_OS_NAME == "linux" && $QT4 == 0 ]]; then
  prefix="-DCMAKE_PREFIX_PATH=`echo /opt/qt5*/lib/cmake/`"
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`echo /opt/qt5*/lib/`
fi

if [[ $BUILDTYPE == "Debug" ]]; then
	cmake .. -DWITH_SERVER=1 -DTEST=1 -DCMAKE_BUILD_TYPE=$BUILDTYPE -DWITH_QT4=$QT4 $prefix
	make -j2
	make test
else
	cmake .. -DWITH_SERVER=1 -DCMAKE_BUILD_TYPE=$BUILDTYPE -DWITH_QT4=$QT4 $prefix
	make package -j2
fi