#!/bin/bash

# This script runs include-what-you-use on the given source files or
# directories, applies its suggestions (forward declarations, minimal and
# canonical #includes), then reformats the touched files with clang-format.
#
# Uses include-what-you-use, iwyu_tool.py, fix_includes.py, clang-format and
# python3. Requires a build directory configured with
# -DCMAKE_EXPORT_COMPILE_COMMANDS=ON to provide compile_commands.json.
#
# The Qt 6 mapping (qt6.imp) makes IWYU prefer capital includes like
# #include <QWidget> over #include <qwidget.h> and keeps forward declarations
# where possible. Regenerate it with tools/generate_qt_mappings.py whenever the
# installed Qt version changes.
#
# Usage:
#   ./iwyu.sh [-b <build-dir>] [-m <mapping>] [-j <jobs>] [-n] <files|dirs...>
#
# Options:
#   -b, --build-dir DIR   build dir with compile_commands.json (default: build)
#   -m, --mapping FILE    IWYU mapping file (default: qt6.imp)
#   -j, --jobs N          IWYU parallel jobs (default: 2)
#   -n, --dry-run         only print IWYU output, do not modify any file

set -o pipefail

# go to the project root directory, this file should be located in the project root directory
cd "${BASH_SOURCE%/*}/" || exit 2 # could not find path, this could happen with special links etc.

# defaults
build_dir="build"
mapping="qt6.imp"
jobs=2
dry_run=0

# 3rd party code that must not be touched by IWYU or clang-format
excludes=("libcockatrice_rng/libcockatrice/rng/sfmt/" \
"libcockatrice_utility/libcockatrice/utility/peglib.h" \
"oracle/src/lzma/" \
"oracle/src/qt-json/" \
"oracle/src/zip/" \
"servatrice/src/smtp/")

usage() {
  cat <<EOM
Usage: $0 [-b <build-dir>] [-m <mapping>] [-j <jobs>] [-n] <files|dirs...>

Runs include-what-you-use on the given files/directories, applies the
suggested include changes in place, and reformats touched files.

Options:
  -b, --build-dir DIR   build dir with compile_commands.json (default: build)
  -m, --mapping FILE    IWYU mapping file (default: qt6.imp)
  -j, --jobs N          IWYU parallel jobs (default: 2)
  -n, --dry-run         only print IWYU output, do not modify any file
  -h, --help            show this help
EOM
}

# parse options
while [[ $* ]]; do
  case "$1" in
    '-b'|'--build-dir')
      build_dir=$2
      shift 2
      ;;
    '-m'|'--mapping')
      mapping=$2
      shift 2
      ;;
    '-j'|'--jobs')
      jobs=$2
      shift 2
      ;;
    '-n'|'--dry-run')
      dry_run=1
      shift
      ;;
    '-h'|'--help')
      usage
      exit 0
      ;;
    *)
      targets+=("$1")
      shift
      ;;
  esac
done

# find the IWYU tools, preferring PATH and falling back to the LLVM dir
find_tool() {
  local tool=$1
  local path
  if path=$(command -v "$tool" 2>/dev/null); then
    echo "$path"
  elif [ -x "/usr/lib/llvm/22/bin/$tool" ]; then
    echo "/usr/lib/llvm/22/bin/$tool"
  else
    return 1
  fi
}

find_tool include-what-you-use > /dev/null || { echo "error: include-what-you-use not found"; exit 1; }
IWYU_TOOL=$(find_tool iwyu_tool.py) || { echo "error: iwyu_tool.py not found"; exit 1; }
FIX_INCLUDES=$(find_tool fix_includes.py) || { echo "error: fix_includes.py not found"; exit 1; }
command -v clang-format >/dev/null 2>&1 || { echo "error: clang-format not found"; exit 1; }

if [ "${#targets[@]}" -eq 0 ]; then
  echo "error: specify at least one source file or directory" >&2
  usage
  exit 1
fi

if [ ! -f "$mapping" ]; then
  echo "error: mapping file '$mapping' not found" >&2
  exit 1
fi

