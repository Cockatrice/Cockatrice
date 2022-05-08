#!/bin/bash

# go to the project root directory, this file should be located in the project root directory
cd "${BASH_SOURCE%/*}/" || exit 2 # could not find path, this could happen with special links etc.

# CMake-Format
cmake_format_cmd="cmake-format"

if ! hash $cmake_format_cmd 2>/dev/null; then
  echo "could not find $cmake_format_cmd, skipping" >&2
  return
fi

$cmake_format_cmd -i cmake/*.cmake */CMakeLists.txt
