#!/bin/bash

# Used by CI to sign, noratize and staple build artifacts under macOS

set -e

APP_PATH="$1"

if [[ -z "$APP_PATH" ]]; then
  echo "Error: No input path supplied. Usage: $0 <path-to-app-bundle>"
  exit 1
fi

if [[ ! -e "$APP_PATH" ]]; then
  echo "Error: Input path '$APP_PATH' does not exist."
  exit 1
fi

echo "::group::Set up certificate"
if [[ -n "$MACOS_CERTIFICATE_NAME" ]]; then
  echo $MACOS_CERTIFICATE | base64 --decode > certificate.p12
  security create-keychain -p "$MACOS_CI_KEYCHAIN_PWD" build.keychain
  security default-keychain -s build.keychain
  security set-keychain-settings -t 3600 -l build.keychain
  security unlock-keychain -p "$MACOS_CI_KEYCHAIN_PWD" build.keychain
  security import certificate.p12 -k build.keychain -P "$MACOS_CERTIFICATE_PWD" -T /usr/bin/codesign
  security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$MACOS_CI_KEYCHAIN_PWD" build.keychain
else
  echo "No signing certificate configured."
fi
echo "::endgroup::"

echo "::group::Sign app"
if [[ -n "$MACOS_CERTIFICATE_NAME" ]]; then
  security unlock-keychain -p "$MACOS_CI_KEYCHAIN_PWD" build.keychain
  /usr/bin/codesign --sign="$MACOS_CERTIFICATE_NAME" --entitlements=".ci/macos.entitlements" --options=runtime --force --deep --timestamp --verbose=3 "$APP_PATH"
else
  echo "No signing certificate configured."
fi
echo "::endgroup::"

echo "::group::Notarize app"
if [[ -n "$MACOS_NOTARIZATION_APPLE_ID" ]]; then
  # Store the notarization credentials so that we can prevent a UI password dialog from blocking the CI
  xcrun notarytool store-credentials "notarytool-profile" --apple-id "$MACOS_NOTARIZATION_APPLE_ID" --team-id "$MACOS_NOTARIZATION_TEAM_ID" --password "$MACOS_NOTARIZATION_PWD"
  echo ""

  # We can't notarize an app bundle directly, but we need to compress it as an archive.
  # Therefore, we create a zip file containing our app bundle, so that we can send it to the
  # notarization service
  echo "Creating temp notarization archive"
  ditto -c -k --keepParent "$APP_PATH" "notarization.zip"
  echo ""

  # Here we send the notarization request to the Apple's Notarization service, waiting for the result.
  # This typically takes a few seconds inside a CI environment, but it might take more depending on the App
  # characteristics. Visit the Notarization docs for more information and strategies on how to optimize it if
  # you're curious
  # Submit for notarization and capture output
  NOTARY_OUTPUT=$(xcrun notarytool submit "notarization.zip" --keychain-profile "notarytool-profile" --wait --output-format json)
  
  # Parse the submission ID from the JSON output
  SUBMISSION_ID=$(echo "$NOTARY_OUTPUT" | jq -r '.id')
  
  # Parse the status field in the JSON
  STATUS=$(echo "$NOTARY_OUTPUT" | jq -r '.status')
  
  if [[ "$STATUS" != "Accepted" ]]; then
    echo "Notarization failed (status: $STATUS). Fetching detailed log..."
    # Fetch and print the notarization log
    xcrun notarytool log "$SUBMISSION_ID" --keychain-profile "notarytool-profile"
  exit 1
  fi

else
  echo "No Apple ID configured."
fi
echo "::endgroup::"

echo "::group::Staple app"
if [[ -n "$MACOS_NOTARIZATION_APPLE_ID" ]]; then
  # Finally, we need to "attach the staple" to our executable, which will allow our app to be
  # validated by macOS even when an internet connection is not available.
  xcrun stapler staple "$APP_PATH"
else
  echo "No Apple ID configured."
fi
echo "::endgroup::"

echo "::group::Cleanup"
# Cleanup keychain and files to avoid leaking credentials
echo "Deleting keychain"
security delete-keychain build.keychain
rm -f certificate.p12 notarization.zip
echo "::endgroup::"
