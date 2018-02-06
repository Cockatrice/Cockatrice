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
		# if run from Travis CI, we want to compare the changes and report back to the contributor
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
			git diff --exit-code ## correct position? what is this used for? original script: https://github.com/Cockatrice/Cockatrice/blob/994a643d9c4e08dad1dfd7b13272598fdb42b196/.ci/travis-compile.sh
        fi

else
	echo "Please install clang-format to use this program! Read our CONTRIBUTING.md file for details."			
fi
