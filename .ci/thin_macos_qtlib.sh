#!/bin/bash

# The macos binaries from aqt are fat (universal), so we thin them to the target architecture to reduce the size of
# the packages and caches using lipo.

# This script is meant to be used by the ci enironment on macos runners only.
# It uses the runner's GITHUB_WORKSPACE env variable.
arch=$(uname -m)
nproc=$(sysctl -n hw.ncpu)

function thin() {
  local libfile=$1
  if [[ $(file -b --mime-type "$libfile") == application/x-mach-binary* ]]; then
    echo "Processing $libfile"
    lipo "$libfile" -thin "$arch" -output "$libfile"
  fi
  return 0
}
export -f thin  # export to allow use in xargs
export arch
set -eo pipefail

echo "::group::Thinning Qt libraries to $arch using $nproc cores"
find "$GITHUB_WORKSPACE/Qt" -type f -print0 | xargs -0 -n1 -P"$nproc" -I{} bash -c "thin '{}'"
echo "::endgroup::"
