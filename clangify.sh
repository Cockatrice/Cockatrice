#!/bin/bash

# This script will run clang-format on all modified, non-3rd-party C++/Header files.
# Never, ever, should this recieve a path with a newline in it. Don't bother proofing it for that.


# use script dir and return on exit
current=$PWD
function cleanup {
  cd $current
}
trap cleanup EXIT
cd "${BASH_SOURCE%/*}/"

# defaults
include=("common" \
"cockatrice/src" \
"oracle/src" \
"servatrice/src")
exclude=("cockatrice/src/qt-json" \
"servatrice/src/smtp" \
"common/sfmt" \
"oracle/src/zip")
exts=("cpp" "h")
cf_cmd="clang-format"
branch="origin/master"

# parse options
while [[ $@ ]]; do
  case "$1" in
    '-b'|'--branch')
      branch=$2
      set_branch=1
      shift 2
      ;;
    '-c'|'--color-diff')
      color=" --color=always"
      mode=diff
      shift
      ;;
    '-d'|'--diff')
      mode=diff
      shift
      ;;
    '-h'|'--help')
      cat <<EOM
A bash script to format your code using clang-format.
If no options are given, all dirty files are edited in place.
If <dir>s are given, all source files in those directories are checked, recursively.

USAGE: $0 [option] [-b[ranch] <git branch or object>] [<dir> ...]

DEFAULTS:
Default includes are:
  ${include[@]/%/
 }
Default excludes are:
  ${exclude[@]/%/
 }
OPTIONS:
    -b, --branch <branch>
        Compare to this git branch and format only files that differ.
        If unspecified it defaults to origin/master.
        To not compare to a branch this has to be explicitly set to "".
        When not comparing to a branch git will not be used at all and all valid files will be parsed.

    -c, --color-diff
        Display a colored diff. Implies --diff.
        Only available on systems which support 'diff --color'.

    -d, --diff
        Display a diff. Implies --test.

    -h, --help
        Display this message and exit.

    -n, --names
        Display a list of filenames that require formatting. Implies --test.

    -t, --test
        Do not edit files in place. Set exit code to 1 if changes are required.

EXIT CODES:
    0 on a successful format or if no files require formatting.
    1 if a file requires formatting.
    2 if given incorrect arguments.
    3 if clang-format could not be found.
EOM
      exit 0
      ;;
    '-n'|'--names')
      mode=name
      shift
      ;;
    '-t'|'--test')
      mode=code
      shift
      ;;
    '--')
      shift
      ;;
    *)
      include=() # empty includes
      while [[ $@ ]]; do
        if [[ -d $1 ]]; then
          include+=("$1")
          shift
        else
          echo "error in parsing arguments of $0: $1 is not a directory" >&2
          exit 2 # input error
        fi
      done
      if ! [[ $set_branch ]]; then
        unset branch # unset branch if not set explicitly
      fi
      break
      ;;
  esac
done

# check availability of clang-format
if ! hash $cf_cmd 2>/dev/null; then
  echo "could not find $cf_cmd" >&2
  # find any clang-format-x.x in /usr/bin
  cf_cmd=$(find /usr/bin -regex '.*/clang-format-[0-9]+\.[0-9]+' -print -quit)
  if [[ $cf_cmd ]]; then
    echo "found $cf_cmd instead" >&2
  else
    exit 3 # special exit code for missing dependency
  fi
fi

if [[ $branch ]]; then
  # get all dirty files through git
  if ! base=$(git merge-base ${branch} HEAD); then
    echo "could not find git merge base" >&2
    exit 2 # input error
  fi
  declare -a reg
  for ex in ${exts[@]}; do
    reg+=(${include[@]/%/.*\\.$ex\$})
  done
  names=$(git diff --name-only $base | grep ${reg[@]/#/-e ^})
else
  names=$(find ${include[@]} -type f -false ${exts[@]/#/-o -name *\\.})
fi

# filter excludes
names=$(<<<"$names" grep -v ${exclude[@]/#/-e ^})

if ! [[ $names ]]; then
  exit 0 # nothing to format means format is successful!
fi

# format
case $mode in
  diff)
    declare -i code=0
    for name in ${names[@]}; do
      if ! $cf_cmd "$name" | diff "$name" - -p $color; then
        code=1
      fi
    done
    exit $code
    ;;
  name)
    declare -i code=0
    for name in ${names[@]}; do
      if ! $cf_cmd "$name" | diff "$name" - -q >/dev/null; then
        echo "$name"
        code=1
      fi
    done
    exit $code
    ;;
  code)
    for name in ${names[@]}; do
      $cf_cmd "$name" | diff "$name" - -q >/dev/null || exit 1
    done
    ;;
  *)
    $cf_cmd -i $names
    ;;
esac
