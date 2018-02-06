#!/bin/bash

if [[ $TRAVIS == true ]]; then

    if [[ $TRAVIS_OS_NAME == "linux" ]]; then
        if [[ $BUILDTYPE == "Debug" ]]; then
            echo "Checking code style..."
            ./clangify.sh
        else
            echo "Skipping... checks are only performed on runs in debug mode"
        fi
    fi
    
    if [[ $TRAVIS_OS_NAME == "osx" ]]; then
        echo "Skipping... checks are only performed on runs from Linux"
    fi

else
    echo "Doing nothing. This script is supposed to be run by Travis CI only!"
fi
