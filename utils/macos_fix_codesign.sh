#!/bin/bash
# Fix code signing for unsigned macOS app bundles (mainly used to test packages from PRs on CI).
# Run from project root: ./utils/macos_fix_codesign.sh [path/to/app.app ...]
#
# Steps:
#   1. Remove quarantine extended attribute
#   2. Ad-hoc sign all dylibs, frameworks and executables inside the bundle
#   3. Deep sign the app bundle
#
# Use after installing an unsigned build to /Applications.
# With no args, fixes cockatrice, oracle, and servatrice in /Applications.

set -eo pipefail

DEFAULT_APPS=(
  /Applications/cockatrice.app
  /Applications/oracle.app
  /Applications/servatrice.app
)

is_safe_app_bundle() {
  local resolved_app="$1"

  [[ "$resolved_app" == *.app ]] || { echo "Error: Path must end with .app (got '$resolved_app')" >&2; return 1; }

  case "$resolved_app" in
    / | /Applications/ | /System | /Users | /usr | /Library)
      echo "Error: Path '$resolved_app' is a system-critical directory" >&2
      return 1
      ;;
    /System/* | /usr/* | /Library/*)
      echo "Error: Path '$resolved_app' is under a system-critical directory" >&2
      return 1
      ;;
    *)
      # Not a system-critical directory; continue validation
      ;;
  esac

  [[ -d "$resolved_app/Contents" ]] || { echo "Error: Path '$resolved_app' lacks Contents/ (not a valid app bundle)" >&2; return 1; }

  return 0
}

fix_app() {
  local app="$1"
  if [[ ! -d "$app" ]]; then
    echo "Skipping (not found): $app"
    return 0
  fi

  if [[ -L "$app" ]]; then
    echo "Error: Symlinks are not allowed (got '$app'). Use a real path to the app bundle." >&2
    return 1
  fi

  local resolved_app
  resolved_app="$(cd -P -- "$app" && pwd -P)" || {
    echo "Error: Failed to resolve path '$app'" >&2
    return 1
  }

  if ! is_safe_app_bundle "$resolved_app"; then
    echo "Error: '$app' is not a valid app bundle path. Use a path ending in .app (e.g. /Applications/MyApp.app)." >&2
    return 1
  fi

  echo "Fixing code signing for: $app"

  # 1. Remove quarantine
  echo "  Removing quarantine..."
  xattr -dr com.apple.quarantine -- "$resolved_app"

  # 2. Ad-hoc sign all binaries and dylibs
  echo "  Signing dylibs, frameworks and executables..."
  # find .dylib files by name and executable files (binaries, frameworks, etc.) by permission via `-perm +111`
  find -- "$resolved_app" -type f \( -name "*.dylib" -o -perm +111 \) -print0 | \
    xargs -0 -I {} codesign -s - -f -- {}

  # 3. Deep sign the app bundle
  echo "  Deep signing app bundle..."
  codesign -s - --force --deep -- "$resolved_app"

  echo "  Done."
}

if [[ $# -gt 0 ]]; then
  for app in "$@"; do
    fix_app "$app"
  done
else
  for app in "${DEFAULT_APPS[@]}"; do
    fix_app "$app"
  done
fi

echo "All done. Try: open /Applications/cockatrice.app"
