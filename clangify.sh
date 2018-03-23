#!/bin/bash

# This script will run clang-format on all modified, non-3rd-party C++/Header files.

set -e

if hash clang-format 2>/dev/null && hash git 2>/dev/null; then
	files_to_clean=($(git diff --name-only $(git merge-base origin/master HEAD)))

	printf "%s\n" ${files_to_clean[@]} | \
		xargs -I{} find '{}' \( -name "*.cpp" -o -name "*.h" \) \
		-not -path "./cockatrice/src/qt-json/*" \
		-not -path "./servatrice/src/smtp/*" \
		-not -path "./common/sfmt/*" \
		-not -path "./oracle/src/zip/*" \
		-not -path "./build*/*" \
		-exec clang-format -style=file -i {} \;
	echo "Successfully formatted following files:"
	printf "%s\n" ${files_to_clean[@]}
else
	echo "Please install clang-format and git to use this program"
fi
