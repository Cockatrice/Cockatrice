#!/bin/bash
# this script is to be used by the ci to fetch the github upload url
# using curl and jq
[[ $ref ]] || missing+=" ref"
[[ $repo ]] || missing+=" repo"
if [[ $missing ]]; then
  echo "missing env:$missing" >&2
  exit 2
fi
tag="${ref##*/}"
api_url="https://api.github.com/repos/$repo/releases/tags/$tag"
upload_url="$(curl "$api_url" | jq -r '.upload_url')"
if [[ $upload_url && $upload_url != null ]]; then
  echo "$upload_url"
  exit 0
else
  echo "failed to fetch upload url from $api_url" >&2
  exit 1
fi
