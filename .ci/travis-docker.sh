#!/bin/bash

# This script is to be sourced in .travis.yaml from the project root directory, do not use it from somewhere else.

project_name="cockatrice"

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
    '--save')
      SAVE=1
      shift
      ;;
    *)
      if [[ $1 == -* ]]; then
        echo "unrecognized option: $1"
        return 3
      fi
      NAME="$1"
      shift
      ;;
  esac
done

# Setup
if ! [[ $NAME ]]; then
  echo "no build name given" >&2
  return 3
fi

export IMAGE_NAME="${project_name,,}_${NAME,,}"

docker_dir=".ci/$NAME"
if ! [[ -r $docker_dir/Dockerfile ]]; then
  echo "could not find dockerfile in $docker_dir" >&2
  return 2 # even if the image is cached, we do not want to run if there is no build file associated with this image
fi

[[ $CACHE ]] || export CACHE="$HOME/$NAME"
if ! [[ -d $CACHE ]]; then
  echo "could not find cache dir: $CACHE" >&2
  unset CACHE
else
  img_dir="$CACHE/image"
  img_save="$img_dir/$IMAGE_NAME.tar.gz"
  if ! [[ -d $img_dir ]]; then
    echo "could not find image dir: $img_dir" >&2
    mkdir -p "$img_dir"
  fi
  export CCACHE_DIR="$CACHE/.ccache"
  if ! [[ -d $CCACHE_DIR ]]; then
    echo "could not find ccache dir: $CCACHE_DIR" >&2
    mkdir -p "$CCACHE_DIR"
  fi
fi


# Get the docker image
if [[ $GET ]]; then
  if [[ $img_save ]] && docker load --input "$img_save"; then
    echo "loaded image"
    docker images
    unset BUILD # do not overwrite the loaded image with build
    unset SAVE # do not overwrite the stored image with the same image
    if [[ $(find "$CCACHE_DIR" -type f -print -quit) ]]; then
      export CACHE="/tmp/cache" # do not overwrite ccache and save it in tmp
      echo "preserving cache to $CACHE"
      mkdir -p "$CACHE"
      cp -rn "$CCACHE_DIR" "$CACHE/.ccache"
    else
      echo "ccache is empty: $(find "$CCACHE_DIR")" >&2
    fi
  else
    echo "could not load cached image, building instead" >&2
    BUILD=1
  fi
fi

# Build the docker image
if [[ $BUILD ]]; then
  if docker build --tag "$IMAGE_NAME" "$docker_dir"; then
    echo "built image"
    docker images
  else
    echo "could not build image $IMAGE_NAME" >&2
  fi
fi

# Save to cache
if [[ $SAVE ]]; then
  if [[ $img_save ]] && docker save --output "$img_save" "$IMAGE_NAME"; then
    echo "saved image to: $img_save"
  else
    echo "could not save image $IMAGE_NAME" >&2
  fi
fi

# Set compile function
function RUN ()
{
  echo "running image:"
  if docker images | grep "$IMAGE_NAME"; then
    args="--mount type=bind,source=$(pwd),target=/src -w=/src"
    if [[ $CCACHE_DIR ]]; then
      args+=" --mount type=bind,source=$CCACHE_DIR,target=/.ccache -e CCACHE_DIR=/.ccache"
    fi
    set -x
    docker run $args $RUN_ARGS "$IMAGE_NAME" bash ".ci/travis-compile.sh" $RUN_OPTS $@
    return $?
  else
    echo "could not find docker image: $IMAGE_NAME" >&2
    return 1
  fi
}
