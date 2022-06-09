#!/bin/bash

# This script will run clang-format on all modified, non-3rd-party C++/Header files.
# Optionally runs cmake-format on all modified cmake files.
# Uses clang-format cmake-format git diff find
# Never, ever, should this receive a path with a newline in it. Don't bother proofing it for that.

set -o pipefail

# go to the project root directory, this file should be located in the project root directory
olddir="$PWD"
cd "${BASH_SOURCE%/*}/" || exit 2 # could not find path, this could happen with special links etc.

# defaults
include=("common" \
"cockatrice/src" \
"dbconverter/src" \
"oracle/src" \
"servatrice/src" \
"tests")
exclude=("servatrice/src/smtp" \
"common/sfmt" \
"common/lib" \
"oracle/src/zip" \
"oracle/src/lzma" \
"oracle/src/qt-json")
exts=("cpp" "h" "proto")
cf_cmd="clang-format"
branch="origin/master"
cmakefile="CMakeLists.txt"
cmakedir="cmake/.*\\.cmake"
cmakeinclude=("cmake/gtest-CMakeLists.txt.in")
color="--"

# parse options
while [[ $* ]]; do
  case "$1" in
    '-b'|'--branch')
      branch=$2
      set_branch=1
      shift 2
      ;;
    '--cmake')
      do_cmake=1
      shift
      ;;
    '-c'|'--color-diff')
      color="--color=always"
      mode="diff"
      shift
      ;;
    '-d'|'--diff')
      mode="diff"
      shift
      ;;
    '-h'|'--help')
      cat <<EOM
A bash script to automatically format your code using clang-format.

If no options are given, all dirty source files are edited in place.
If <dir>s are given, all source files in those directories of the project root
path are formatted. To only format changed files in these directories use the
--branch option in combination. <dir> has to be a path relative to the project
root path or a full path inside $PWD.

USAGE: $0 [option] [--branch <git branch or object>] [<dir> ...]

DEFAULTS:
Current includes are:
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
        When not comparing to a branch, git will not be used at all and every
        source file in the entire project will be parsed.

    --cmake
        Use cmake-format to format cmake files as well.

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

    --cf-version
        Print the version of clang-format being used before continuing.

EXIT CODES:
    0 on a successful format or if no files require formatting.
    1 if a file requires formatting.
    2 if given incorrect arguments.
    3 if clang-format could not be found.

EXAMPLES:
    $0 --branch $USER/patch-2 ${include[0]}
        Formats all changed files compared to the git branch "$USER/patch-2"
        in the directory ${include[0]}.

    $0 --test . || echo "code requires formatting"
        Tests if the source files in the current directory are correctly
        formatted and prints an error message if formatting is required.

    $0 --cmake --branch "" ""
        Unconditionally format all cmake files and no source files.
EOM
      exit 0
      ;;
    '-n'|'--names')
      mode="name"
      shift
      ;;
    '-t'|'--test')
      mode="code"
      shift
      ;;
    '--cf-version')
      print_version=1
      shift
      ;;
    '--')
      dashdash=1
      shift
      ;;
    *)
      if [[ ! $dashdash && $1 =~ ^-- ]]; then
        echo "error in parsing arguments of $0: $1 is an unrecognized option" >&2
        exit 2 # input error
      fi
      if [[ ! $1 ]] || next_dir=$(cd "$olddir" && cd -- "$1" && pwd); then
        if ! [[ $set_include ]]; then
          include=() # remove default includes
          set_include=1
        fi
        if [[ $1 ]]; then
          if [[ $next_dir != $PWD/* ]]; then
            echo "error in parsing arguments of $0: $next_dir is not in $PWD" >&2
            exit 2 # input error
          fi
          include+=("$next_dir")
        fi
      else
        echo "error in parsing arguments of $0: $1 is not a directory" >&2
        exit 2 # input error
      fi
      if ! [[ $set_branch ]]; then
        unset branch # unset branch if not set explicitly
      fi
      shift
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

# check availability of cmake-format
if [[ $do_cmake ]] && ! hash cmake-format 2>/dev/null; then
  echo "could not find cmake-format" >&2
  exit 3
fi

if [[ $branch ]]; then
  # get all dirty files through git
  if ! base=$(git merge-base "$branch" HEAD); then
    echo "could not find git merge base" >&2
    exit 2 # input error
  fi
  mapfile -t basenames < <(git diff --diff-filter=d --name-only "$base")
  names=()
  for ex in "${exts[@]}"; do
    for path in "${include[@]}"; do
      for name in "${basenames[@]}"; do
        rx="^$path/.$ex$"
        if [[ $name =~ $rx ]]; then
          names+=("$name")
        fi
      done
    done
  done
  if [[ $do_cmake ]]; then
    cmake_names=()
    for name in "${basenames[@]}"; do
      dirrx="^$cmakedir$"
      filerx="(^|/)$cmakefile$"
      if [[ $name =~ $dirrx || $name =~ $filerx ]]; then
        cmake_names+=("$name")
      fi
      for include in "${cmakeinclude[@]}"; do
        if [[ $name == "$include" ]]; then
          cmake_names+=("$name")
        fi
      done
    done
  fi
else
  exts_o=()
  for ext in "${exts[@]}"; do
    exts_o+=(-o -name "*\\.$ext")
  done
  unset "exts_o[0]" # remove first -o
  mapfile -t names < <(find "${include[@]}" -type f "${exts_o[@]}")
  if [[ $do_cmake ]]; then
    mapfile -t cmake_names < <(find . -maxdepth 2 -type f -name "$cmakefile" -o -path "./${cmakedir/.}")
    cmake_names+=("${cmakeinclude[@]}")
  fi
fi

# filter excludes
for path in "${exclude[@]}"; do
  for i in "${!names[@]}"; do
    rx="^$path/"
    if [[ ${names[$i]} =~ $rx ]]; then
      unset "names[$i]"
    fi
  done
done

# optionally print version
if [[ $print_version ]]; then
  $cf_cmd -version
  [[ $do_cmake ]] && echo "cmake-format $(cmake-format --version)"
  echo "----------"
fi

if [[ ! ${cmake_names[*]} ]]; then
  unset do_cmake
fi
if [[ ! ( ${names[*]} || $do_cmake ) ]]; then
  exit 0 # nothing to format means format is successful!
fi

# format
case $mode in
  diff)
    declare -i code=0
    for name in "${names[@]}"; do
      if ! $cf_cmd "$name" | diff "$name" - -p "$color"; then
        code=1
      fi
    done
    for name in "${cmake_names[@]}"; do
      if ! cmake-format "$name" | diff "$name" - -p "$color"; then
        code=1
      fi
    done
    exit $code
    ;;
  name)
    declare -i code=0
    for name in "${names[@]}"; do
      if ! $cf_cmd "$name" | diff "$name" - -q >/dev/null; then
        echo "$name"
        code=1
      fi
    done
    for name in "${cmake_names[@]}"; do
      if ! cmake-format "$name" --check; then
        echo "$name"
        code=1
      fi
    done
    exit $code
    ;;
  code)
    for name in "${names[@]}"; do
      $cf_cmd "$name" | diff "$name" - -q >/dev/null || exit 1
    done
    for name in "${cmake_names[@]}"; do
      cmake-format "$name" --check || exit 1
    done
    ;;
  *)
    if [[ "${names[*]}" ]]; then
      $cf_cmd -i "${names[@]}"
    fi
    if [[ $do_cmake ]]; then
      cmake-format -i "${cmake_names[@]}"
    fi
    ;;
esac
