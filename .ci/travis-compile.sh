#!/bin/bash

# This script is to be used in .travis.yaml from the project root directory, do not use it from somewhere else.

set -e

# Read arguments
while [[ "$@" ]]; do
  case "$1" in
    '--format')
      CHECK_FORMAT=1
      shift
      ;;
    '--install')
      MAKE_INSTALL=1
      shift
      ;;
    '--package')
      MAKE_PACKAGE=1
      shift
      ;;
    '--server')
      MAKE_SERVER=1
      shift
      ;;
    '--test')
      MAKE_TEST=1
      shift
      ;;
    '--debug')
      BUILDTYPE="Debug"
      shift
      ;;
    '--release')
      BUILDTYPE="Release"
      shift
      ;;
    *)
      BUILDTYPE="$1"
      shift
      ;;
  esac
done

# Check formatting using clang-format
if [[ $CHECK_FORMAT ]]; then
  echo "Checking your code using clang-format..."
  if ! diff="$(./clangify.sh --color-diff --cf-version)"; then
    cat <<EOM
***********************************************************
***                                                     ***
***    Your code does not comply with our styleguide.   ***
***                                                     ***
***  Please correct it or run the "clangify.sh" script. ***
***  Then commit and push those changes to this branch. ***
***   Check our CONTRIBUTING.md file for more details.  ***
***                                                     ***
***                     Thank you ♥                     ***
***                                                     ***
***********************************************************

The following changes should be made:
$diff

Exiting...
EOM
    exit 2
  else
    echo "Thank you for complying with our code standards."
  fi
fi

# Setup
./servatrice/check_schema_version.sh
mkdir -p build
cd build

# Add cmake flags
if [[ $MAKE_SERVER ]]; then
  flags+=" -DWITH_SERVER=1"
fi
if [[ $MAKE_TEST ]]; then
  flags+=" -DTEST=1"
  BUILDTYPE="Debug" # test requires buildtype Debug
fi
if [[ $BUILDTYPE ]]; then
  flags+=" -DCMAKE_BUILD_TYPE=$BUILDTYPE"
fi

# Add qt install location when using brew
if [[ $(uname) == "Darwin" ]]; then
  PATH="/usr/local/opt/ccache/bin:$PATH"
  flags+=" -DCMAKE_PREFIX_PATH=/usr/local/opt/qt5/"
fi

# Compile
cmake --version
cmake .. $flags
make -j2

if [[ $MAKE_TEST ]]; then
  make test
fi

if [[ $MAKE_INSTALL ]]; then
  make install
fi

if [[ $MAKE_PACKAGE ]]; then
  make package
fi
