#!/bin/bash

# Fetch master branch
git fetch origin master

# Unshallow if needed
echo "Finding merge base"
if ! git merge-base origin/master HEAD; then
  echo "Could not find merge base, unshallowing repo"
  git fetch --unshallow
fi

# Check formatting using format.sh
echo "Checking your code using format.sh..."

./format.sh --color-diff --cmake --shell --print-version --branch origin/master
err=$?

case $err in
  1)
    cat <<EOM

***********************************************************
***                                                     ***
***   Your code does not comply with our style guide.   ***
***                                                     ***
***   Please correct it or run the "format.sh" script.  ***
***  Then commit and push those changes to this branch. ***
***   Check our CONTRIBUTING.md file for more details.  ***
***                                                     ***
***                    Thank you ❤️                     ***
***                                                     ***
***********************************************************

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

EOM
    exit 0
    ;;

  *)
    echo "Something went wrong in our formatting checks: format.sh returned $err" >&2
    ;;
esac
