#!/bin/bash

# This script will run clang-format on all modified, non-3rd-party C++/Header files.

set -e

if hash clang-format 2>/dev/null && hash git 2>/dev/null; then
	git diff --name-only | xargs -I{} find '{}' \( -name "*.cpp" -o -name "*.h" \) \
		-not -path "./cockatrice/src/qt-json/*" \
		-not -path "./servatrice/src/smtp/*" \
		-not -path "./common/sfmt/*" \
		-not -path "./oracle/src/zip/*" \
		-not -path "./build*/*" \
		-exec clang-format -style=file -i {} \;
	echo "Repository properly formatted"
else
	echo "Please install clang-format to use this program"
fi
