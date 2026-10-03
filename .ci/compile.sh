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
# --evict-ccache <age> runs ccache eviction based on given age after build
# --sccache uses sccache instead, needs a generator that runs the compiler itself
# --dir <dir> sets the name of the build dir, default is "build"
# --cmake-generator <generator> sets CMAKE_GENERATOR as used by cmake
# --target-macos-version <version> sets the min os version - only used for macOS builds
# --profile writes an MSBuild performance summary and binary log, Windows only
# --sccache uses sccache instead, needs a generator that runs the compiler itself
# uses env: BUILDTYPE MAKE_INSTALL MAKE_PACKAGE PACKAGE_TYPE PACKAGE_SUFFIX MAKE_SERVER MAKE_NO_CLIENT MAKE_TEST USE_CCACHE CCACHE_SIZE CCACHE_EVICTION_AGE BUILD_DIR CMAKE_GENERATOR TARGET_MACOS_VERSION BUILD_PARALLEL_LEVEL BUILD_PROFILE USE_SCCACHE
# (correspond to args: --debug/--release --install --package <package type> --suffix <suffix> --server --test --ccache <ccache_size> --dir <dir> --profile --sccache)
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
    '--evict-ccache')
      shift
      if [[ $# == 0 ]]; then
        echo "::error file=$0::--evict-ccache expects an argument"
        exit 3
      fi
      CCACHE_EVICTION_AGE=$1
      shift
      ;;
    '--sccache')
      USE_SCCACHE=1
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
    '--profile')
      BUILD_PROFILE=1
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

# cmake reads the generator platform from the environment, so a leftover platform
# follows the generator around; Ninja has no platform to select.
if [[ $CMAKE_GENERATOR != "Visual Studio"* ]]; then
  unset CMAKE_GENERATOR_PLATFORM
fi

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
if [[ $USE_CCACHE && $USE_SCCACHE ]]; then
  echo "::error file=$0::ccache and sccache at the same time is not a supported combination"
  exit 1
fi
if [[ $USE_SCCACHE && $CMAKE_GENERATOR != "Ninja"* ]]; then
  # A compiler cache wraps the compiler, and only generators that run the compiler
  # themselves are given a launcher to wrap: cmake does not pass one to MSBuild.
  echo "::error file=$0::sccache needs a Ninja generator, not '$CMAKE_GENERATOR', which would ignore it"
  exit 1
fi
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
  # PCH-aware caching is required or ccache refuses to cache any TU that
  # consumes a precompiled header, silently recompiling everything on every run.
  ccache --set-config sloppiness=pch_defines,time_macros
  if [[ $CCACHE_SIZE ]]; then
    # note, this setting persists after running the script
    ccache --max-size "$CCACHE_SIZE"
  fi
fi
if [[ $USE_SCCACHE ]]; then
  flags+=("-DUSE_SCCACHE=1")
  flags+=("-DCMAKE_C_COMPILER_LAUNCHER=sccache" "-DCMAKE_CXX_COMPILER_LAUNCHER=sccache")
fi
if [[ $PACKAGE_TYPE ]]; then
  flags+=("-DCPACK_GENERATOR=$PACKAGE_TYPE")
fi
if [[ $USE_VCPKG ]]; then
  flags+=("-DUSE_VCPKG=1")
  flags+=("-DVCPKG_INSTALL_OPTIONS=--x-abi-tools-use-exact-versions")
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

function cpuinfo() {
  # The number of cores decides how much build parallelism is useful; the hosted
  # runners are not all sized alike, so read it off the machine instead of assuming.
  case "$RUNNER_OS" in
  Windows)
    echo "cores: ${NUMBER_OF_PROCESSORS:-unknown}"
    ;;
  macOS)
    echo "cores: $(sysctl -n hw.ncpu)"
    ;;
  *)
    echo "cores: $(nproc 2>/dev/null || echo unknown)"
    ;;
  esac
}

