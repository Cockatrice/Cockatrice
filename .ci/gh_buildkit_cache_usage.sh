#!/bin/bash
# Reports GitHub Actions cache usage, grouped by key prefix (first segment before "-" or "/") or for one prefix.
#
# Usage: .ci/gh_buildkit_cache_usage.sh [-m|--markdown] [key-prefix]
#   -m, --markdown  output as a markdown table
#   No key-prefix:  all caches grouped by prefix.
#   One arg:        caches with that key prefix (default buildkit-blob).
# Requires: gh, jq

set -eo pipefail

LIMIT=1000
MARKDOWN=false

if ! command -v gh &>/dev/null; then
  echo "Error: gh (GitHub CLI) is required. Install from https://cli.github.com/" >&2
  exit 1
fi
if ! command -v jq &>/dev/null; then
  echo "Error: jq is required. Install with: brew install jq" >&2
  exit 1
fi

while [[ $# -gt 0 ]]; do
  case "$1" in
    -m|--markdown) MARKDOWN=true; shift ;;
    *) break ;;
  esac
done

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
  if [[ "$MARKDOWN" == true ]]; then
    echo "| PREFIX | COUNT | SIZE (GiB) |"
    echo "|--------|------:|----------:|"
    while IFS=$'\t' read -r prefix count gib; do
      echo "| $prefix | $count | $gib |"
    done < <(echo "$json" | jq -r '
      def prefix: (if (.key | test("/")) then (.key | split("/")[0]) else (.key | split("-")[0]) end);
      def to_gib: . / 1024 / 1024 / 1024 | . * 100 | floor / 100;
      map(. + {prefix: prefix}) | group_by(.prefix) |
      map({prefix: .[0].prefix, count: length, totalBytes: (map(.sizeInBytes) | add // 0)}) |
      sort_by(-.totalBytes) |
      .[] | "\(.prefix)\t\(.count)\t\(.totalBytes | to_gib)"
    ')
    echo "| **Total** | $total_entries | $total_gib |"
  else
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
    printf "%-12s %6s %10.2f\n" "Total" "$total_entries" "$total_gib"
  fi
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

if [[ "$MARKDOWN" == true ]]; then
  echo "| PREFIX | COUNT | SIZE (GiB) |"
  echo "|--------|------:|----------:|"
  echo "| $KEY_PREFIX | $count | $total_gib |"
  echo "| **Total** | $count | $total_gib |"
else
  printf "%-12s %6s %10s\n" "PREFIX" "COUNT" "SIZE (GiB)"
  printf "%-12s %6s %10s\n" "------------" "------" "----------"
  printf "%-12s %6s %10.2f\n" "$KEY_PREFIX" "$count" "$total_gib"
  printf "%-12s %6s %10.2f\n" "Total" "$count" "$total_gib"
fi
