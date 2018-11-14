#!/bin/bash

# This script is to be used in .travis.yaml from the project root directory, do not use it from somewhere else.

set -ex

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
    *)
      BUILDTYPE="$1"
      shift
      ;;
  esac
done

./servatrice/check_schema_version.sh

# Check formatting using clang-format
if [[ $CHECK_FORMAT ]]; then
  if ! files="$(./clangify.sh --names)"; then
    cat <<EOM
*****************************************************
***  This PR does not comply with our code style  ***
***  Run ./clangify.sh to fix up any differences  ***
***  Check our CONTRIBUTING.md file for details!  ***
***                  Thank you ♥                  ***
*****************************************************
The following files should be reformatted:
$files
EOM
    exit 2
  fi
fi

mkdir -p build
cd build

# Add flags
if [[ $MAKE_SERVER ]]; then
  flags+=" -DWITH_SERVER=1"
fi
if [[ $MAKE_TEST ]]; then
  flags+=" -DTEST=1"
fi
if [[ $BUILDTYPE ]]; then
  flags+=" -DCMAKE_BUILD_TYPE=$BUILDTYPE"
fi

# Add qt install location when using brew
if [[ $(uname) == "Darwin" ]]; then
  PATH="/usr/local/opt/ccache/bin:$PATH"
  flags+=" -DCMAKE_PREFIX_PATH=/usr/local/opt/qt5/"
fi

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
