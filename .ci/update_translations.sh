#!/bin/bash

# ci script to update translation files
# usage:
# $0 cockatrice/cockatrice_en@source.ts cockatrice/src common
# or
# FILE="cockatrice/cockatrice_en@source.ts"
# DIRS="cockatrice/src common"
# $0
# note: directories can't contain spaces

# check parameters
if [[ ! $FILE ]]; then
  FILE="$1"
  shift
fi
if [[ ! $FILE ]]; then
  echo "no output file selected" >&2
  exit 2;
fi
if [[ ! $DIRS ]]; then
  DIRS="$*"
fi
if [[ ! $DIRS ]]; then
  echo "no source directories selected to translate" >&2
  exit 2;
fi
if [[ ! -e $FILE ]]; then
  echo "output file does not exist at: $FILE" >&2
  exit 3;
fi

# print version
if ! lupdate -version; then
  echo "failed to run lupdate" >&2
  exit 4;
fi

# run lupdate, duplicating the output in stderr and saving it
# for convenience we ignore that $DIRS will be split on spaces 
# shellcheck disable=SC2086
if ! got="$(lupdate $DIRS -ts "$FILE" | tee /dev/stderr)"; then
  echo "failed to update $FILE with $DIRS" >&2
  exit 4;
fi

# trim output
output="${got##*(}" # trim everything before last (
output="${output%%)*}" # trim everything after first )
if [[ $output == $got ]]; then
  echo "could not parse generated output" >&2
  exit 4;
fi

# write output to ci environment file
echo "output=$output" >> $GITHUB_OUTPUT
