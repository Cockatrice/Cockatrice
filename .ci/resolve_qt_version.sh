#!/bin/bash

# This script is used to resolve the latest patch version of Qt (and latest-1 in range) using aqtinstall.
# It interprets wildcards to get the latest patch version, e.g. "6.6.*" --> "6.6.3".
# For fallbacks, it calculates the previous patch version for wildcard inputs as well, e.g. "6.6.*" --> "6.6.3" --> "6.6.2".

# This script is meant to be used by the CI environment.
# It uses the runner's GITHUB_OUTPUT to store environment variables.

# Usage example: .ci/resolve_qt_version.sh "6.6.*"

qt_version_input="$1"
if [[ -z $qt_version_input ]]; then
  echo "usage: $0 [version]"
  exit 2
fi

# If version is already specific (no wildcard), use it as-is and skip calculating the previous patch version
if [[ $qt_version_input != *"*" ]]; then
  echo "Version $qt_version_input is already resolved"
  echo "version=$qt_version_input" >> "$GITHUB_OUTPUT"
  exit 0
fi

if ! hash aqt; then
  echo "::error::aqt could not be found, ensure aqtinstall is installed"
  exit 2
fi

# Resolve latest patch
if [[ $RUNNER_OS == macOS ]]; then
  if ! qt_resolved=$(aqt list-qt mac desktop --spec "$qt_version_input" --latest-version); then
    exit 1
  fi
elif [[ $RUNNER_OS == Windows ]]; then
  if ! qt_resolved=$(aqt list-qt windows desktop --spec "$qt_version_input" --latest-version); then
    exit 1
  fi
else
  echo "::error::aqt command for $RUNNER_OS not defined."
  exit 1
fi

if [[ -n $qt_resolved ]]; then
  echo "Resolved $qt_version_input to $qt_resolved"
else
  echo "::error::Could not resolve Qt version for $qt_version_input"
  exit 1
fi


# Calculate the previous patch version
# Do not roll back to the previous minor version (e.g. 6.6.0 will not return 6.5.99).
qt_previous_patch=""

if [[ $qt_resolved =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)$ ]]; then
  qt_major=${BASH_REMATCH[1]}
  qt_minor=${BASH_REMATCH[2]}
  qt_patch=${BASH_REMATCH[3]}

  if (( qt_patch > 0 )); then
    qt_previous_patch="$qt_major.$qt_minor.$((qt_patch - 1))"
  fi
else
  echo "::error::Resolved Qt version has unexpected SemVer format: $qt_resolved"
  exit 1
fi

if [[ -n $qt_previous_patch ]]; then
  echo "Calculated previous patch version as $qt_previous_patch"
else
  echo "Keeping previous patch version empty because $qt_resolved is the first patch version"
fi

echo "version=$qt_resolved" >> "$GITHUB_OUTPUT"
echo "version_previous_patch=$qt_previous_patch" >> "$GITHUB_OUTPUT"
