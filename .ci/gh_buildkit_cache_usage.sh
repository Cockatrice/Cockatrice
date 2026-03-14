#!/bin/bash
# Reports total count and size of GitHub Actions caches whose key starts with a given prefix.
# Used for BuildKit GHA cache (keys like buildkit-blob-1-sha256:...).
#
# Requires: gh (GitHub CLI), jq
# Run from repo root with gh auth login already done (or GITHUB_TOKEN set in CI).
#
# Usage: .ci/gh_buildkit_cache_usage.sh [key-prefix]
#   Default key-prefix: buildkit-blob
#
# Example: .ci/gh_buildkit_cache_usage.sh
# Example: .ci/gh_buildkit_cache_usage.sh buildkit-blob

set -eo pipefail

KEY_PREFIX="${1:-buildkit-blob}"
LIMIT=1000

if ! command -v gh &>/dev/null; then
  echo "Error: gh (GitHub CLI) is required. Install from https://cli.github.com/" >&2
  exit 1
fi
if ! command -v jq &>/dev/null; then
  echo "Error: jq is required. Install with: brew install jq" >&2
  exit 1
fi

json=$(gh cache list --key "$KEY_PREFIX" --limit "$LIMIT" --json key,sizeInBytes 2>/dev/null) || {
  echo "Error: gh cache list failed (check gh auth or repo access)" >&2
  exit 1
}

count=$(echo "$json" | jq 'length')
total_bytes=$(echo "$json" | jq 'map(.sizeInBytes) | add // 0')

# Human-readable size (GiB, 2 decimals)
total_gib=$(echo "$total_bytes" | jq -r '. / 1024 / 1024 / 1024 | . * 100 | floor / 100')

echo "Cache key prefix: $KEY_PREFIX"
echo "Entries: $count"
echo "Total size: $total_bytes bytes ($total_gib GiB)"
