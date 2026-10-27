#!/bin/bash

# This script is to be used by the ci environment from the project root directory, do not use it from somewhere else.

# Compiles cockatrice inside of a ci environment
# --install runs make install
# --package [<package type>] runs make package, optionally force the type
# --suffix <suffix> renames package with this suffix, requires arg
# --server compiles servatrice
# --test runs tests
# --sanitize builds with AddressSanitizer and UndefinedBehaviorSanitizer, gcc/clang only
# --coverage instruments the test run for line coverage and prints a gcovr summary, gcc only

# --debug or --release sets the build type ie CMAKE_BUILD_TYPE
# --ccache [<size>] uses ccache and shows stats, optionally provide size
# --evict-ccache <age> runs ccache eviction based on given age after build
# --dir <dir> sets the name of the build dir, default is "build"
# --cmake-generator <generator> sets CMAKE_GENERATOR as used by cmake
# --target-macos-version <version> sets the min os version - only used for macOS builds
# --profile writes an MSBuild performance summary and binary log, Windows only
# uses env: BUILDTYPE MAKE_INSTALL MAKE_PACKAGE PACKAGE_TYPE PACKAGE_SUFFIX MAKE_SERVER MAKE_NO_CLIENT MAKE_TEST USE_CCACHE CCACHE_SIZE CCACHE_EVICTION_AGE BUILD_DIR CMAKE_GENERATOR TARGET_MACOS_VERSION MAKE_SANITIZE MAKE_COVERAGE VCPKG_TARGET_MACOS_VERSION BUILD_PARALLEL_LEVEL BUILD_PROFILE
# (correspond to args: --debug/--release --install --package <package type> --suffix <suffix> --server --test --sanitize --coverage --ccache <ccache_size> --dir <dir> --profile)
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
    '--sanitize')
      MAKE_SANITIZE=1
      shift
      ;;
    '--coverage')
      MAKE_COVERAGE=1
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

# Setup
./servatrice/check_schema_version.sh
if [[ ! $BUILDTYPE ]]; then
  BUILDTYPE=Release
fi
if [[ ! $BUILD_DIR ]]; then
  BUILD_DIR="build"
fi
mkdir -p "$BUILD_DIR"

if [[ $MAKE_COVERAGE ]]; then
  # Resolved while the working directory is still the source root. $0 is relative to the
  # directory the script was invoked from, so after the cd below it no longer resolves, and
  # under `set -e` the failed subshell would abort the build rather than just yielding an empty
  # variable. Only the coverage report needs this, so only coverage pays for it.
  SRC_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
fi

cd "$BUILD_DIR"

# Set minimum CMake Version
export CMAKE_POLICY_VERSION_MINIMUM=3.10

# Add cmake flags
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
if [[ $MAKE_SANITIZE ]]; then
  flags+=("-DENABLE_SANITIZERS=ON")
else
  # The release-package step re-runs cmake in the same build dir, so an
  # ENABLE_SANITIZERS=ON left over from the debug step would otherwise leak
  # into the shipped binaries (libasan/libubsan Depends in the DEB).
  flags+=("-DENABLE_SANITIZERS=OFF")
fi
if [[ $MAKE_COVERAGE ]]; then
  flags+=("-DENABLE_COVERAGE=ON")
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
if [[ $PACKAGE_TYPE ]]; then
  flags+=("-DCPACK_GENERATOR=$PACKAGE_TYPE")
fi
if [[ $USE_VCPKG ]]; then
  flags+=("-DUSE_VCPKG=1")
  flags+=("-DVCPKG_INSTALL_OPTIONS=--x-abi-tools-use-exact-versions")
fi
if [[ ${WITH_CRASH_REPORTING:-} == "ON" ]]; then
  # Release CI opts into the sentry-native SDK (default is OFF so from-source, distro and
  # offline package builds never fetch it). Debug builds stay SDK-free so crashes from
  # developer builds never reach Sentry.
  flags+=("-DWITH_CRASH_REPORTING=ON")
