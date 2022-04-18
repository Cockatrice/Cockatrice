#!/bin/bash

# This script is to be used by the ci environment from the project root directory, do not use it from somewhere else.

# Compiles cockatrice inside of a ci environment
# --format runs the clang-format script first
# --install runs make install
# --package [<package type>] runs make package, optionally force the type
# --suffix <suffix> renames package with this suffix, requires arg
# --server compiles servatrice
# --test runs tests
# --debug or --release or <arg> sets the build type ie CMAKE_BUILD_TYPE
# --ccache uses ccache and shows stats
# --dir <dir> sets the name of the build dir, default is "build"
# uses env: BUILDTYPE CHECK_FORMAT MAKE_INSTALL MAKE_PACKAGE PACKAGE_TYPE PACKAGE_SUFFIX MAKE_SERVER MAKE_TEST USE_CCACHE BUILD_DIR (correspond to args: <buildtype>/--debug/--release --format --install --package <package type> --suffix <suffix> --server --test --ccache --dir <dir>)
# exitcode: 1 for failure, 3 for invalid arguments
LINT_SCRIPT=".ci/lint_cpp.sh"

# Read arguments
while [[ $# != 0 ]]; do
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
        exit 3
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
    '--ccache')
      USE_CCACHE=1
      shift
      ;;
    '--dir')
      shift
      if [[ $# == 0 ]]; then
        echo "::error file=$0::--dir expects an argument"
        exit 3
      fi
      BUILD_DIR="$1"
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
  source "$LINT_SCRIPT"
  echo "::endgroup::"
fi

set -e

# Setup
./servatrice/check_schema_version.sh
if [[ ! $BUILD_DIR ]]; then
  BUILD_DIR="build"
fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

if [[ ! $CMAKE_BUILD_PARALLEL_LEVEL ]]; then
  CMAKE_BUILD_PARALLEL_LEVEL=2 # default machines have 2 cores
fi

# Add cmake flags
flags=()
if [[ $MAKE_SERVER ]]; then
  flags+=("-DWITH_SERVER=1")
fi
if [[ $MAKE_TEST ]]; then
  flags+=("-DTEST=1")
fi
if [[ ! $BUILDTYPE ]]; then
  BUILDTYPE=Release
fi
flags+=("-DCMAKE_BUILD_TYPE=$BUILDTYPE")
if [[ $PACKAGE_TYPE ]]; then
  flags+=("-DCPACK_GENERATOR=$PACKAGE_TYPE")
fi

if [[ $(uname) == "Darwin" ]]; then
  if [[ $USE_CCACHE ]]; then
    # prepend ccache compiler binaries to path
    PATH="/usr/local/opt/ccache/libexec:$PATH"
  fi
  # Add qt install location when using homebrew
  flags+=("-DCMAKE_PREFIX_PATH=/usr/local/opt/qt5/")
fi

# Compile
if [[ $USE_CCACHE ]]; then
  echo "::group::Show ccache stats"
  ccache --show-stats
  echo "::endgroup::"
fi

echo "::group::Configure cmake"
cmake --version
cmake .. "${flags[@]}"
echo "::endgroup::"

echo "::group::Build project"
cmake --build . --config "$BUILDTYPE"
echo "::endgroup::"

if [[ $USE_CCACHE ]]; then
  echo "::group::Show ccache stats again"
  ccache --show-stats
  echo "::endgroup::"
fi

if [[ $MAKE_TEST ]]; then
  echo "::group::Run tests"
  ctest -C "$BUILDTYPE"
  echo "::endgroup::"
fi

if [[ $MAKE_INSTALL ]]; then
  echo "::group::Install"
  cmake --build . --target install --config "$BUILDTYPE"
  echo "::endgroup::"
fi

if [[ $MAKE_PACKAGE ]]; then
  echo "::group::Create package"
  cmake --build . --target package --config "$BUILDTYPE"
  echo "::endgroup::"

  if [[ $PACKAGE_SUFFIX ]]; then
    echo "::group::Update package name"
    ../.ci/name_build.sh "$PACKAGE_SUFFIX"
    echo "::endgroup::"
  fi
fi
