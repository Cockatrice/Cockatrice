#!/bin/bash

if [[ $TRAVIS_OS_NAME == "linux" ]]; then
  if [[ $BUILDTYPE == "Debug" ]]; then
    ## Check code style
	../clangify.sh
  else
    echo "Skipping... checks are only performed on runs in debug mode"
  fi
fi

if [[ $TRAVIS_OS_NAME == "osx" ]]; then
  echo "Skipping... checks are only performed on runs from Linux"
fi
