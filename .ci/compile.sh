#!/bin/bash

# This script is to be used by the ci environment from the project root directory, do not use it from somewhere else.

# Compiles cockatrice inside of a ci environment
# --install runs make install
# --package [<package type>] runs make package, optionally force the type
# --suffix <suffix> renames package with this suffix, requires arg
# --server compiles servatrice
# --test runs tests
# --debug or --release sets the build type ie CMAKE_BUILD_TYPE
# --ccache [<size>] uses ccache and shows stats, optionally provide size
# --dir <dir> sets the name of the build dir, default is "build"
# --target-macos-version <version> sets the min os version - only used for macOS builds
# uses env: BUILDTYPE MAKE_INSTALL MAKE_PACKAGE PACKAGE_TYPE PACKAGE_SUFFIX MAKE_SERVER MAKE_NO_CLIENT MAKE_TEST USE_CCACHE CCACHE_SIZE BUILD_DIR CMAKE_GENERATOR TARGET_MACOS_VERSION
# (correspond to args: --debug/--release --install --package <package type> --suffix <suffix> --server --test --ccache <ccache_size> --dir <dir>)
# exitcode: 1 for failure, 3 for invalid arguments

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
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Set minimum CMake Version
export CMAKE_POLICY_VERSION_MINIMUM=3.10

# Add cmake flags
flags=("-DCMAKE_BUILD_TYPE=$BUILDTYPE")
if [[ $MAKE_SERVER ]]; then
  flags+=("-DWITH_SERVER=1")
fi
if [[ $MAKE_NO_CLIENT ]]; then
  flags+=("-DWITH_CLIENT=0" "-DWITH_ORACLE=0" "-DWITH_DBCONVERTER=0")
fi
if [[ $MAKE_TEST ]]; then
  flags+=("-DTEST=1")
fi
if [[ $USE_CCACHE ]]; then
  flags+=("-DUSE_CCACHE=1")
  if [[ $CCACHE_SIZE ]]; then
    # note, this setting persists after running the script
    ccache --max-size "$CCACHE_SIZE"
  fi
fi
if [[ $PACKAGE_TYPE ]]; then
  flags+=("-DCPACK_GENERATOR=$PACKAGE_TYPE")
fi
if [[ $USE_VCPKG ]]; then
  flags+=("-DUSE_VCPKG=1")
fi

# Add cmake --build flags
buildflags=(--config "$BUILDTYPE")

function ccachestatsverbose() {
  # note, verbose only works on newer ccache, discard the error
  local got
  if got="$(ccache --show-stats --verbose 2>/dev/null)"; then
    echo "$got"
  else
    ccache --show-stats
  fi
}

# Compile
if [[ $RUNNER_OS == macOS ]]; then

  if [[ $TARGET_MACOS_VERSION ]]; then
    # CMAKE_OSX_DEPLOYMENT_TARGET is a vanilla cmake flag needed to compile to target macOS version
    flags+=("-DCMAKE_OSX_DEPLOYMENT_TARGET=$TARGET_MACOS_VERSION")

    # vcpkg dependencies need a vcpkg triplet file to compile to the target macOS version
    # an easy way is to copy the x64-osx.cmake file and modify it
    triplets_dir="/tmp/cmake/triplets"
    triplet_version="custom-triplet"
    triplet_file="$triplets_dir/$triplet_version.cmake"
    arch=$(uname -m)
    if [[ $arch == x86_64 ]]; then
      arch="x64"
    fi
    mkdir -p "$triplets_dir"
    cp "../vcpkg/triplets/$arch-osx.cmake" "$triplet_file"
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
    # have hdiutil repeat the command 10 times in hope of success
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
  # Enable MTT, see https://devblogs.microsoft.com/cppblog/improved-parallelism-in-msbuild/
  # and https://devblogs.microsoft.com/cppblog/cpp-build-throughput-investigation-and-tune-up/#multitooltask-mtt
  buildflags+=(-- -p:UseMultiToolTask=true -p:EnableClServerMode=true)
fi

if [[ $USE_CCACHE ]]; then
  echo "::group::Show ccache stats"
  ccachestatsverbose
  echo "::endgroup::"
fi

echo "::group::Configure cmake"
cmake --version
echo "Running cmake with flags: ${flags[*]}"
cmake .. "${flags[@]}"
echo "::endgroup::"

echo "::group::Build project"
echo "Running cmake --build with flags: ${buildflags[*]}"
cmake --build . "${buildflags[@]}"
echo "::endgroup::"

if [[ $USE_CCACHE ]]; then
  echo "::group::Show ccache stats again"
  ccachestatsverbose
  echo "::endgroup::"
fi

if [[ $RUNNER_OS == macOS ]]; then
  echo "::group::Inspect Mach-O binaries"
  for app in cockatrice oracle servatrice dbconverter; do
    binary="$GITHUB_WORKSPACE/build/$app/$app.app/Contents/MacOS/$app"
    echo "Inspecting $app..."
    vtool -show-build "$binary"
    file "$binary"
    lipo -info "$binary"
    echo ""
  done
  echo "::endgroup::"
fi

if [[ $MAKE_TEST ]]; then
  echo "::group::Run tests"
  ctest -C "$BUILDTYPE" --output-on-failure
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
    cd ..
    BUILD_DIR="$BUILD_DIR" .ci/name_build.sh "$PACKAGE_SUFFIX"
    echo "::endgroup::"
  fi
fi
