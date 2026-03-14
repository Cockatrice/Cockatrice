#!/bin/bash
# Reports GitHub Actions cache usage: either grouped by key prefix or for a single prefix.
# Prefix = first segment of key (before first "-" or "/"). E.g. buildkit-blob-* -> buildkit, vcpkg/xxx -> vcpkg.
#
# Requires: gh (GitHub CLI), jq
# Run from repo root with gh auth login already done (or GITHUB_TOKEN set in CI).
#
# Usage: .ci/gh_buildkit_cache_usage.sh [key-prefix]
#   No args: fetch all caches, group by prefix, print table (count + size per prefix) and grand total.
#   One arg: report only caches whose key starts with key-prefix (default: buildkit-blob).
#
# Example: .ci/gh_buildkit_cache_usage.sh              # all prefixes
# Example: .ci/gh_buildkit_cache_usage.sh buildkit-blob # single prefix

set -eo pipefail

LIMIT=1000

if ! command -v gh &>/dev/null; then
  echo "Error: gh (GitHub CLI) is required. Install from https://cli.github.com/" >&2
  exit 1
fi
if ! command -v jq &>/dev/null; then
  echo "Error: jq is required. Install with: brew install jq" >&2
  exit 1
fi

if [[ $# -eq 0 ]]; then
  # Grouped mode: all caches, group by first segment (before - or /)
  json=$(gh cache list --limit "$LIMIT" --json key,sizeInBytes 2>/dev/null) || {
    echo "Error: gh cache list failed (check gh auth or repo access)" >&2
    exit 1
  }
  # Add prefix field, group by prefix, sum count and size, sort by size desc; output GiB (2 decimals)
  total_entries=$(echo "$json" | jq 'length')
  total_bytes=$(echo "$json" | jq 'map(.sizeInBytes) | add // 0')
  total_gib=$(echo "$total_bytes" | jq -r '. / 1024 / 1024 / 1024 | . * 100 | floor / 100')
  printf "%-12s %6s %10s\n" "PREFIX" "COUNT" "SIZE (GiB)"
  printf "%-12s %6s %10s\n" "------------" "------" "----------"
  while IFS=$'\t' read -r prefix count gib; do
    printf "%-12s %6s %10.2f\n" "$prefix" "$count" "$gib"
  done < <(echo "$json" | jq -r '
    def prefix: (if (.key | test("/")) then (.key | split("/")[0]) else (.key | split("-")[0]) end);
    def to_gib: . / 1024 / 1024 / 1024 | . * 100 | floor / 100;
    map(. + {prefix: prefix}) | group_by(.prefix) |
    map({prefix: .[0].prefix, count: length, totalBytes: (map(.sizeInBytes) | add // 0)}) |
    sort_by(-.totalBytes) |
    .[] | "\(.prefix)\t\(.count)\t\(.totalBytes | to_gib)"
  ')
  echo ""
  printf "Total: %s entries, %s GiB\n" "$total_entries" "$total_gib"
  exit 0
fi

# Single-prefix mode
KEY_PREFIX="${1:-buildkit-blob}"
json=$(gh cache list --key "$KEY_PREFIX" --limit "$LIMIT" --json key,sizeInBytes 2>/dev/null) || {
  echo "Error: gh cache list failed (check gh auth or repo access)" >&2
  exit 1
}

count=$(echo "$json" | jq 'length')
total_bytes=$(echo "$json" | jq 'map(.sizeInBytes) | add // 0')
total_gib=$(echo "$total_bytes" | jq -r '. / 1024 / 1024 / 1024 | . * 100 | floor / 100')

echo "Cache key prefix: $KEY_PREFIX"
echo "Entries: $count"
echo "Total size: $total_bytes bytes ($total_gib GiB)"
