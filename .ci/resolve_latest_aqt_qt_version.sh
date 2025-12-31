#!/bin/bash

# We cache thinned versions of universal ("fat") Qt binaries to reduce the size of the packages and caches.
# The universal binaries are install via aqtinstall. We use wildcards to get the latest patch version. E.g. "6.6.*" -> "6.6.3".
# This script is used to resolve the latest patch version of Qt.

# This script is meant to be used by the ci enironment on macos runners only.
# It uses the runner's GITHUB_OUTPUT env variable.

# Usage example: .ci/resolve_latest_aqt_qt_version.sh "6.6.*"
QT_SPEC=$1

# If version is already specific (no wildcard), use it as-is
if [[ "$QT_SPEC" != *"*" ]]; then
	echo "version=$QT_SPEC" >> "$GITHUB_OUTPUT"
	exit 0
fi

# Resolve latest patch
QT_RESOLVED=$(aqt list-qt mac desktop --spec "$QT_SPEC" --latest-version)
echo "QT_RESOLVED=$QT_RESOLVED"
if [ -z "$QT_RESOLVED" ]; then
	echo "Error: Could not resolve Qt version for $QT_SPEC"
	exit 1
fi

echo "version=$QT_RESOLVED" >> "$GITHUB_OUTPUT"
