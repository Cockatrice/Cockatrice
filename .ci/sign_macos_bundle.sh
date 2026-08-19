#!/bin/bash

# This script is to be used by the ci environment.

# Signs and notarizes a macOS app bundle
# Requires: $1 - path to the app bundle
# Environment variables:
# - MACOS_CERTIFICATE_NAME: Name of the certificate for signing (optional, skips signing if not set)
# - MACOS_CI_KEYCHAIN_PWD: Password for the CI keychain (required if MACOS_CERTIFICATE_NAME is set)
# - MACOS_NOTARIZATION_APPLE_ID: Apple ID for notarization (optional, skips notarization if not set)
# - MACOS_NOTARIZATION_PWD: Password for notarization (required if MACOS_NOTARIZATION_APPLE_ID is set)
# - MACOS_NOTARIZATION_TEAM_ID: Team ID for notarization (required if MACOS_NOTARIZATION_APPLE_ID is set)
# exitcode: 1 for failure, 2 for invalid arguments

set -e

# Check input arguments
if [[ $# -lt 1 ]]; then
  echo "::error file=$0::No argument passed to the script - provide <path_to_app_bundle>"
  exit 2
fi

APP_BUNDLE_PATH="$1"

# Verify that app bundle exists
if [[ ! -f "$APP_BUNDLE_PATH" ]]; then
  echo "::error file=$0::App bundle not found at: $APP_BUNDLE_PATH"
  exit 1
fi

# Sign app bundle
if [[ -n "$MACOS_CERTIFICATE_NAME" ]]; then
  echo "::group::Sign app bundle"
  security unlock-keychain -p "$MACOS_CI_KEYCHAIN_PWD" build.keychain
  /usr/bin/codesign --sign="$MACOS_CERTIFICATE_NAME" --entitlements=".ci/macos.entitlements" --options=runtime --force --deep --timestamp --verbose "$APP_BUNDLE_PATH"
  
  # Verify signature
  codesign --verify --deep --strict --verbose=2 "$APP_BUNDLE_PATH"
  spctl -a -v --type install "$APP_BUNDLE_PATH"
  echo "::endgroup::"
else
  echo "::error file=$0::MACOS_CERTIFICATE_NAME not set. Can not sign app bundle."
  exit 1
fi

# Notarize app bundle
if [[ -n "$MACOS_NOTARIZATION_APPLE_ID" ]]; then
  echo "::group::Notarize app bundle"
  # Store the notarization credentials so that we can prevent a UI password dialog from blocking the CI
  xcrun notarytool store-credentials "notarytool-profile" --apple-id "$MACOS_NOTARIZATION_APPLE_ID" --team-id "$MACOS_NOTARIZATION_TEAM_ID" --password "$MACOS_NOTARIZATION_PWD"
  
  # We can't notarize an app bundle directly, but we need to compress it as an archive.
  # Therefore, we create a zip file containing our app bundle, so that we can send it to the notarization service
  echo ""
  echo "Creating temp notarization archive..."
  ditto -c -k --keepParent "$APP_BUNDLE_PATH" "notarization.zip"
  
  # Here we send the notarization request to the Apple's Notarization service, waiting for the result.
  # This typically takes a few seconds inside a CI environment, but it might take more depending on the App characteristics.
  # Visit the Notarization docs for more information and strategies on how to optimize it if you're curious.
  echo ""
  xcrun notarytool submit "notarization.zip" --keychain-profile "notarytool-profile" --wait
  echo "::endgroup::"
  
  echo "::group::Staple app"
  # Finally, we need to "attach the staple" to our executable, which will allow our app to be
  # validated by macOS even when an internet connection is not available.
  echo "Attach staple"
  xcrun stapler staple "$APP_BUNDLE_PATH"
  
  # Validate staple
  xcrun stapler validate "$APP_BUNDLE_PATH"
  echo "::endgroup::"
else
  echo "::error file=$0::MACOS_NOTARIZATION_APPLE_ID not set. Can not notarize app bundle."
  exit 1
fi

echo "::group::Cleanup"
# Cleanup keychain and files to avoid leaking credentials
echo "Deleting keychain"
security delete-keychain build.keychain
rm -f certificate.p12 notarization.zip
echo "::endgroup::"
