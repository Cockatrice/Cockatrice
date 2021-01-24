#!/bin/bash

# This script is to be used by the ci environment from the project root directory, do not use it from somewhere else.

# Read arguments
while [[ "$@" ]]; do
  case "$1" in
    '--')
      shift
      ;;
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
        PACKAGE_TYPE="$1"
        shift
      fi
      ;;
    '--suffix')
      shift
      if [[ $# == 0 ]]; then
        echo "::error file=$0::--suffix expects an argument"
        exit 1
      fi
      PACKAGE_SUFFIX="$1"
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
      if [[ $1 == -* ]]; then
        echo "::error file=$0::unrecognized option: $1"
        exit 3
      fi
      BUILDTYPE="$1"
      shift
      ;;
  esac
done

# Check formatting using clang-format
if [[ $CHECK_FORMAT ]]; then
  echo "::group::Run linter"
  source ./.ci/lint.sh
  echo "::endgroup::"
fi

set -e

# Setup
./servatrice/check_schema_version.sh
mkdir -p build
cd build

if [[ ! $CMAKE_BUILD_PARALLEL_LEVEL ]]; then
  CMAKE_BUILD_PARALLEL_LEVEL=2 # default machines have 2 cores
fi

# Add cmake flags
if [[ $MAKE_SERVER ]]; then
  flags+=" -DWITH_SERVER=1"
fi
if [[ $MAKE_TEST ]]; then
  flags+=" -DTEST=1"
fi
if [[ $BUILDTYPE ]]; then
  flags+=" -DCMAKE_BUILD_TYPE=$BUILDTYPE"
fi
if [[ $PACKAGE_TYPE ]]; then
  flags+=" -DCPACK_GENERATOR=$PACKAGE_TYPE"
fi

if [[ $(uname) == "Darwin" ]]; then
  # prepend ccache compiler binaries to path
  PATH="/usr/local/opt/ccache/libexec:$PATH"
  # Add qt install location when using homebrew
  flags+=" -DCMAKE_PREFIX_PATH=/usr/local/opt/qt5/"
fi

# Compile
echo "::group::Show ccache stats"
ccache --show-stats
echo "::endgroup::"

echo "::group::Configure cmake"
cmake --version
cmake .. $flags
echo "::endgroup::"

echo "::group::Build project"
cmake --build .
echo "::endgroup::"

echo "::group::Show ccache stats again"
ccache --show-stats
echo "::endgroup::"

if [[ $MAKE_TEST ]]; then
  echo "::group::Run tests"
  cmake --build . --target test
  echo "::endgroup::"
fi

if [[ $MAKE_INSTALL ]]; then
  echo "::group::Install"
  cmake --build . --target install
  echo "::endgroup::"
fi

if [[ $MAKE_PACKAGE ]]; then
  echo "::group::Create package"
  cmake --build . --target package
  echo "::endgroup::"

  if [[ $PACKAGE_SUFFIX ]]; then
    echo "::group::Update package name"
    ../.ci/name_build.sh "$PACKAGE_SUFFIX"
    echo "::endgroup::"
  fi
fi