fi
if [[ ${BUILDTYPE:-Release} == "Release" && -n ${COCKATRICE_SENTRY_DSN:-} ]]; then
  # Release-only: Debug builds stay DSN-free so crashes from developer builds
  # never reach Sentry. Empty (local builds, forks without the secret, and non-tag
  # CI builds, which never receive the secret) keeps today's mock fallback via the
  # compiled-in default. CI passes the value from the SENTRY_DSN secret on tag
  # Release builds only, so pre-release test artifacts never report to Sentry.
  flags+=("-DCOCKATRICE_SENTRY_DSN=$COCKATRICE_SENTRY_DSN")
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
    # The triplet is what vcpkg hashes into the ABI hash of every port, so two legs that
    # agree here reuse each other's binary cache. Targets 14, 15 and 26 on the same Xcode
    # produce identical binaries when they share the lowest deployment target, so they all
    # build against 14 to keep one vcpkg cache scope instead of three.
    # Keep in sync with "vcpkg_scope" and "vcpkg_deployment_target" in desktop-build.yml.
    vcpkg_target="${VCPKG_TARGET_MACOS_VERSION:-$TARGET_MACOS_VERSION}"
    echo "set(VCPKG_CMAKE_SYSTEM_VERSION $vcpkg_target)" >>"$triplet_file"
    echo "set(VCPKG_OSX_DEPLOYMENT_TARGET $vcpkg_target)" >>"$triplet_file"
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
    # cmake only hands MSBuild a job count when one is asked for, and without it
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
    # Ninja runs the compiler itself, so it neither takes the MSBuild switches
    # (it rejects -p: options outright) nor a job count: it sizes its own pool
    # for the cores it finds.
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
  if [[ $MAKE_SANITIZE ]]; then
    # detect_leaks=0: Qt and the sanitizers' own runtimes both leak at exit by design, and
    # LeakSanitizer's default is aggressive enough to bury our real findings under hundreds
    # of process-teardown reports. Malloc/free misuse and use-after-free are what ASan is for
    # and they stay fully on.
    # abort_on_error=1: a bad access leaves the process in an unusable state, so continuing
    # only produces a second, more confusing report from the wreckage.
    # UBSAN halt_on_error=1: without it UBSan prints "runtime error: ..." but the
    # process exits 0 (recover mode), so gtest/ctest report a pass and the job
    # stays green. Combined with -fno-sanitize-recover=undefined at compile time,
    # the first finding aborts the test process with a non-zero exit.
    export ASAN_OPTIONS="detect_leaks=0:abort_on_error=1:print_stacktrace=1:symbolize=1"
    export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1:symbolize=1"
  fi
  # Every TEST() is its own CTest entry, so the suite is process-spawn bound rather than CPU
  # bound and -j recovers nearly all of the per-process overhead. The test binaries are safe to
  # run concurrently: the only one that claims a fixed resource is single_instance_manager_test,
  # which already scopes its local socket name to its own pid so parallel copies cannot collide.
  ctest -C "$BUILDTYPE" --output-on-failure -j "${CTEST_JOBS:-4}"
  echo "::endgroup::"
fi

if [[ $MAKE_COVERAGE ]]; then
  # gcovr is not a build dependency and is not available everywhere, so a missing one is reported
  # and skipped rather than failing an otherwise green build.
  echo "::group::Coverage report"
  GCOVR="$(command -v gcovr || true)"
  if [[ -z $GCOVR ]]; then
    echo "gcovr not found; install it to produce a coverage report"
  else
    "$GCOVR" --version
    # This script configures with `cmake ..`, so the build dir it was started from is the object
    # directory and the source root is one level up. Both are passed explicitly: gcovr otherwise
    # walks the current directory and trips over in-tree build directories left by other tooling,
    # whose stale .gcno files cannot be resolved. --config is named too, so the filters apply no
    # matter what directory gcovr happens to start from.
    GCOVR_ARGS=("." --root "$SRC_ROOT" --object-directory "." --config "$SRC_ROOT/gcovr.cfg")
    "$GCOVR" "${GCOVR_ARGS[@]}" --txt --decisions
    "$GCOVR" "${GCOVR_ARGS[@]}" --html-details coverage-details.html --html-title "Cockatrice line coverage"
    "$GCOVR" "${GCOVR_ARGS[@]}" --lcov -o coverage.lcov
    "$GCOVR" "${GCOVR_ARGS[@]}" --cobertura-pretty -o coverage.xml
    "$GCOVR" "${GCOVR_ARGS[@]}" --json-summary-pretty -o coverage-summary.json
    echo "Coverage artifacts written to coverage-details.html, coverage.lcov, coverage.xml and coverage-summary.json"
  fi
  echo "::endgroup::"
fi

