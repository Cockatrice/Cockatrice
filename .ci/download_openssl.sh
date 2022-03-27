#!/bin/bash

# Read arguments
while [[ $# != 0 ]]; do
  case "$1" in
    '--')
      shift
      ;;
    '--arch')
      shift
      if [[ $# == 0 ]]; then
        echo "::error file=$0::--arch expects an argument"
        exit 3
      fi
      OS_ARCH="$1"
      shift
      ;;
    *)
      echo "::error file=$0::unrecognized option: $1"
      exit 3
      ;;
  esac
done

set -e

OPEN_SSL_VERSION="1.1.1n"
DEST_PATH="C:\OpenSSL-Win$OS_ARCH"

curl -JLSs "https://github.com/CristiFati/Prebuilt-Binaries/raw/master/OpenSSL/v1.1.1/OpenSSL-$OPEN_SSL_VERSION-Win-pc0$OS_ARCH.zip" -o OpenSSL.zip
unzip -q "OpenSSL.zip"
rm "OpenSSL.zip"
mv "OpenSSL\OpenSSL\\$OPEN_SSL_VERSION" "$DEST_PATH"
rm -r "OpenSSL"
echo "Installed OpenSSL v$OPEN_SSL_VERSION to $DEST_PATH"