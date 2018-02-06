#!/bin/bash

if [[ $TRAVIS == true ]]; then

  if [[ $TRAVIS_OS_NAME == "linux" ]] ; then
    echo "Skipping... Linux packages are installed with the apt addon for sudo disabled container builds as configuered in travis.yml"
  fi
  if [[ $TRAVIS_OS_NAME == "osx" ]] ; then
    brew update
    brew install ccache protobuf qt@5.5
  fi

else
  echo "Doing nothing. This script is supposed to be run by Travis CI only!"
fi
