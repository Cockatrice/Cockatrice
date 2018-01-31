#!/bin/bash

if [[ $TRAVIS_OS_NAME == "linux" ]]; then
  if [[ $BUILDTYPE == "Debug" ]]; then #TODO: generalize h cpp files search to top-directory? / check with ctrlaltca
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
    git diff --quiet && echo "Code style - OK" || (
      echo "*****************************************************";
      echo "***  This PR is not clean against our code style  ***";
      echo "***  Run clang-format and fix up any differences  ***";
      echo "***  Check our CONTRIBUTING.md file for details!  ***";
      echo "***                  Thank you ♥                  ***";
      echo "*****************************************************";
    )
    git diff --exit-code
  else
    echo "Skipping... checks are only performed on runs in debug mode"
fi

if [[ $TRAVIS_OS_NAME == "osx" ]]; then
  echo "Skipping... checks are only performed on runs from Linux"
fi
