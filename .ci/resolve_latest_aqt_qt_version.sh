#!/bin/bash

# This script is used to resolve the latest patch version of Qt using aqtinstall.
# It interprets wildcards to get the latest patch version. E.g. "6.6.*" -> "6.6.3".

# This script is meant to be used by the ci enironment.
# It uses the runner's GITHUB_OUTPUT env variable.

# Usage example: .ci/resolve_latest_aqt_qt_version.sh "6.6.*"

qt_spec=$1
if [[ ! $qt_spec ]]; then
  echo "usage: $0 [version]"
  exit 2
fi

# If version is already specific (no wildcard), use it as-is
if [[ $qt_spec != *"*" ]]; then
  echo "version $qt_spec is already resolved"
  echo "version=$qt_spec" >> "$GITHUB_OUTPUT"
  exit 0
fi

if ! hash aqt; then
  echo "aqt could not be found, has aqtinstall been installed?"
  exit 2
fi

# Resolve latest patch
if ! qt_resolved=$(aqt list-qt mac desktop --spec "$qt_spec" --latest-version); then
  exit 1
fi

echo "resolved $qt_spec to $qt_resolved"
if [[ ! $qt_resolved ]]; then
  echo "Error: Could not resolve Qt version for $qt_spec"
  exit 1
fi

echo "version=$qt_resolved" >> "$GITHUB_OUTPUT"