if [[ $RUNNER_OS == Windows && ! $MAKE_NO_CLIENT ]]; then
  echo "::group::Probe the installer's holder lookup"
  # The working directory is still the build dir here: the package steps below are what
  # leave and re-enter it, and that is what made a $BUILD_DIR-prefixed path name
  # build/build. Both paths are relative to the dir the probe was built into.
  probe="./installer-holder-probe.exe"
  if [[ ! -f $probe ]]; then
    echo "::error file=$0::installer-holder-probe.exe was not built"
    exit 1
  fi
  # The probe exists to name a holder that a process-and-module scan cannot see: one
  # that has a file open without having loaded it. Holding a temporary DLL with
  # FileShare.None is exactly that case, so this asserts the reason the probe was
  # added instead of only that the Restart Manager answers at all. It needs no NSIS
  # and no packaging, so it runs on every Windows build that gets this far.
  # The holder is a process of its own: the probe never names the process that started it,
  # which is what keeps an in-place uninstaller from being named as a holder of the
  # directory it is emptying. The same DLL held by the script that starts the probe
  # therefore has to come back as nothing, and that is the second case below.
  probe_check="./holder-probe-check.ps1"
  cat >"$probe_check" <<'POWERSHELL'
$ErrorActionPreference = "Stop"
$dir = Join-Path ([IO.Path]::GetTempPath()) "cockatrice-holder-probe-$PID"
Remove-Item -Recurse -Force $dir -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force $dir | Out-Null
$probe = (Resolve-Path -LiteralPath $env:PROBE_EXE).Path

function Invoke-Probe([string] $guardDir) {
    $start = New-Object System.Diagnostics.ProcessStartInfo
    $start.FileName = $probe
    $start.UseShellExecute = $false
    $start.RedirectStandardOutput = $true
    $start.EnvironmentVariables["COCKATRICE_GUARD_DIR"] = $guardDir
    $process = [System.Diagnostics.Process]::Start($start)
    $read = $process.StandardOutput.ReadToEndAsync()
    if (-not $process.WaitForExit(20000)) {
        try { $process.Kill() } catch { }
        throw "the holder probe did not finish within 20 s for $guardDir"
    }
    $output = $read.Result
    $code = $process.ExitCode
    $process.Dispose()
    return @{ Code = $code; Output = $output }
}

function Wait-UntilLocked([string] $path) {
    $deadline = (Get-Date).AddSeconds(15)
    while ((Get-Date) -lt $deadline) {
        try {
            $open = [IO.File]::Open($path, "Open", "ReadWrite", "None")
            $open.Dispose()
        } catch {
            return
        }
        Start-Sleep -Milliseconds 100
    }
    throw "the holder never locked $path"
}

