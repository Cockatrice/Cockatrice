#!/bin/bash

# This script is to be used in .travis.yaml from the project root directory, do not use it from somewhere else.

set -e

project_name="cockatrice"
compile=".ci/travis-compile.sh"

# Read arguments
while [[ "$@" ]]; do
  case "$1" in
    '--build')
      BUILD=1
      shift
      ;;
    '--get')
      GET=1
      shift
      ;;
    '--run' | '--run-arg')
      RUN=1
      if [[ $1 == --run-arg ]]; then
        RUN_ARGS="$RUN_ARGS $2"
      else
        RUN_OPTS="$RUN_OPTS $2"
      fi
      shift 2
      ;;
    '--save')
      SAVE=1
      shift
      ;;
    '--set-cache')
      CACHE=$2
      if ! [[ -d $CACHE ]]; then
        echo "could not find cache path: $CACHE" >&2
        exit 3
      fi
      shift 2
      ;;
    *)
      if [[ $1 == -* ]]; then
        echo "unrecognized option: $1"
        exit 3
      fi
      NAME="$1"
      shift
      ;;
  esac
done

# Setup
if ! [[ $NAME ]]; then
  echo "no build name given" >&2
  exit 3
fi

docker_dir=".ci/$NAME"
if ! [[ -r $docker_dir/Dockerfile ]]; then
  echo "could not find dockerfile in $docker_dir" >&2
  exit 2 # even if the image is cached, we do not want to run if there is no build file associated with this image
fi

img="${project_name,,}_${NAME,,}"

[[ $CACHE ]] || CACHE="$HOME/$NAME"
if ! [[ -d $CACHE ]]; then
  echo "could not find cache dir: $CACHE" >&2
  unset CACHE
else
  img_dir="$CACHE/image"
  img_save="$img_dir/$img.tar.gz"
  if ! [[ -d $img_dir ]]; then
    echo "could not find image dir: $img_dir" >&2
    mkdir -p "$img_dir"
  fi
  ccache_dir="$CACHE/.ccache"
  if ! [[ -d $ccache_dir ]]; then
    echo "could not find ccache dir: $ccache_dir" >&2
    mkdir -p "$ccache_dir"
  fi
fi


# Get the docker image
if [[ $GET ]]; then
  if [[ $img_save ]] && docker load --input "$img_save"; then
    echo "loaded image"
    docker images
    unset BUILD # do not overwrite the loaded image with build
    unset SAVE # do not overwrite the stored image with the same image
    if [[ $(find "$ccache_dir" -type f -print -quit) ]]; then
      export CACHE="/tmp/cache" # do not overwrite ccache and save it in tmp
      echo "preserving cache to $CACHE"
      mkdir -p "$CACHE"
      cp -rn "$ccache_dir" "$CACHE/.ccache"
    else
      echo "ccache is empty: $(find "$ccache_dir")" >&2
    fi
  else
    echo "could not load cached image, building instead" >&2
    BUILD=1
  fi
fi

# Build the docker image
if [[ $BUILD ]]; then
  docker build --tag "$img" "$docker_dir"
  echo "built image"
  docker images
fi

# Run compilation on the docker image
if [[ $RUN ]]; then
  echo "running image:"
  if docker images | grep "$img"; then
    args="--mount type=bind,source=$(pwd),target=/src -w=/src"
    if [[ $ccache_dir ]]; then
      args+=" --mount type=bind,source=$ccache_dir,target=/.ccache -e CCACHE_DIR=/.ccache"
    fi
    set -x
    docker run $args $RUN_ARGS "$img" bash "$compile" $RUN_OPTS
    set +x
  else
    echo "could not find docker image: $img" >&2
    exit 1
  fi
fi

# Save to cache
if [[ $SAVE ]]; then
  if [[ $img_save ]]; then
    docker save --output "$img_save" "$img"
    echo "saved image to: $img_save"
  else
    echo "could not save image $img" >&2
  fi
fi
