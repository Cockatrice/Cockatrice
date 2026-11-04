#!/bin/bash
# Run QtTesting XML test files against the Cockatrice build.
#
# Usage:
#   ./Testing/run_test.sh [--visual] [--all | <test-file.xml> ...]
#
# Options:
#   --visual    Run with native display (shows the GUI window).
#               Default is headless (QT_QPA_PLATFORM=offscreen).
#   --all       Run every *.xml file in Testing/.
#
# Environment:
#   COCKATRICE_BIN   Path to the client binary (default: build/debug/cockatrice/cockatrice,
#                    or the .app bundle binary on macOS).
#   TEST_TIMEOUT     Seconds a single test may run before it is killed (default: 180).
#                    Uses timeout, or gtimeout from coreutils on macOS.
#
# Requires the debug build to exist at build/debug/cockatrice/cockatrice.

set -euo pipefail
shopt -s nullglob

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd -P)"
if [ -n "${COCKATRICE_BIN:-}" ]; then
    COCKATRICE="$COCKATRICE_BIN"
elif [ "$(uname -s)" = "Darwin" ]; then
    # A macOS build produces an app bundle, not a bare executable.
    COCKATRICE="$REPO_ROOT/build/debug/cockatrice/cockatrice.app/Contents/MacOS/cockatrice"
else
    COCKATRICE="$REPO_ROOT/build/debug/cockatrice/cockatrice"
fi
TEST_DIR="$REPO_ROOT/Testing"
TEST_TIMEOUT="${TEST_TIMEOUT:-180}"

# Stock macOS has no `timeout`; coreutils provides `gtimeout`. Without either the
# runner still works, but a hung test is not killed.
TIMEOUT=()
if command -v timeout >/dev/null 2>&1; then
    TIMEOUT=(timeout --foreground "$TEST_TIMEOUT")
elif command -v gtimeout >/dev/null 2>&1; then
    TIMEOUT=(gtimeout --foreground "$TEST_TIMEOUT")
else
    echo "Warning: neither timeout nor gtimeout found; tests are not time-limited." >&2
fi

if [ ! -f "$COCKATRICE" ]; then
    echo "Error: $COCKATRICE not found. Build first:" >&2
    echo "  cmake --build build/debug" >&2
    exit 1
fi

VISUAL=0
if [ "${1:-}" = "--visual" ]; then
    VISUAL=1
    shift
fi

if [ "${1:-}" = "--all" ]; then
    shift
    TEST_FILES=("$TEST_DIR"/*.xml)
    if [ ${#TEST_FILES[@]} -eq 0 ]; then
        echo "No *.xml files found in $TEST_DIR" >&2
        exit 1
    fi
elif [ $# -ge 1 ]; then
    TEST_FILES=("$@")
else
    echo "Usage: $0 [--visual] [--all | <test-file.xml> ...]" >&2
    echo "  Run a single test:  $0 Testing/smoke_test.xml" >&2
    echo "  Run all tests:      $0 --all" >&2
    echo "  Run visually:       $0 --visual Testing/smoke_test.xml" >&2
    echo "  Run all visually:   $0 --visual --all" >&2
    exit 1
fi

PASS=0
FAIL=0
FAILED_TESTS=()

run_one() {
    local file="$1"

    local -a cmd=()
    if [ "${#TIMEOUT[@]}" -gt 0 ]; then
        cmd=("${TIMEOUT[@]}")
    fi
    cmd+=("$COCKATRICE" --no-connect --test "$file")

    if [ "$VISUAL" -eq 1 ]; then
        "${cmd[@]}"
    else
        env QT_QPA_PLATFORM=offscreen "${cmd[@]}"
    fi
}

for f in "${TEST_FILES[@]}"; do
    name="$(basename "$f")"
    printf "%-40s " "$name"
    if run_one "$f"; then
        echo "PASS"
        PASS=$((PASS + 1))
    else
        echo "FAIL"
        FAIL=$((FAIL + 1))
        FAILED_TESTS+=("$name")
    fi
done

TOTAL=$((PASS + FAIL))
echo ""
echo "Results: $PASS/$TOTAL passed, $FAIL failed"
if [ "$FAIL" -gt 0 ]; then
    echo "Failed:"
    for t in "${FAILED_TESTS[@]}"; do
        echo "  $t"
    done
    exit 1
fi
