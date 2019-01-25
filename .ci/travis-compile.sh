#!/bin/bash

# This script is to be used in .travis.yaml from the project root directory, do not use it from somewhere else.

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
      if [[ $# != 0 && $1 != -* ]]; then
        PACKAGE_NAME="$1"
        shift
        if [[ $# != 0 && $1 != -* ]]; then
          PACKAGE_TYPE="$1"
          shift
        fi
      fi
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
      if [[ $1 == -* ]]; then
        echo "unrecognized option: $1"
        exit 3
      fi
      BUILDTYPE="$1"
      shift
      ;;
  esac
done

# Check formatting using clang-format
if [[ $CHECK_FORMAT ]]; then
  echo "Checking your code using clang-format..."
  diff="$(./clangify.sh --diff --cf-version)"
  err=$?
  case $err in
    1)
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

Used clang-format version:
${diff%%
*}

The following changes should be made:
${diff#*
}

Exiting...
EOM
      exit 2
      ;;
    0)
      echo "Thank you for complying with our code standards."
      ;;
    *)
      echo "Something went wrong in our formatting checks: clangify returned $err" >&2
      ;;
  esac
fi

set -e

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
if [[ $PACKAGE_TYPE ]]; then
  flags+=" -DCPACK_GENERATOR=$PACKAGE_TYPE"
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
  if [[ $PACKAGE_NAME ]]; then
    found=$(find . -maxdepth 1 -type f -name "Cockatrice-*.*" -print -quit)
    path=${found%/*}
    file=${found##*/}
    if [[ ! $file ]]; then
      echo "could not find package" >&2
      exit 1
    fi
    mv "$path/$file" "$path/${file%.*}-$PACKAGE_NAME.${file##*.}"
  fi
fi
