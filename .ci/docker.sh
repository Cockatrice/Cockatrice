#!/bin/bash

# This script is to be used by the ci environment from the project root directory, do not use it from somewhere else.

# Creates or loads docker images to use in compilation, creates RUN function to start compilation on the docker image.
# <arg> sets the name of the docker image, these correspond to directories in .ci
# --get loads the image from a previously saved image cache, will build if no image is found
# --build builds the image from the Dockerfile in .ci/$NAME
# --save stores the image, if an image was loaded it will not be stored
# --interactive immediately starts the image interactively for debugging
# --set-cache <location> sets the location to cache the image or for ccache
# requires: docker
# uses env: NAME CACHE BUILD GET SAVE INTERACTIVE
# (correspond to args: <name> --set-cache <cache> --build --get --save --interactive)
# sets env: RUN CCACHE_DIR IMAGE_NAME RUN_ARGS RUN_OPTS BUILD_SCRIPT
# exitcode: 1 for failure, 2 for missing dockerfile, 3 for invalid arguments
export BUILD_SCRIPT=".ci/compile.sh"

project_name="cockatrice"
save_extension=".tar.gz"
image_cache="image"
ccache_cache=".ccache"

# Read arguments
while [[ $# != 0 ]]; do
  case "$1" in
    '--build')
      BUILD=1
      shift
      ;;
    '--get')
      GET=1
      shift
      ;;
    '--interactive')
      INTERACTIVE=1
      shift
      ;;
    '--save')
      SAVE=1
      shift
      ;;
    '--set-cache')
      CACHE=$2
      if ! [[ -d $CACHE ]]; then
        echo "could not find cache path: $CACHE" >&2
        return 3
      fi
      shift 2
      ;;
    *)
      if [[ ${1:0:1} == - ]]; then
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

export IMAGE_NAME="${project_name,,}_${NAME,,}" # lower case

docker_dir=".ci/$NAME"
if ! [[ -r $docker_dir/Dockerfile ]]; then
  echo "could not find Dockerfile in $docker_dir" >&2
  return 2 # even if the image is cached, we do not want to run if there is no way to build this image
fi

if ! [[ $CACHE ]]; then
  echo "cache dir is not set!" >&2
  CACHE="$(mktemp -d)"
  echo "set cache dir to $CACHE" >&2
fi
if ! [[ -d $CACHE ]]; then
  echo "could not find cache dir: $CACHE" >&2
  mkdir -p "$CACHE"
  unset GET # the dir is empty
fi
if [[ $GET || $SAVE ]]; then
  img_dir="$CACHE/$image_cache"
  img_save="$img_dir/$IMAGE_NAME$save_extension"
  if ! [[ -d $img_dir ]]; then
    echo "could not find image dir: $img_dir" >&2
    mkdir -p "$img_dir"
  fi
fi
export CCACHE_DIR="$CACHE/$ccache_cache"
if ! [[ -d $CCACHE_DIR ]]; then
  echo "could not find ccache dir: $CCACHE_DIR" >&2
  mkdir -p "$CCACHE_DIR"
fi

# Get the docker image from previously stored save
if [[ $GET ]]; then
  if [[ $img_save ]] && docker load --input "$img_save"; then
    echo "loaded image"
    docker images
    unset BUILD # do not overwrite the loaded image with build
    unset SAVE # do not overwrite the stored image with the same image
    if [[ $(find "$CCACHE_DIR" -type f -print -quit) ]]; then # check contents of ccache
      echo "setting ccache to readonly"
      export RUN_ARGS="$RUN_ARGS -e CCACHE_READONLY=1 -e CCACHE_NOSTATS=1" # do not overwrite ccache
    else
      echo "ccache is empty: $(find "$CCACHE_DIR")" >&2
    fi
  else
    echo "could not load cached image, building instead" >&2
    BUILD=1
  fi
fi

# Build the docker image from dockerfile
if [[ $BUILD ]]; then
  if docker build --tag "$IMAGE_NAME" "$docker_dir"; then
    echo "built image"
    docker images
  else
    echo "could not build image $IMAGE_NAME" >&2
    return 1
  fi
fi

# Save docker image to cache (compressed)
if [[ $SAVE ]]; then
  if [[ $img_save ]] && docker save --output "$img_save" "$IMAGE_NAME"; then
    echo "saved image to: $img_save"
  else
    echo "could not save image $IMAGE_NAME" >&2
  fi
fi

# Set compile function, runs the compile script on the image, passes arguments to the script
function RUN ()
{
  echo "running image:"
  if [[ $(docker images) =~ "$IMAGE_NAME" ]]; then
    local args=(--mount "type=bind,source=$PWD,target=/src")
    args+=(--workdir "/src")
    args+=(--user "$(id -u):$(id -g)")
    if [[ $CCACHE_DIR ]]; then
      args+=(--mount "type=bind,source=$CCACHE_DIR,target=/.ccache")
      args+=(--env "CCACHE_DIR=/.ccache")
      args+=(--env "CMAKE_GENERATOR="Ninja"")
    fi
    docker run "${args[@]}" $RUN_ARGS "$IMAGE_NAME" bash "$BUILD_SCRIPT" $RUN_OPTS "$@"
    return $?
  else
    echo "could not find docker image: $IMAGE_NAME" >&2
    return 3
  fi
}

# for debugging, start the docker image interactively instead of building
# starts immediately, does not require sourcing or RUN
if [[ $INTERACTIVE ]]; then
  export BUILD_SCRIPT="-i"
  export RUN_ARGS="$RUN_ARGS -it"
  RUN
fi
