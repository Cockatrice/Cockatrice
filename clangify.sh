#!/bin/bash

# This script will run clang-format on all non-3rd-party C++/Header files.

set -e

if [[ $TRAVIS == false ]]; then
    echo "Applying code style to repository..."
fi

if hash clang-format 2>/dev/null; then
	find . \( -name "*.cpp" -o -name "*.h" \) \
		-not -path "./cockatrice/src/qt-json/*" \
		-not -path "./servatrice/src/smtp/*" \
		-not -path "./common/sfmt/*" \
		-not -path "./oracle/src/zip/*" \
		-not -path "./build*/*" \
		-exec clang-format -style=file -i {} \;

		if [[ $TRAVIS == false ]]; then
            echo "Repository properly formatted"
		else
			git clean -f
			if git diff --quiet; then
				## exit code = 0 (no differences)
				echo "Code style - OK"   ## 
			else (
				## exit code = 1 (differences)
				echo "*****************************************************";
				echo "***  This PR is not clean against our code style  ***";
				echo "***  Run clang-format and fix up any differences  ***";
				echo "***  Check our CONTRIBUTING.md file for details!  ***";
				echo "***                  Thank you ♥                  ***";
				echo "*****************************************************";
				)
			fi
        fi

else
	echo "Please install clang-format to use this program! Read our CONTRIBUTING.md file for details."			
fi