if [ ! -f "$build_dir/compile_commands.json" ]; then
  echo "error: '$build_dir/compile_commands.json' not found" >&2
  echo "configure the build with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON first" >&2
  exit 1
fi

tmpdir=$(mktemp -d /tmp/iwyu.XXXXXX)
trap 'rm -rf "$tmpdir"' EXIT

# filter the compilation database: drop excluded 3rd party code and strip the
# GCC-only flags that clang/include-what-you-use rejects
root="$PWD"
python3 - "$build_dir/compile_commands.json" "$root" "${excludes[@]}" <<'PYEOF' > "$tmpdir/compile_commands.json"
import json
import os
import re
import sys

cc_path, root = sys.argv[1], sys.argv[2]
excludes = sys.argv[3:]
root = os.path.normpath(root)
excl_norm = [os.path.normpath(os.path.join(root, e)) for e in excludes]

out = []
for entry in json.load(open(cc_path)):
    path = os.path.normpath(entry['file'])
    if not path.startswith(root + os.sep):
        continue
    # skip generated code: protobuf output and moc autogen files
    if re.search(r'\.pb\.(h|cc)$', path) or '_autogen/' in path:
        continue
    if any(path.startswith(e + os.sep) or path == e for e in excl_norm):
        continue
    command = entry.get('command', '')
    command = re.sub(r'-mno-direct-extern-access', '', command)
    command = re.sub(r'--coverage', '', command)
    out.append(dict(entry, command=command))

json.dump(out, sys.stdout)
PYEOF

mapping_abs="$PWD/$mapping"
if [ "$dry_run" = 1 ]; then
  python3 "$IWYU_TOOL" -p "$tmpdir" -o iwyu -j "$jobs" "${targets[@]}" \
    -- -Xiwyu "--mapping_file=$mapping_abs"
else
  before=$(git diff --name-only 2>/dev/null | sort -u)

  python3 "$IWYU_TOOL" -p "$tmpdir" -o iwyu -j "$jobs" "${targets[@]}" \
    -- -Xiwyu "--mapping_file=$mapping_abs" |
    python3 "$FIX_INCLUDES" --nosafe_headers --nocomments --noreorder --basedir="$root"

  # fix_includes can emit absolute paths for headers that the compiler resolved
  # through an absolute -I entry. Those are machine-specific and break every
  # other checkout and CI, so rewrite them into the project-relative quoted
  # form the codebase uses. Must run before clang-format so the regroup/sort
  # pass sees the final paths.
  python3 - "$root" <<'PYEOF'
import os
import re
import subprocess
import sys

root = os.path.normpath(sys.argv[1])
pattern = re.compile(r'^(\s*#\s*include\s*)"(/)')

# Collect from git so untracked scratch files are left alone.
changed = subprocess.run(
    ['git', 'diff', '--name-only', '--diff-filter=ACMR'],
    capture_output=True, text=True, check=True).stdout.split()

for rel in changed:
    if not rel.endswith(('.h', '.cpp', '.cc', '.cxx')):
        continue
    path = os.path.join(root, rel)
    if not os.path.isfile(path):
        continue
    src = open(path, encoding='utf-8').read()
    if '"' + os.sep not in src:
        continue
    out = []
    for line in src.split('\n'):
        m = pattern.match(line)
        if not m:
            out.append(line)
            continue
        target = os.path.normpath(line[m.end(1) + 1:].rstrip('"'))
        if not target.startswith(root + os.sep):
            out.append(line)
            continue
        spelled = os.path.relpath(target, os.path.dirname(path))
        out.append(f'{m.group(1)}"{spelled}"')
    new = '\n'.join(out)
    if new != src:
        open(path, 'w', encoding='utf-8').write(new)
PYEOF

  # reformat every file this IWYU pass modified (fix_includes also touches
  # headers pulled in by the requested files)
  touched=$(comm -13 <(printf '%s\n' "$before") <(git diff --name-only 2>/dev/null | sort -u))
  for file in $touched; do
    case "$file" in
      *.h|*.cpp)
        clang-format -i "$file"
        ;;
    esac
  done
fi
