#!/usr/bin/env bash

# Compiles Cockatrice inside GitHub Actions CI, run only from project root directory (GITHUB_WORKSPACE)
#
# Build flow and script structure:
#  - Read options
#  - Pre-checks (CI, Servatrice schema)
#  - Prepare compilation (CMake options, OS-specific configuration)
#  - Pre-buld (ccache)
#  - Build (Configure, Compile)
#  - Post-build (ccache, Inspect, Test, Install, Package)
#
# Supported command-line options:
#  --ccache [<size>]                   Use ccache (USE_CCACHE) (optionally: provide cache size, e.g. "500M" or "1G" (CCACHE_SIZE))
#  --cmake-generator <generator>       Sets the CMake generator (CMAKE_GENERATOR), e.g. "Ninja", "Visual Studio 18 2026"
#  --debug                             Sets the build type (BUILDTYPE --> CMAKE_BUILD_TYPE)
#  --dir <dir>                         Sets the name of the build dir (BUILD_DIR)
#  --evict-ccache <age>                Evicts compiler cache older than <age> after build (CCACHE_EVICTION_AGE), e.g. "7d"
#  --install                           Runs cmake install (MAKE_INSTALL)
#  --no-client                         Compiles without Cockatrice and Oracle (MAKE_NO_CLIENT)
#  --package [<package type>]          Runs cmake package (MAKE_PACKAGE) (optionally: define type (PACKAGE_TYPE)), e.g. "DEB" [Linux only?]
#  --release                           Sets the build type (BUILDTYPE --> CMAKE_BUILD_TYPE)
#  --server                            Compiles Servatrice (MAKE_SERVER)
#  --suffix <suffix>                   Renames package with provided suffix (PACKAGE_SUFFIX), e.g. "-macOS15"
#  --target-macos-version <version>    Sets the min OS version (TARGET_MACOS_VERSION), e.g. "14" [macOS only]
#  --test                              Runs tests (MAKE_TEST)
#  --vcpkg                             Use vcpkg package manager to resolve dependencies (USE_VCPKG)
#
# Exit codes:
#  1 --> Failure
#  3 --> Invalid argument(s)

set -euo pipefail


### Initialize configuration variables:
# Precedence: command-line options > environment variables > built-in script defaults
BUILDTYPE="${BUILDTYPE:-Release}"                   # See "--debug" and "--release" flags (defaults to "Release" if omitted)
BUILD_DIR="${BUILD_DIR:-build}"                     # See value for "--dir" option (defaults to "build" if omitted)
CCACHE_EVICTION_AGE="${CCACHE_EVICTION_AGE:-}"      # See value for "--evict-ccache" option
CCACHE_SIZE="${CCACHE_SIZE:-}"                      # See value for "--ccache" option
CMAKE_GENERATOR="${CMAKE_GENERATOR:-}"              # See "--cmake-generator" option
MAKE_INSTALL="${MAKE_INSTALL:-0}"                   # See "--install" flag
MAKE_NO_CLIENT="${MAKE_NO_CLIENT:-0}"               # See "--no-client" flag
MAKE_PACKAGE="${MAKE_PACKAGE:-0}"                   # See "--package" option
MAKE_SERVER="${MAKE_SERVER:-0}"                     # See "--server" flag
MAKE_TEST="${MAKE_TEST:-0}"                         # See "--test" flag
PACKAGE_SUFFIX="${PACKAGE_SUFFIX:-}"                # See "--suffix" option
PACKAGE_TYPE="${PACKAGE_TYPE:-}"                    # See value for "--package" option
TARGET_MACOS_VERSION="${TARGET_MACOS_VERSION:-}"    # See "--target-macos-version" option
USE_CCACHE="${USE_CCACHE:-0}"                       # See "--ccache" option
USE_VCPKG="${USE_VCPKG:-0}"                         # See "--vcpkg" flag


