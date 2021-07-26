#!/bin/bash

# fetch master branch
git fetch origin master

# unshallow if needed
echo "Finding merge base"
if ! git merge-base origin/master HEAD; then
  echo "Could not find merge base, unshallowing repo"
  git fetch --unshallow
fi

# Check formatting using clangify
echo "Checking your code using clang-format..."

diff="$(./clangify.sh --diff --cf-version --branch origin/master)"
err=$?

case $err in
  1)
    cat <<EOM

***********************************************************
***                                                     ***
***   Your code does not comply with our style guide.   ***
***                                                     ***
***  Please correct it or run the "clangify.sh" script. ***
***  Then commit and push those changes to this branch. ***
***   Check our CONTRIBUTING.md file for more details.  ***
***                                                     ***
***                    Thank you ❤️                      ***
***                                                     ***
***********************************************************

Used clang-format version:
${diff%%
*}

The following changes should be made:
${diff#*
}

Exiting...
EOM
      exit 2
      ;;

    0)
      cat <<EOM

***********************************************************
***                                                     ***
***       Your code complies with our style guide!      ***
***                                                     ***
***                      Awesome 👍                     ***
***                                                     ***
***********************************************************

Exiting...
EOM
      exit 0
      ;;

    *)
      echo ""
      echo "Something went wrong in our formatting checks: clangify returned $err" >&2
      echo ""
      ;;
  esac
