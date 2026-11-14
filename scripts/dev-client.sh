#!/usr/bin/env bash
#
# Run a Cockatrice client with disposable, per-profile settings.
#
# Every profile gets its own XDG data, cache, and config directories under
# build/dev/profiles/<name>/, so several clients can run side by side with
# separate settings and card databases.
#
# Unrecognised arguments are passed through to the client unchanged.

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
profile="default"
connect="${COCKATRICE_DEV_CONNECT:-devplayer@127.0.0.1:4747}"
no_connect=0
passthrough=()

usage() {
    cat <<'EOF'
Usage: scripts/dev-client.sh [options] [-- client-options...]

Options:
  --profile NAME     Isolated settings profile (default: default)
  --connect URL      Connect on startup (default: devplayer@127.0.0.1:4747)
  --no-connect       Start without connecting anywhere
  -h, --help         Show this help

Environment:
  COCKATRICE_BIN        Client binary to run
  COCKATRICE_DEV_HOME   Base directory for profiles (default: build/dev)
  COCKATRICE_DEV_CONNECT  Default --connect value

Examples:
  scripts/dev-client.sh
  scripts/dev-client.sh --profile second --connect devplayer2@127.0.0.1:4747
  scripts/dev-client.sh --no-connect --profile scratch
EOF
}

die() {
    echo "dev-client: $*" >&2
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --profile)
            [[ $# -ge 2 ]] || die "--profile needs a name"
            profile="$2"
            shift 2
            ;;
        --profile=*)
            profile="${1#*=}"
            shift
            ;;
        --connect)
            [[ $# -ge 2 ]] || die "--connect needs a value"
            connect="$2"
            shift 2
            ;;
        --connect=*)
            connect="${1#*=}"
            shift
            ;;
        --no-connect)
            no_connect=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            passthrough+=("$@")
            break
            ;;
        *)
            passthrough+=("$1")
            shift
            ;;
    esac
done

[[ "$profile" =~ ^[A-Za-z0-9._-]+$ ]] || die "profile name may only contain letters, numbers, '.', '_', and '-'"

if [[ -n "${COCKATRICE_BIN:-}" ]]; then
    binary="$COCKATRICE_BIN"
elif [[ -x "$repo_root/build/debug/cockatrice/cockatrice" ]]; then
    binary="$repo_root/build/debug/cockatrice/cockatrice"
elif [[ -x "$repo_root/build/test/cockatrice/cockatrice" ]]; then
    binary="$repo_root/build/test/cockatrice/cockatrice"
else
    die "no cockatrice binary found; build it with:
  cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug -DWITH_SERVER=0 -DWITH_GUI_TEST=ON
  cmake --build build/debug -j 24"
fi

base="${COCKATRICE_DEV_HOME:-$repo_root/build/dev}"
profile_dir="$base/profiles/$profile"

export XDG_DATA_HOME="$profile_dir/data"
export XDG_CACHE_HOME="$profile_dir/cache"
export XDG_CONFIG_HOME="$profile_dir/config"
mkdir -p "$XDG_DATA_HOME" "$XDG_CACHE_HOME" "$XDG_CONFIG_HOME"

args=()
if [[ "$no_connect" -eq 0 ]]; then
    has_connect=0
    for arg in "${passthrough[@]}"; do
        case "$arg" in
            --connect|--connect=*|-c) has_connect=1 ;;
        esac
    done
    if [[ "$has_connect" -eq 0 ]]; then
        args+=(--connect "$connect")
        echo "Connecting to: $connect"
    fi
else
    args+=(--no-connect)
fi
args+=("${passthrough[@]}")

echo "Client profile: $profile ($profile_dir)"

exec "$binary" "${args[@]}"
