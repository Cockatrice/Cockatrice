#!/bin/bash

# Check formatting using clang-format
echo "Checking your code using clang-format..."
diff="$(./clangify.sh --color-diff --cf-version)"
err=$?
case $err in
  1)
    cat <<EOM
***********************************************************
***                                                     ***
***    Your code does not comply with our styleguide.   ***
***                                                     ***
***  Please correct it or run the "clangify.sh" script. ***
***  Then commit and push those changes to this branch. ***
***   Check our CONTRIBUTING.md file for more details.  ***
***                                                     ***
***                     Thank you ♥                     ***
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
      echo "Thank you for complying with our code standards."
      ;;
    *)
      echo "Something went wrong in our formatting checks: clangify returned $err" >&2
      ;;
  esac
