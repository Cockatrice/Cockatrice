#!/bin/bash

# This script is to be used by the CI environment from the project root directory, do not use it from somewhere else.

# Compiles Cockatrice inside a CI environment
#
# Supported arguments:
#  --install                           Runs cmake install
#  --package [<package type>]          Runs cmake package (optionally: define the type)
#  --suffix <suffix>                   Renames package with provided suffix
#  --server                            Compiles Servatrice
#  --test                              Runs tests
#  --debug / --release                 Sets the build type (CMAKE_BUILD_TYPE)
#  --ccache [<size>]                   Uses ccache and shows stats, optionally provide size
#  --evict-ccache <age>                Runs ccache eviction based on given age after build
#  --dir <dir>                         Sets the name of the build dir, default is "build"
#  --cmake-generator <generator>       Sets the CMake generator (CMAKE_GENERATOR)
#  --target-macos-version <version>    Sets the min OS version - only used for macOS builds
#
# Used environment variables:
#  BUILDTYPE
#  MAKE_INSTALL
#  MAKE_PACKAGE
#  PACKAGE_TYPE
#  PACKAGE_SUFFIX
#  MAKE_SERVER
#  MAKE_NO_CLIENT
#  MAKE_TEST
#  USE_CCACHE
#  CCACHE_SIZE
#  CCACHE_EVICTION_AGE
#  BUILD_DIR
#  CMAKE_GENERATOR
#  TARGET_MACOS_VERSION
#
# Exitcodes:
#  1 --> Failure
#  3 --> Invalid argument(s)

#TODO order above comment, check & add
# there are also missing ones like "--no-client" and "--vcpkg"
# arguments and env variables are doubled and are not used uniformly between Linux and macOS/Windows builds which adds complexity

