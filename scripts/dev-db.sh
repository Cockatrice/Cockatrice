#!/usr/bin/env bash
#
# Manage the seeded MySQL development database.
#
# The stack is defined in docker-compose.dev.yml. On a fresh volume MySQL runs
# servatrice/servatrice.sql and then servatrice/dev/seed.sql, so the database
# starts with accounts, rooms, channels, and moderation fixtures already in
# place.

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
compose_file="$repo_root/docker-compose.dev.yml"
seed_file="$repo_root/servatrice/dev/seed.sql"
url="${COCKATRICE_DEV_DATABASE_URL:-mysql://servatrice:password@127.0.0.1:3306/servatrice}"

usage() {
    cat <<'EOF'
Usage: scripts/dev-db.sh <command>

Commands:
  up       Start MySQL and wait until it is healthy
  seed     Re-apply servatrice/dev/seed.sql to the running database
  reset    Remove the container and volume, then start a fresh seeded database
  stop     Stop the container, keeping the database volume
  url      Print the DATABASE_URL servatrice expects

Environment:
  COCKATRICE_DEV_DATABASE_URL  Use an external MySQL instead of the compose stack

Examples:
  scripts/dev-db.sh up
  scripts/dev-db.sh seed
  scripts/dev-db.sh reset
EOF
}

die() {
    echo "dev-db: $*" >&2
    exit 1
}

compose=()
detect_compose() {
    if docker compose version >/dev/null 2>&1; then
        compose=(docker compose)
    elif command -v docker-compose >/dev/null 2>&1; then
        compose=(docker-compose)
    else
        die "Docker Compose is not available; install the docker compose plugin or set COCKATRICE_DEV_DATABASE_URL"
    fi
}

compose_cmd() {
    "${compose[@]}" -f "$compose_file" "$@"
}

command="${1:-}"
[[ -n "$command" ]] || {
    usage
    exit 1
}

case "$command" in
    url)
        echo "$url"
        ;;
    up)
        if [[ -n "${COCKATRICE_DEV_DATABASE_URL:-}" ]]; then
            echo "Using external database: $url"
            echo "Apply the schema and seed yourself if the database is empty:"
            echo "  mysql < servatrice/servatrice.sql"
            echo "  mysql < servatrice/dev/seed.sql"
            exit 0
        fi
        detect_compose
        compose_cmd up -d --wait
        echo
        echo "MySQL is ready."
        echo "DATABASE_URL=$(bash "$repo_root/scripts/dev-db.sh" url)"
        echo "Start the server with: scripts/dev-servatrice.sh --mysql"
        ;;
    seed)
        [[ -f "$seed_file" ]] || die "seed file not found: $seed_file"
        if [[ -n "${COCKATRICE_DEV_DATABASE_URL:-}" ]]; then
            die "seed is only automated for the compose stack; apply $seed_file manually for an external database"
        fi
        detect_compose
        compose_cmd exec -T mysql mysql -uservatrice -ppassword servatrice < "$seed_file"
        echo "Seed applied."
        ;;
    reset)
        if [[ -n "${COCKATRICE_DEV_DATABASE_URL:-}" ]]; then
            die "reset only manages the compose stack; reset the external database yourself"
        fi
        detect_compose
        compose_cmd down -v
        compose_cmd up -d --wait
        echo
        echo "Fresh seeded database ready."
        echo "Start the server with: scripts/dev-servatrice.sh --mysql"
        ;;
    stop)
        if [[ -n "${COCKATRICE_DEV_DATABASE_URL:-}" ]]; then
            die "stop only manages the compose stack"
        fi
        detect_compose
        compose_cmd down
        ;;
    -h|--help|help)
        usage
        ;;
    *)
        usage
        die "unknown command: $command"
        ;;
esac