### Read options
while [[ $# != 0 ]]; do
  case "$1" in

    # Flags (no argument)
    '--debug')
      BUILDTYPE="Debug"
      shift
      ;;
    '--install')
      MAKE_INSTALL=1
      shift
      ;;
    '--no-client')
      MAKE_NO_CLIENT=1
      shift
      ;;
    '--release')
      BUILDTYPE="Release"
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
    '--vcpkg')
      USE_VCPKG=1
      shift
      ;;

    # Options (accept an argument)
    '--ccache')
      USE_CCACHE=1
      shift
      if [[ $# != 0 && ${1:0:1} != - ]]; then
        CCACHE_SIZE="$1"
        shift
      fi
      ;;
    '--cmake-generator')
      shift
      if [[ $# == 0 ]]; then
        echo "::error file=$0::--cmake-generator expects an argument"
        exit 3
      fi
      CMAKE_GENERATOR="$1"
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
    '--evict-ccache')
      shift
      if [[ $# == 0 ]]; then
        echo "::error file=$0::--evict-ccache expects an argument"
        exit 3
      fi
      CCACHE_EVICTION_AGE="$1"
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
    '--target-macos-version')
      shift
      if [[ $# == 0 ]]; then
        echo "::error file=$0::--target-macos-version expects an argument"
        exit 3
      fi
      TARGET_MACOS_VERSION="$1"
      shift
      ;;

    # Positional arguments are not supported
    '--')
	  shift
      if [[ $# != 0 ]]; then
        echo "::error file=$0::Unexpected positional arguments: $*"
        exit 3
      fi
      break
      ;;

    # Unknown options
    *)
      echo "::error file=$0::Unrecognized option: $1"
      exit 3
      ;;
  esac
done


### Pre-checks

# CI context (GitHub Actions provided environment variable)
if [[ -z ${RUNNER_OS:-} ]]; then
  echo "::notice::RUNNER_OS (GitHub Actions CI environment variable) is not available and OS-specific build configuration will be skipped"
fi

# Schema version consistency
if [[ $MAKE_SERVER == 1 ]]; then
  ./servatrice/check_schema_version.sh
fi


### Prepare compilation

# CMake options
flags=("-DCMAKE_BUILD_TYPE=$BUILDTYPE")

if [[ -n $CMAKE_GENERATOR ]]; then
  flags+=(-G "$CMAKE_GENERATOR")
fi
if [[ $MAKE_NO_CLIENT == 1 ]]; then
  flags+=("-DWITH_CLIENT=0" "-DWITH_ORACLE=0")
fi
if [[ $MAKE_SERVER == 1 ]]; then
  flags+=("-DWITH_SERVER=1")
fi
if [[ $MAKE_TEST == 1 ]]; then
  flags+=("-DTEST=1")
fi
if [[ -n $PACKAGE_TYPE ]]; then
  flags+=("-DCPACK_GENERATOR=$PACKAGE_TYPE")
fi
if [[ $USE_CCACHE == 1 ]]; then
  flags+=("-DUSE_CCACHE=1")
  if [[ -n $CCACHE_SIZE ]]; then
    # Note: Setting persists after running the script
    ccache --max-size "$CCACHE_SIZE"
  fi
fi
if [[ $USE_VCPKG == 1 ]]; then
  flags+=("-DUSE_VCPKG=1")
fi

# CMake --build options
buildflags=(--config "$BUILDTYPE")

if [[ ${RUNNER_OS:-} == "Windows" ]]; then
  # Enable MSBuild switches for MTT, see https://devblogs.microsoft.com/cppblog/improved-parallelism-in-msbuild/
  # and https://devblogs.microsoft.com/cppblog/cpp-build-throughput-investigation-and-tune-up/#multitooltask-mtt
  buildflags+=(-- -p:UseMultiToolTask=true -p:EnableClServerMode=true)
fi

# Other OS-specific configuration (GitHub Actions runner)
if [[ ${RUNNER_OS:-} == "macOS" ]]; then
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

  if [[ -n $TARGET_MACOS_VERSION ]]; then
    # CMAKE_OSX_DEPLOYMENT_TARGET is a vanilla CMake option needed to compile to target macOS version
    flags+=("-DCMAKE_OSX_DEPLOYMENT_TARGET=$TARGET_MACOS_VERSION")

    if [[ $USE_VCPKG == 1 ]]; then
	  echo "vcpkg triplet"
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
	  triplet_source="./vcpkg/triplets/$arch-osx.cmake"

      if [[ ! -f "$triplet_source" ]]; then
        triplet_source="./vcpkg/triplets/community/$arch-osx.cmake"
      fi

      cp "$triplet_source" "$triplet_file"
      echo "set(VCPKG_CMAKE_SYSTEM_VERSION $TARGET_MACOS_VERSION)" >>"$triplet_file"
      echo "set(VCPKG_OSX_DEPLOYMENT_TARGET $TARGET_MACOS_VERSION)" >>"$triplet_file"
      flags+=("-DVCPKG_OVERLAY_TRIPLETS=$triplets_dir")
      flags+=("-DVCPKG_HOST_TRIPLET=$triplet_version")
      flags+=("-DVCPKG_TARGET_TRIPLET=$triplet_version")
      echo "::endgroup::"

	  echo "::group::Generated triplet $triplet_file"
      cat "$triplet_file"
      echo "::endgroup::"
    fi
  fi

  echo "::group::Setup signing certificate"
  if [[ -n $MACOS_CERTIFICATE_NAME ]]; then
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

  if [[ $MAKE_PACKAGE == 1 ]]; then
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

elif [[ ${RUNNER_OS:-} == "Windows" ]]; then
  :

elif [[ ${RUNNER_OS:-} == "Linux" ]]; then
  :

fi


### Pre-build

# ccache
if [[ $USE_CCACHE == 1 ]]; then
  echo "::group::Show ccache configuration"
  ccache --version
  ccache --show-config
  echo " -----"
  ccache --show-stats
  ccache --show-compression
  echo " -----"
  ccache --show-stats --verbose
  ccache --show-compression
  echo " -----"
  ccache --zero-stats
  echo " -----"
  ccache --show-stats
  echo " -----"
  ccache --show-stats --verbose
  echo "::endgroup::"
fi


### Build

# Configure CMake
echo "::group::Generate buildsystem"
cmake --version
echo "Running CMake configuration with following flags: ${flags[*]}"
cmake -S . -B "$BUILD_DIR" "${flags[@]}"
echo "::endgroup::"

# Compile
echo "::group::Build project"
echo "Running CMake with following build flags: ${buildflags[*]}"
cmake --build "$BUILD_DIR" "${buildflags[@]}"
echo "::endgroup::"


### Post-build

# ccache
if [[ $USE_CCACHE == 1 ]]; then

  if [[ -n $CCACHE_EVICTION_AGE ]]; then
    echo "::group::evict ccache files older than $CCACHE_EVICTION_AGE"
    ccache --evict-older-than "$CCACHE_EVICTION_AGE"
    echo "::endgroup::"
  fi

  echo "::group::Show ccache statistics"
  ccache --show-stats --verbose
  ccache --show-compression
  echo "::endgroup::"

elif [[ -n $CCACHE_EVICTION_AGE ]]; then
  echo "::error file=$0::ccache eviction is enabled while ccache is disabled!"
fi

# Inspect
if [[ ${RUNNER_OS:-} == "macOS" ]]; then
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
if [[ $MAKE_TEST == 1 ]]; then
  echo "::group::Run tests"
  ctest --version
  ctest --build-config "$BUILDTYPE" --test-dir "$BUILD_DIR" --output-on-failure
  echo "::endgroup::"
fi

# Install
if [[ $MAKE_INSTALL == 1 ]]; then
  echo "::group::Install"
  cmake --install "$BUILD_DIR" --config "$BUILDTYPE"
  echo "::endgroup::"
fi

# Package
if [[ $MAKE_PACKAGE == 1 ]]; then
  echo "::group::Debug NSIS / CPack"
  cat "_CPack_Packages/win64/NSIS/NSISOutput.log"
  echo " -----"
  find . -name "NSIS.definitions.nsh"
  find "$BUILD_DIR" -name "NSIS.definitions.nsh"
  ls -l "$BUILD_DIR/NSIS.definitions.nsh"
  echo "::endgroup:"
  
  echo "::group::Create package"
  cpack --version
  (
    cd "$BUILD_DIR"
    cpack
  )
  # cpack --config "$BUILD_DIR/CPackConfig.cmake"
  # cmake --build "$BUILD_DIR" --target package
  #TODO included --config? compare with former isntall and other --target variants used
  echo "::endgroup::"

  if [[ -n $PACKAGE_SUFFIX ]]; then
    echo "::group::Update package name"
    .ci/name_build.sh "$PACKAGE_SUFFIX"
    echo "::endgroup::"
  fi
fi