# Compile
if [[ $RUNNER_OS == macOS ]]; then
  # QTDIR is needed for macOS since we actually only use the cached thin Qt binaries instead of the install-qt-action,
  # which sets a few environment variables
  if QTDIR=$(find "$GITHUB_WORKSPACE/Qt" -depth -maxdepth 2 -name macos -type d -print -quit); then
    echo "found QTDIR at $QTDIR"
  else
    echo "could not find QTDIR!"
    exit 2
  fi
  # the qtdir is located at Qt/[qtversion]/macos
  # we use find to get the first subfolder with the name "macos"
  # this works independent of the qt version as there should be only one version installed on the runner at a time
  export QTDIR

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
  if [[ $CMAKE_GENERATOR == "Visual Studio"* ]]; then
    # cmake only hands MSBuild a job count when it is asked for, and without it
    # MSBuild builds a single node: one project at a time, whatever MTT does inside
    # it. Ask for a node per core.
    if [[ ! $BUILD_PARALLEL_LEVEL ]]; then
      BUILD_PARALLEL_LEVEL="${NUMBER_OF_PROCESSORS:-1}"
    fi
    buildflags+=(--parallel "$BUILD_PARALLEL_LEVEL")

    # Enable MTT, see https://devblogs.microsoft.com/cppblog/improved-parallelism-in-msbuild/
    # and https://devblogs.microsoft.com/cppblog/cpp-build-throughput-investigation-and-tune-up/#multitooltask-mtt
    # EnforceProcessCountAcrossBuilds makes the CL_MPCount ceiling apply across all
    # the projects the nodes are building, instead of each node running its own
    # cl.exe processes and /m and MTT multiplying into an oversubscribed machine.
    buildflags+=(-- -p:UseMultiToolTask=true -p:EnableClServerMode=true -p:EnforceProcessCountAcrossBuilds=true -p:CL_MPCount="$BUILD_PARALLEL_LEVEL")
    if [[ $BUILD_PROFILE ]]; then
      # A performance summary says which projects and tasks cost what, the binary log
      # keeps the full timeline; both are far too noisy to always produce. The files
      # land next to the solution, i.e. in the build dir the workflow uploads. These
      # join the MTT options above, which already carried the "--" separator.
      buildflags+=("-flp:PerformanceSummary;v=q;LogFile=msbuild-perf.log" -bl:msbuild.binlog)
    fi
  else
    # Ninja runs the compiler itself, so it neither needs the MSVC switches nor
    # wants a job count: it sizes its own pool for the cores it finds.
    echo "ninja $(ninja --version)"
  fi
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
cpuinfo
echo "Running cmake --build with flags: ${buildflags[*]}"
cmake --build . "${buildflags[@]}"
echo "::endgroup::"

if [[ $BUILD_PROFILE && $RUNNER_OS == Windows && -f msbuild-perf.log ]]; then
  # Repeat the summary in the job log so the numbers are readable without
  # downloading the binary log, which is hundreds of megabytes.
  echo "::group::MSBuild performance summary"
  cat msbuild-perf.log
  echo "::endgroup::"
fi

if [[ $USE_CCACHE ]]; then
  if [[ $CCACHE_EVICTION_AGE ]]; then
    echo "::group::evict ccache files older than $CCACHE_EVICTION_AGE"
    ccache --evict-older-than "$CCACHE_EVICTION_AGE"
    echo "::endgroup::"
  fi
  echo "::group::Show ccache stats again"
  ccachestatsverbose
  echo "::endgroup::"
elif [[ $USE_SCCACHE ]]; then
  echo "::group::Show sccache stats"
  sccache --show-stats
  echo "::endgroup::"
elif [[ $CCACHE_EVICTION_AGE ]]; then
  echo "::error file=$0::ccache eviction is enabled while ccache is disabled!"
