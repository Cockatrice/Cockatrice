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

# Group cache keys by first segment (before "-" or "/"), sum sizes, output prefix, count, GiB (TSV).
JQ_GROUPED_TSV='
  def prefix: (if (.key | test("/")) then (.key | split("/")[0]) else (.key | split("-")[0]) end);
  def to_gib: . / 1024 / 1024 / 1024 | . * 100 | floor / 100;
  map(. + {prefix: prefix}) | group_by(.prefix) |
  map({prefix: .[0].prefix, count: length, totalBytes: (map(.sizeInBytes) | add // 0)}) |
  sort_by(-.totalBytes) |
  .[] | "\(.prefix)\t\(.count)\t\(.totalBytes | to_gib)"
'

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
  echo "Grouping all caches by prefix..."
  json=$(gh cache list --limit "$LIMIT" --json key,sizeInBytes 2>/dev/null) || {
    echo "Error: gh cache list failed (check gh auth or repo access)" >&2
    exit 1
  }
  rows=$(echo "$json" | jq -r "$JQ_GROUPED_TSV")
  total_entries=$(echo "$json" | jq 'length')
  total_bytes=$(echo "$json" | jq 'map(.sizeInBytes) | add // 0')
else
  echo "Reporting caches for prefix: $KEY_PREFIX"
  KEY_PREFIX="${1:-buildkit-blob}"
  json=$(gh cache list --key "$KEY_PREFIX" --limit "$LIMIT" --json key,sizeInBytes 2>/dev/null) || {
    echo "Error: gh cache list failed (check gh auth or repo access)" >&2
    exit 1
  }
  total_entries=$(echo "$json" | jq 'length')
  total_bytes=$(echo "$json" | jq 'map(.sizeInBytes) | add // 0')
fi

total_gib=$(echo "$total_bytes" | jq -r '. / 1024 / 1024 / 1024 | . * 100 | floor / 100')
[[ $# -ne 0 ]] && rows="${KEY_PREFIX}"$'\t'"${total_entries}"$'\t'"${total_gib}"

if [[ "$MARKDOWN" == true ]]; then
  echo "| PREFIX | COUNT | SIZE (GiB) |"
  echo "|--------|------:|----------:|"
  while IFS=$'\t' read -r prefix count gib; do
    echo "| $prefix | $count | $gib |"
  done <<< "$rows"
  echo "| **Total** | $total_entries | $total_gib |"
else
  printf "%-12s %6s %10s\n" "PREFIX" "COUNT" "SIZE (GiB)"
  printf "%-12s %6s %10s\n" "------------" "------" "----------"
  while IFS=$'\t' read -r prefix count gib; do
    printf "%-12s %6s %10.2f\n" "$prefix" "$count" "$gib"
  done <<< "$rows"
  printf "%-12s %6s %10.2f\n" "Total" "$total_entries" "$total_gib"
fi
