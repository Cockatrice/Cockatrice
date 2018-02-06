#!/bin/bash

# This script will run clang-format on all non-3rd-party C++/Header files.

set -e

if hash clang-format 2>/dev/null; then
	find . \( -name "*.cpp" -o -name "*.h" \) \
		-not -path "./cockatrice/src/qt-json/*" \
		-not -path "./servatrice/src/smtp/*" \
		-not -path "./common/sfmt/*" \
		-not -path "./oracle/src/zip/*" \
		-not -path "./build*/*" \
		-exec clang-format -i {} \;
	echo "Repository properly formatted"
else
	echo "Please install clang-format to use this program"
fi