fi

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

  if [[ $RUNNER_OS == Windows ]]; then
    echo "::group::Check installer for build-tree artifacts"
    cd "$BUILD_DIR"
    package="$(find . -maxdepth 1 -type f -name 'Cockatrice-*.exe' -print -quit)"
    if [[ ! $package ]]; then
      echo "::error file=$0::Could not find installer to inspect"
      exit 1
    fi
    seven_zip="$(command -v 7z || true)"
    if [[ ! $seven_zip ]]; then
      seven_zip="/c/Program Files/7-Zip/7z.exe"
    fi
    if [[ ! -f $seven_zip ]]; then
      echo "::warning file=$0::7-Zip not found, skipping installer content check"
    else
      echo "Inspecting $package"
      # Fail the build if the installer contains any path left behind by the MSBuild or
      # Qt AUTOMOC tooling (build-tree artifacts must live in the build dir, not the install)
      if "$seven_zip" l "$package" |
        grep -E "_autogen|\.dir[\\/]|\.tlog|(^|[\\/])x64[\\/]|(^|[\\/])\.qt[\\/]|(^|[\\/])\.qsb[\\/]|(^|[\\/])\.lupdate[\\/]|CMakeFiles"; then
        echo "::error file=$0::Installer contains build-tree artifacts"
        exit 1
      fi
      echo "Installer content is clean"

      # Fail the build if the installer is missing a Qt runtime the applications
      # link against. The DLLs reach the package by globbing every *.dll out of
      # the build output directories, so a module the client links statically
      # (Qt6Multimedia.dll, Qt6QuickWidgets.dll) can be left out without anything
      # else noticing, and the only symptom is a client that refuses to start
      # after an update. required-qt-runtime.txt is written by the top level
      # CMakeLists.txt from the same module list the targets are built against.
      if [[ ! -f required-qt-runtime.txt ]]; then
        echo "::error file=$0::required-qt-runtime.txt not found in the build dir, cannot verify the Qt runtime"
        exit 1
      fi
      # 7-Zip writes CRLF on Windows, so the \r has to come off before any
      # whole-line comparison below. Normalize separators to /, and strip the
      # install-root prefixes an NSIS listing may carry, so the entries can be
      # compared as exact relative paths. -xF keeps a '.' in a DLL name from
      # being a wildcard; -i because that is how the installed tree behaves -
      # the uninstaller clears both $INSTDIR\plugins and $INSTDIR\Plugins.
      # The archive side and the manifest side have to agree on line endings.
      # CMake writes required-qt-runtime.txt with the platform's line ending and
      # 7-Zip writes CRLF on Windows, so a trailing \r survives on $entry and
      # defeats the whole-line match - every entry then reads as missing even
      # though the listing holds it verbatim.
      # [$] rather than \$ so the literal dollar is a bracket expression and the
      # linter does not read these as unexpanded shell variables (SC2016).
      installer_paths="$("$seven_zip" l -slt "$package" | tr -d '\r' |
        sed -n 's/^Path = //p' |
        sed -e 's|\\|/|g' -e 's|^[$]INSTDIR/||' -e 's|^[$]OUTDIR/||' -e 's|^[$]PLUGINSDIR/||' -e 's|^\./||' -e 's|^/||')"
      manifest="$(tr -d '\r' <required-qt-runtime.txt)"
      missing=""
      checked=0
      while IFS= read -r entry; do
        if [[ -z $entry || $entry == \#* ]]; then
          continue
        fi
        checked=$((checked + 1))
        if ! grep -qxiF "$entry" <<<"$installer_paths"; then
          missing+=" $entry"
        fi
      done <<<"$manifest"
      if [[ -n $missing ]]; then
        echo "::error file=$0::Installer is missing required Qt runtime files:$missing"
        # Print both sides of the comparison. A mismatch here has twice been
        # caused by the shape of 7-Zip's output rather than by a missing file,
        # and a bare "missing" line cannot tell those apart.
        echo "required-qt-runtime.txt asked for $checked path(s); 7-Zip listed $(grep -c '' <<<"$installer_paths") path(s)"
        # Indent with parameter expansion rather than sed 's/^/  /', which the
        # linter reports as SC2001.
        echo "--- required ---"
        echo "${manifest//$'\n'/$'\n  '}"
        echo "--- first 40 paths 7-Zip lists in the installer ---"
        listed_head="$(head -40 <<<"$installer_paths")"
        echo "${listed_head//$'\n'/$'\n  '}"
        exit 1
      fi
      echo "Installer contains the full Qt runtime ($checked files)"
    fi
    echo "::endgroup::"
  fi
fi
