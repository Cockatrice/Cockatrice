#!/bin/bash

if [[ $TRAVIS_OS_NAME == "linux" ]]; then
  echo "Checking code style..."
  cd ..
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
  git diff --quiet || (
    echo "*****************************************************";
    echo "***  This PR is not clean against our code style  ***";
    echo "***  Run clang-format and fix up any differences  ***";
    echo "***  Check our CONTRIBUTING.md file for details!  ***";
    echo "***                  Thank you ♥                  ***";
    echo "*****************************************************";
  )
  git diff --exit-code
fi

#TODO: message on success