# Read arguments
while [[ $# != 0 ]]; do
  case "$1" in
    '--')
      shift
      ;;
    '--install')
      MAKE_INSTALL=1
      shift
      ;;
    '--package')
      MAKE_PACKAGE=1
      shift
      if [[ $# != 0 && ${1:0:1} != - ]]; then
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
    '--no-client')
      MAKE_NO_CLIENT=1
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
      if [[ $# != 0 && ${1:0:1} != - ]]; then
        CCACHE_SIZE="$1"
        shift
      fi
      ;;
    '--evict-ccache')
      shift
      if [[ $# == 0 ]]; then
        echo "::error file=$0::--evict-ccache expects an argument"
        exit 3
      fi
      CCACHE_EVICTION_AGE=$1
      shift
      ;;
    '--vcpkg')
      USE_VCPKG=1
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
    '--cmake-generator')
      shift
      if [[ $# == 0 ]]; then
        echo "::error file=$0::--cmake-generator expects an argument"
        exit 3
      fi
      export CMAKE_GENERATOR=$1
      shift
      ;;
    '--target-macos-version')
      shift
      if [[ $# == 0 ]]; then
        echo "::error file=$0::--target-macos-version expects an argument"
        exit 3
      fi
      TARGET_MACOS_VERSION="$1"
      shift
      ;;
    *)
      echo "::error file=$0::unrecognized option: $1"
      exit 3
      ;;
  esac
done

set -e

# Setup
./servatrice/check_schema_version.sh
if [[ ! $BUILDTYPE ]]; then
  BUILDTYPE=Release
fi

if [[ ! $BUILD_DIR ]]; then
  BUILD_DIR="build"
fi
# TODO check BUILD_DIR logic

# Add CMake flags
flags=("-DCMAKE_BUILD_TYPE=$BUILDTYPE")
if [[ $MAKE_SERVER ]]; then
  flags+=("-DWITH_SERVER=1")
fi

if [[ $MAKE_NO_CLIENT ]]; then
  flags+=("-DWITH_CLIENT=0" "-DWITH_ORACLE=0")
fi

if [[ $MAKE_TEST ]]; then
  flags+=("-DTEST=1")
fi

if [[ $USE_CCACHE ]]; then
  flags+=("-DUSE_CCACHE=1")
  if [[ $CCACHE_SIZE ]]; then
    # Note, this setting persists after running the script
    ccache --max-size "$CCACHE_SIZE"
  fi
fi

if [[ $PACKAGE_TYPE ]]; then
  flags+=("-DCPACK_GENERATOR=$PACKAGE_TYPE")
fi

if [[ $USE_VCPKG ]]; then
  flags+=("-DUSE_VCPKG=1")
fi

# Add CMake --build flags
buildflags=(--config "$BUILDTYPE")

# Prepare compilation
if [[ $RUNNER_OS == macOS ]]; then
# TODO qtdir

  # QTDIR is needed for macOS since we actually only use the cached thin Qt binaries instead of the install-qt-action,
  # which sets a few environment variables
  if QTDIR=$(find "$GITHUB_WORKSPACE/Qt" -depth -maxdepth 2 -name macos -type d -print -quit); then
    echo "found QTDIR at $QTDIR"
  else
    echo "could not find QTDIR!"
    exit 2
  fi

  # The qtdir is located at Qt/<qtversion>/macos
  # We use "find" to get the first subfolder with the name "macos"
  # This works independent of the Qt version as there should be only one version installed on the runner at a time
  export QTDIR

  if [[ $TARGET_MACOS_VERSION ]]; then
    # CMAKE_OSX_DEPLOYMENT_TARGET is a vanilla CMake flag needed to compile to target macOS version
    flags+=("-DCMAKE_OSX_DEPLOYMENT_TARGET=$TARGET_MACOS_VERSION")

    # vcpkg dependencies need a vcpkg triplet file to compile to the target macOS version
    # An easy way is to copy the x64-osx.cmake file and modify it
    triplets_dir="/tmp/cmake/triplets"
    triplet_version="custom-triplet"
    triplet_file="$triplets_dir/$triplet_version.cmake"
    arch=$(uname -m)

    if [[ $arch == x86_64 ]]; then
      arch="x64"
    fi

    mkdir -p "$triplets_dir"
    triplet_source="../vcpkg/triplets/$arch-osx.cmake"

    if [[ ! -f "$triplet_source" ]]; then
      triplet_source="../vcpkg/triplets/community/$arch-osx.cmake"
    fi

    cp "$triplet_source" "$triplet_file"
    echo "set(VCPKG_CMAKE_SYSTEM_VERSION $TARGET_MACOS_VERSION)" >>"$triplet_file"
    echo "set(VCPKG_OSX_DEPLOYMENT_TARGET $TARGET_MACOS_VERSION)" >>"$triplet_file"
    flags+=("-DVCPKG_OVERLAY_TRIPLETS=$triplets_dir")
    flags+=("-DVCPKG_HOST_TRIPLET=$triplet_version")
    flags+=("-DVCPKG_TARGET_TRIPLET=$triplet_version")
    echo "::group::Generated triplet $triplet_file"
    cat "$triplet_file"
    echo "::endgroup::"
  fi

  echo "::group::Signing Certificate"

  if [[ -n "$MACOS_CERTIFICATE_NAME" ]]; then
    echo "$MACOS_CERTIFICATE" | base64 --decode >"certificate.p12"
    security create-keychain -p "$MACOS_CI_KEYCHAIN_PWD" build.keychain
    security default-keychain -s build.keychain
    security set-keychain-settings -t 3600 -l build.keychain
    security unlock-keychain -p "$MACOS_CI_KEYCHAIN_PWD" build.keychain
    security import certificate.p12 -k build.keychain -P "$MACOS_CERTIFICATE_PWD" -T /usr/bin/codesign
    security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$MACOS_CI_KEYCHAIN_PWD" build.keychain
    echo "macOS signing certificate successfully imported and keychain configured."
  else
    echo "No signing certificate configured. Skipping set up of keychain in macOS environment."
  fi

  echo "::endgroup::"

  if [[ $MAKE_PACKAGE ]]; then
    # Workaround https://github.com/actions/runner-images/issues/7522
    # Have hdiutil repeat the command 10 times in hope of success
    hdiutil_script="/tmp/hdiutil.sh"
    # shellcheck disable=SC2016
    echo '#!/bin/bash
    i=0
    while ! hdiutil "$@"; do
      if (( ++i >= 10 )); then
        echo "Error: hdiutil failed $i times!" >&2
        break
      fi
      sleep 1
    done' >"$hdiutil_script"
    chmod +x "$hdiutil_script"
    flags+=(-DCPACK_COMMAND_HDIUTIL="$hdiutil_script")
  fi

elif [[ $RUNNER_OS == Windows ]]; then
  # Enable MSBuild switches for MTT, see https://devblogs.microsoft.com/cppblog/improved-parallelism-in-msbuild/
  # and https://devblogs.microsoft.com/cppblog/cpp-build-throughput-investigation-and-tune-up/#multitooltask-mtt
  buildflags+=(-- -p:UseMultiToolTask=true -p:EnableClServerMode=true)
fi

# Pre-build ccache
if [[ $USE_CCACHE ]]; then
  echo "::group::Clear ccache stats"
  # https://ccache.dev/manual/4.13.6.html#_command_line_options
  ccache --version
  ccache --show-config
  ccache --show-stats    # remove again
  ccache --show-log-stats    # helpful?
  ccache --zero-stats    # zero former cache statistics (but not the configuration options)
  ccache --show-stats    # helpful?
  echo "::endgroup::"
fi

# Configure CMake
echo "::group::Configure CMake"
cmake --version
echo "Running CMake configuration with following flags: ${flags[*]}"
cmake -S . -B "$BUILD_DIR" "${flags[@]}"
# cmake -S .. -B "$BUILD_DIR" "${flags[@]}"
echo "::endgroup::"

# Build
echo "::group::Build project"
echo "Running CMake with following build flags: ${buildflags[*]}"
cmake --build "$BUILD_DIR" "${buildflags[@]}"
echo "::endgroup::"

# Post-build ccache
if [[ $USE_CCACHE ]]; then

  if [[ $CCACHE_EVICTION_AGE ]]; then
    echo "::group::evict ccache files older than $CCACHE_EVICTION_AGE"
    ccache --evict-older-than "$CCACHE_EVICTION_AGE"
    echo "::endgroup::"
  fi

  echo "::group::Show ccache stats"
  ccache --verify    # remove again
  ccache --show-stats --verbose    # too verbose?
  ccache --show-compression    # helpful?
  echo "::endgroup::"

elif [[ $CCACHE_EVICTION_AGE ]]; then
  echo "::error file=$0::ccache eviction is enabled while ccache is disabled!"
fi

# [macOS] Inspect binaries
if [[ $RUNNER_OS == macOS ]]; then

  echo "::group::Inspect Mach-O binaries"

  for app in cockatrice oracle servatrice; do
    binary="$GITHUB_WORKSPACE/build/$app/$app.app/Contents/MacOS/$app"
    echo "Inspecting $app..."
    vtool -show-build "$binary"
    file "$binary"
    lipo -info "$binary"
    echo ""

  done
  echo "::endgroup::"
fi

 # Test
if [[ $MAKE_TEST ]]; then
  echo "::group::Run tests"
  ctest --version
  ctest --build-config "$BUILDTYPE" --test-dir "$BUILD_DIR" --output-on-failure
  echo "::endgroup::"
fi

# Install
if [[ $MAKE_INSTALL ]]; then
  echo "::group::Install"
  cmake --build "$BUILD_DIR" --target install --config "$BUILDTYPE"
  # cmake --install "$BUILD_DIR" --config "$BUILDTYPE"
  echo "::endgroup::"
fi

# Package
if [[ $MAKE_PACKAGE ]]; then
  echo "::group::Create package"
  cpack --version
  cmake --build "$BUILD_DIR" --target package --config "$BUILDTYPE"
  echo "::endgroup::"

  if [[ $PACKAGE_SUFFIX ]]; then
    echo "::group::Update package name"
    BUILD_DIR="$BUILD_DIR" .ci/name_build.sh "$PACKAGE_SUFFIX"
    echo "::endgroup::"
  fi
fi
