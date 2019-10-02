#!/usr/bin/env sh

CONTAINER_NAME=cockatrice
VERSION=latest

function runDocker {
    if [ $# -gt 2 ]; then
        echo "Use: .run.sh <<CONTAINER_NAME>> <<COCKATRICE_VERSION>>"
        exit 1
    fi

    if [ $# -gt 0 ]; then
        CONTAINER_NAME=$1
    fi
    if [ $# -gt 1 ]; then
        VERSION=$2
    fi

    docker run --privileged --rm -e DISPLAY=$DISPLAY --name ${CONTAINER_NAME} \
           -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
           -v /home/$USER/.local/share/Cockatrice/Cockatrice:/root/.local/share/Cockatrice/Cockatrice \
           cockatrice/cockatrice:${VERSION}
}

(
    if ( flock -ne 1000 ); then
        trap "xhost -local:docker" EXIT
        echo "Starting Cockatrice..."
        xhost +local:docker
        runDocker $@
    else
        echo "Already there is a process running!"
    fi
) 1000>.cockatrice
