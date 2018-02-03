#!/bin/bash

if [[ $TRAVIS_OS_NAME == "linux" ]]; then
  if [[ $BUILDTYPE == "Debug" ]]; then   ## TODO: generalize h cpp files search to top-directory? other comments, 3 nested if's the right approach? / check with ctrlaltca
    echo "Checking code style..."
    clang-format -i \
      common/*.h \
      common/*.cpp \
      cockatrice/src/*.h \
      cockatrice/src/*.cpp \
      oracle/src/*.h \
      oracle/src/*.cpp \
      servatrice/src/*.h \
      servatrice/src/*.cpp
      
    git clean -f
    git diff --quiet
	if [[  == "" ]]; then   ## missing condition, should check result of "--exit-code" from "diff --quiet" (0 or 1)
	  echo "Code style - OK"
	else (   ## brackets correct?
      echo "*****************************************************";
      echo "***  This PR is not clean against our code style  ***";
      echo "***  Run clang-format and fix up any differences  ***";
      echo "***  Check our CONTRIBUTING.md file for details!  ***";
      echo "***                  Thank you ♥                  ***";
      echo "*****************************************************";
      )
    fi
    git diff --exit-code   ## correct position?
  else
    echo "Skipping... checks are only performed on runs in debug mode"
  fi
fi

if [[ $TRAVIS_OS_NAME == "osx" ]]; then
  echo "Skipping... checks are only performed on runs from Linux"
fi