try {
    $file = Join-Path $dir "locked.dll"
    [IO.File]::WriteAllText($file, "x")

    # The holder is a process of its own, not this script: the probe never names the process
    # that started it (the case below asserts that), so a file held here would be left out on
    # purpose and the check would pass for the wrong reason.
    $holderScript = Join-Path $dir "hold.ps1"
    Set-Content -LiteralPath $holderScript -Value @'
param([string] $Path)
$handle = [IO.File]::Open($Path, "Open", "ReadWrite", "None")
Start-Sleep -Seconds 120
'@
    $powershell = Join-Path $env:SystemRoot "System32\WindowsPowerShell\v1.0\powershell.exe"
    $holder = Start-Process -FilePath $powershell -PassThru -ArgumentList @(
        "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "`"$holderScript`"", "-Path", "`"$file`"")
    try {
        Wait-UntilLocked $file
        $locked = Invoke-Probe $dir
        if ($locked.Code -ne 1) { throw "expected exit 1 for a locked file, got $($locked.Code) : $($locked.Output)" }
        if ($locked.Output -notmatch "pid\s+$($holder.Id)\b") {
            throw "the lookup did not name the process holding the file: $($locked.Output)"
        }
    } finally {
        try { $holder.Kill() } catch { }
        try { $holder.WaitForExit(5000) | Out-Null } catch { }
    }

    # The probe is started by the installer, or by an uninstaller that runs in place from the
    # directory it is emptying, so the asking process holds files there itself. Naming it would
    # ask the user to close the program that is asking - a question neither the close nor the
    # end offer can answer, so the guard repeated it until the update was cancelled. A holder
    # that is only the asking process has to come back as nothing.
    $handle = [IO.File]::Open($file, "Open", "ReadWrite", "None")
    try {
        $self = Invoke-Probe $dir
    } finally {
        $handle.Dispose()
    }
    if ($self.Code -ne 0) { throw "expected exit 0 when the asking process is the only holder, got $($self.Code) : $($self.Output)" }
    if ($self.Output -match "pid\s+$PID\b") { throw "the lookup named the process that started it: $($self.Output)" }
    $empty = Join-Path $dir "empty"
    New-Item -ItemType Directory -Force $empty | Out-Null
    $nothing = Invoke-Probe $empty
    if ($nothing.Code -ne 0) { throw "expected exit 0 for a directory with nothing open, got $($nothing.Code) : $($nothing.Output)" }
    # A process running from the directory is a holder even when it has no file open: an
    # instance holds its payload as mapped images, not as handles, and that is the state
    # the guard used to miss - it passed, the purge failed, and the install ended in a
    # warning with no offer to close and try again. The image is a copy of a native
    # Windows program, so it starts without its usual directory next to it.
    $copy = Join-Path $dir "fake-cockatrice.exe"
    Copy-Item -LiteralPath (Join-Path $env:SystemRoot "System32\cmd.exe") -Destination $copy
    $running = Start-Process -FilePath $copy -ArgumentList "/c", "ping -n 20 127.0.0.1 >nul" -PassThru
    try {
        Start-Sleep -Milliseconds 1500
        $byImage = Invoke-Probe $dir
    } finally {
        try { $running.Kill() } catch { }
        # Wait for the image to be released, or the cleanup below leaves the copy behind.
        try { $running.WaitForExit(5000) | Out-Null } catch { }
    }
    if ($byImage.Code -ne 1) { throw "expected exit 1 for a process running from the directory, got $($byImage.Code) : $($byImage.Output)" }
    if ($byImage.Output -notmatch "fake-cockatrice\.exe \(pid\s+$($running.Id)") {
        throw "the lookup did not name the process running from the directory: $($byImage.Output)"
    }
    Write-Host "holder lookup named pid $($holder.Id) for a file another process holds, named nothing for a file only the asking process holds, named nothing for an empty directory, and named pid $($running.Id) for a process running from the directory"
} finally {
    Remove-Item -Recurse -Force $dir -ErrorAction SilentlyContinue
}
POWERSHELL
  if ! PROBE_EXE="$probe" powershell -NoProfile -File "$probe_check"; then
    rm -f "$probe_check"
    exit 1
  fi
  rm -f "$probe_check"
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

      # Fail the build if the installer does not carry the runtime the
      # applications link against - a file that must be there, or a DLL that must
      # not be. The runtime DLLs reach the package by globbing every *.dll out of
      # the build output directories, so anything that build produced can ride
      # along and nothing else in the build objects to it: a Qt module the client
      # links statically (Qt6Multimedia.dll, Qt6QuickWidgets.dll) can be left out,
      # and the test framework's gtest.dll can be shipped. Either only surfaces on
      # a user's machine after an update has already replaced their install, as
      # "The code execution cannot proceed because Qt6Multimedia.dll was not
      # found" or "abseil_dll.dll is either not designed to run on Windows or it
      # contains an error". required-runtime.txt is written by the top level
      # CMakeLists.txt from the same module lists the targets are built against
      # plus the vcpkg bin directory the app-local copies come from.
      if [[ ! -f required-runtime.txt ]]; then
        echo "::error file=$0::required-runtime.txt not found in the build dir, cannot verify the runtime"
        exit 1
      fi
      # 7-Zip writes CRLF on Windows, so the \r has to come off before any
      # whole-line comparison below. Normalize separators to /, and strip the
      # install-root prefixes an NSIS listing may carry, so the entries can be
      # compared as exact relative paths. -xF keeps a '.' in a DLL name from
      # being a wildcard; -i because that is how the installed tree behaves -
      # the uninstaller clears both $INSTDIR\plugins and $INSTDIR\Plugins.
      # The archive side and the manifest side have to agree on line endings.
      # CMake writes required-runtime.txt with the platform's line ending and
      # 7-Zip writes CRLF on Windows, so a trailing \r survives on $entry and
      # defeats the whole-line match - every entry then reads as missing even
      # though the listing holds it verbatim.
      # [$] rather than \$ so the literal dollar is a bracket expression and the
      # linter does not read these as unexpanded shell variables (SC2016).
      installer_paths="$("$seven_zip" l -slt "$package" | tr -d '\r' |
        sed -n 's/^Path = //p' |
        sed -e 's|\\|/|g' -e 's|^[$]INSTDIR/||' -e 's|^[$]OUTDIR/||' -e 's|^[$]PLUGINSDIR/||' -e 's|^\./||' -e 's|^/||')"
      manifest="$(tr -d '\r' <required-runtime.txt)"
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
        echo "::error file=$0::Installer is missing required runtime files:$missing"
        # Print both sides of the comparison. A mismatch here has twice been
        # caused by the shape of 7-Zip's output rather than by a missing file,
        # and a bare "missing" line cannot tell those apart.
        echo "required-runtime.txt asked for $checked path(s); 7-Zip listed $(grep -c '' <<<"$installer_paths") path(s)"
        # Indent with parameter expansion rather than sed 's/^/  /', which the
        # linter reports as SC2001.
        echo "--- required ---"
        echo "${manifest//$'\n'/$'\n  '}"
        echo "--- first 40 paths 7-Zip lists in the installer ---"
        listed_head="$(head -40 <<<"$installer_paths")"
        echo "${listed_head//$'\n'/$'\n  '}"
        exit 1
      fi
      echo "Installer carries the Qt runtime of every module the three applications link ($checked files)"

      # Now the other direction: a top level DLL in the package that the manifest
      # does not name. The missing check above cannot see a runtime that was
      # never expected.
      #
      # Two things are reported here and neither may fail the build.
      #
      # The test framework is a fact rather than a guess - no shipped executable
      # links gtest or gmock, and the build output directories the "*.dll" glob
      # reads are shared with the test targets, so gtest.dll and gmock.dll are
      # one filter away from shipping in every release. Even so it is reported
      # instead of enforced: a check that has never passed on a real package has
      # not earned the right to block a release, and the first Windows run of an
      # enforcing version is exactly when its own wrong assumptions show up.
      #
      # Everything else is expected to be there. required-runtime.txt names the
      # Qt modules the three applications link, and the protobuf, Abseil,
      # OpenSSL, liblzma, ffmpeg and Qt debug DLLs in the package come in on the
      # build output glob instead, because nothing in the build declares them by
      # name. So this list is a report of what the package carries, not a verdict.
      # Read it when a dependency is added; do not fail on it.
      forbidden=""
      unexpected=""
      while IFS= read -r entry; do
        [[ -z $entry ]] && continue
        # Top level only. A DLL under Plugins/ or a nested image format plugin is
        # a plugin the platform loads by name, not a runtime the executable
        # imports, and requiring one here would list all of them every run.
        [[ $entry == */* ]] && continue
        if grep -qxiF "$entry" <<<"$manifest"; then
          continue
        fi
        # NSIS extracts its plugins to $PLUGINSDIR, which the path normalization
        # above strips, so they arrive here looking like payload files. They are
        # part of the installer, not of the installed tree.
        if [[ $entry == [Nn][Ss][Dd][Ii][Aa][Ll][Oo][Gg][Ss].dll ||
          $entry == [Nn][Ss][Ee][Xx][Ee][Cc].dll ||
          $entry == [Ss][Yy][Ss][Tt][Ee][Mm].dll ||
          $entry == [Uu][Ss][Ee][Rr][Ii][Nn][Ff][Oo].dll ||
          $entry == [Ii][Nn][Ee][Tt].dll ]]; then
          continue
        fi
        if [[ $entry == [Gg][Tt][Ee][Ss][Tt]* || $entry == [Gg][Mm][Oo][Cc][Kk]* ]]; then
          forbidden+=" $entry"
        else
          unexpected+=" $entry"
        fi
      done < <(grep -i '\.dll$' <<<"$installer_paths")
      if [[ -n $forbidden ]]; then
        echo "::warning file=$0::Installer contains test framework DLLs that no shipped executable links:$forbidden"
        echo "A build output directory shared with the test targets has leaked into the package."
      fi
      if [[ -n $unexpected ]]; then
        echo "Top level DLLs in the package beyond the linked Qt modules:$unexpected"
        echo "These arrive on the build output glob, not from required-runtime.txt, so they are reported and not required."
      fi
      if [[ -z $forbidden && -z $unexpected ]]; then
        echo "Installer carries no runtime files beyond the linked Qt modules"
      fi
    fi
    echo "::endgroup::"
  fi
fi
