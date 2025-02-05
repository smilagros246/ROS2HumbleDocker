#!/bin/bash

# Verificación de argumentos
if [ -z "$1" ]; then
    echo "Please, give me a ROS image name."
    exit 1
fi

if [ -z "$2" ]; then
    USER_NAME=$USER  # Se usa el usuario actual si no se proporciona uno
else
    USER_NAME=$2
fi

CONTAINER_LABEL="ros2_$1"
CONTAINER_ALIAS="$1"

# Verificación de glxinfo antes de usarlo
if ! command -v glxinfo &> /dev/null; then
    echo "glxinfo command not found! Execute 'sudo apt install mesa-utils' to install it."
    exit 1
fi

# Directorios de contenedor
CONTAINER_USER_HOME="/home/$USER_NAME"
CONTAINER_WORKDIR="$CONTAINER_USER_HOME/robotics40_ws"
CONTAINER_SCRIPTS="$CONTAINER_WORKDIR/scripts"

# Directorios de host
HOST_USER_HOME="$(pwd)/docker/container/$CONTAINER_ALIAS/home/$USER_NAME"
HOST_WORKDIR="$HOST_USER_HOME/robotics40_ws"
HOST_SCRIPTS="$(pwd)/scripts"

# Crear las carpetas necesarias en el host
mkdir -p "$HOST_USER_HOME"
mkdir -p "$HOST_WORKDIR"

# Activar el uso de GPU si es compatible
VENDOR=$(glxinfo | grep vendor | grep OpenGL | awk '{ print $4 }')

if [ "$VENDOR" == "NVIDIA" ]; then
    USE_GPUS="--gpus all"
    echo "NVIDIA's GPU WAS detected. Activating '--gpus all' flag."
else
    USE_GPUS=""
    echo "NVIDIA's GPU WAS NOT detected."
fi

# Ejecutar el contenedor Docker
docker run -it --rm \
    $USE_GPUS \
    --user "$(id -u):$(id -g)" \
    --env="DISPLAY=$DISPLAY" \
    --env="QT_X11_NO_MITSHM=1" \
    --network="host" \
    --ipc="host" \
    --privileged \
    --oom-kill-disable \
    --volume="$HOST_USER_HOME:$CONTAINER_USER_HOME:rw" \
    --volume="/etc/group:/etc/group:ro" \
    --volume="/etc/passwd:/etc/passwd:ro" \
    --volume="/etc/shadow:/etc/shadow:ro" \
    --volume="/etc/sudoers.d:/etc/sudoers.d:ro" \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    --volume="$HOST_WORKDIR:$CONTAINER_WORKDIR:rw" \
    --volume="$HOST_SCRIPTS:$CONTAINER_SCRIPTS:rw" \
    --name="$CONTAINER_LABEL" \
    --volume="/dev:/dev:rw" \
    --workdir="$CONTAINER_WORKDIR" \
    ros2_foxy_docker:robotics40 \
    bash
