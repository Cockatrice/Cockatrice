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
#   COCKATRICE_BIN   Path to the client binary (default: build/debug/cockatrice/cockatrice).
#   TEST_TIMEOUT     Seconds a single test may run before it is killed (default: 180).
#
# Requires the debug build to exist at build/debug/cockatrice/cockatrice.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd -P)"
COCKATRICE="${COCKATRICE_BIN:-$REPO_ROOT/build/debug/cockatrice/cockatrice}"
TEST_DIR="$REPO_ROOT/Testing"
TEST_TIMEOUT="${TEST_TIMEOUT:-180}"

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
    local name
    name="$(basename "$file")"

    if [ "$VISUAL" -eq 1 ]; then
        if timeout --foreground "$TEST_TIMEOUT" "$COCKATRICE" --no-connect --test "$file"; then
            return 0
        else
            return 1
        fi
    else
        if timeout --foreground "$TEST_TIMEOUT" env QT_QPA_PLATFORM=offscreen "$COCKATRICE" --no-connect --test "$file"; then
            return 0
        else
            return 1
        fi
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
