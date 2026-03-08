#!/bin/bash
# used by the ci to rename build artifacts
# renames the file to [original name][SUFFIX].[original extension]
# where SUFFIX is either available in the environment or as the first arg
# expected to be run in the build directory unless BUILD_DIR is set
# adds output to GITHUB_OUTPUT
builddir="${BUILD_DIR:=.}"
findrx="Cockatrice-*.*"

if [[ $1 ]]; then
  SUFFIX="$1"
fi

# check env
if [[ ! $SUFFIX ]]; then
  echo "::error file=$0::SUFFIX is missing"
  exit 2
fi

set -e

# find file
found="$(find "$builddir" -maxdepth 1 -type f -name "$findrx" -print -quit)"
path="${found%/*}"    # remove all including first "/" from right side
file="${found##*/}"    # remove all including last "/" from left side
if [[ ! $file ]]; then
  echo "::error file=$0::could not find package"
  exit 1
fi
oldpwd="$PWD"
if ! cd "$path"; then
  echo "::error file=$0::could not get file path"
  exit 1
fi

# set filename
name="${file%.*}"    # remove all including first "." from right side
new_name="$name$SUFFIX"
extension="${file##*.}"    # remove all including last "." from left side
filename="$new_name.$extension"
echo "renaming '$file' to '$filename'"
mv "$file" "$filename"

cd "$oldpwd"
relative_path="$path/$filename"
ls -l "$relative_path"
echo "path=$relative_path" >>"$GITHUB_OUTPUT"
echo "name=$new_name" >>"$GITHUB_OUTPUT"
