#!/usr/bin/env bash

set -euo pipefail

# Used by the CI build script to rename package artifacts
#
# Appends PACKAGE_SUFFIX to the package's filename
#  <package>     = <filename><extension>
#  <package_new> = <filename><PACKAGE_SUFFIX><extension>
# PACKAGE_SUFFIX must be passed as the first argument to the script
# Adds output to GITHUB_OUTPUT
#
# Expected to be run in the repository root where CPack executes from and places its output binary
# Expects a single binary for package_pattern and picks the first match
# Expects <extension> to be e.g. ".dmg", ".deb" or ".exe" (".tar.gz" etc. with more than one dot will break)

# Initialize PACKAGE_SUFFIX from positional argument
PACKAGE_SUFFIX="${1:-}"

# Check variable
if [[ -z $PACKAGE_SUFFIX ]]; then
  echo "::error file=$0::Missing required argument: PACKAGE_SUFFIX"
  exit 2
fi

package_pattern="Cockatrice-*.*"

# Find package in current directory
package_path="$(find "$PWD" -maxdepth 1 -type f -name "$package_pattern" -print -quit)"

if [[ -z "$package_path" ]]; then
  echo "::error file=$0::Could not find package"
  exit 1
fi

# <package> = <filename><extension>
package="${package_path##*/}"  # remove folder path (keep e.g. "Cockatrice-3.0.0.deb")
filename="${package%.*}"       # remove extension (keep e.g. "Cockatrice-3.0.0")
extension=".${package##*.}"    # remove filename (keep e.g. ".deb")

# Rename package (build artifact)
filename_new="$filename$PACKAGE_SUFFIX"
package_new="$filename_new$extension"
package_path_new="$PWD/$package_new"

echo "Renaming '$package' to '$package_new'"
mv "$package_path" "$package_path_new"
du -h "$package_path_new"

echo "package_path=$package_path_new" >>"$GITHUB_OUTPUT"
echo "package=$package_new" >>"$GITHUB_OUTPUT"
echo "filename=$filename_new" >>"$GITHUB_OUTPUT"
