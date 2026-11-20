#!/usr/bin/env bash
#
# Run a local Servatrice with the checked-in development profile.
#
# The default profile needs no database: it listens on localhost, accepts
# guest logins, and exposes the General and Testing rooms from
# servatrice/dev.ini. Pass --mysql to run against the seeded development
# database managed by scripts/dev-db.sh.
#
# Any unrecognised argument is passed through to servatrice unchanged, so
# options such as --set key=value or --test-hash keep working.

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
config="$repo_root/servatrice/dev.ini"
database_url="${DATABASE_URL:-}"
port=""
log_to_console=1
passthrough=()

usage() {
    cat <<'EOF'
Usage: scripts/dev-servatrice.sh [options] [-- servatrice-options...]

Options:
  --config FILE        Configuration file (default: servatrice/dev.ini)
  --port PORT          Listen on PORT instead of the configured port
  --mysql[=URL]        Use the seeded MySQL dev database
  --database-url URL   Use an explicit mysql:// URL
  --no-log-to-console  Do not force --log-to-console
  -h, --help           Show this help

Environment:
  SERVATRICE_BIN       Servatrice binary to run
  DATABASE_URL         Database URL used by --mysql when no URL is given

Examples:
  scripts/dev-servatrice.sh
  scripts/dev-servatrice.sh --port 14747
  scripts/dev-db.sh up && scripts/dev-servatrice.sh --mysql
EOF
}

die() {
    echo "dev-servatrice: $*" >&2
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --config)
            [[ $# -ge 2 ]] || die "--config needs a file"
            config="$2"
            shift 2
            ;;
        --config=*)
            config="${1#*=}"
            shift
            ;;
        --port)
            [[ $# -ge 2 ]] || die "--port needs a value"
            port="$2"
            shift 2
            ;;
        --port=*)
            port="${1#*=}"
            shift
            ;;
        --mysql)
            database_url="${DATABASE_URL:-$(bash "$repo_root/scripts/dev-db.sh" url)}"
            shift
            ;;
        --mysql=*)
            database_url="${1#*=}"
            shift
            ;;
        --database-url)
            [[ $# -ge 2 ]] || die "--database-url needs a value"
            database_url="$2"
            shift 2
            ;;
        --database-url=*)
            database_url="${1#*=}"
            shift
            ;;
        --no-log-to-console)
            log_to_console=0
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

[[ -f "$config" ]] || die "configuration file not found: $config"

if [[ -n "${SERVATRICE_BIN:-}" ]]; then
    binary="$SERVATRICE_BIN"
elif [[ -x "$repo_root/build/test/servatrice/servatrice" ]]; then
    binary="$repo_root/build/test/servatrice/servatrice"
elif [[ -x "$repo_root/build/debug/servatrice/servatrice" ]]; then
    binary="$repo_root/build/debug/servatrice/servatrice"
else
    die "no servatrice binary found; build it with:
  cmake -S . -B build/test -DCMAKE_BUILD_TYPE=Debug -DWITH_SERVER=1 -DTEST=1
  cmake --build build/test -j 24"
fi

args=(--config "$config")
[[ "$log_to_console" -eq 1 ]] && args+=(--log-to-console)
[[ -n "$port" ]] && args+=("--set" "server/port=$port")
args+=("${passthrough[@]}")

# Resolve the effective host and port through the binary itself, so a port in
# the ini file and --set overrides are both reflected in the printed command.
print_args=(--config "$config")
[[ -n "$port" ]] && print_args+=(--set "server/port=$port")
effective_config="$("$binary" "${print_args[@]}" --print-config 2>/dev/null || true)"
effective_port="$(printf '%s\n' "$effective_config" | awk -F= '$1 == "server/port" { print $2; exit }')"
effective_host="$(printf '%s\n' "$effective_config" | awk -F= '$1 == "server/host" { print $2; exit }')"
effective_port="${effective_port:-${port:-4747}}"
if [[ -z "$effective_host" || "$effective_host" == "any" ]]; then
    effective_host="127.0.0.1"
fi

if [[ -n "$database_url" ]]; then
    export DATABASE_URL="$database_url"
    redacted_url="$(printf '%s' "$database_url" | sed -E 's#(://[^:/@]+):[^@]*@#\1:***@#')"
    echo "Database: $redacted_url"
fi

echo "Servatrice listening on ${effective_host}:${effective_port}"
echo "Connect with: scripts/dev-client.sh --connect devplayer@${effective_host}:${effective_port}"
echo

exec "$binary" "${args[@]}"
