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

# Directorios de contenedor
CONTAINER_USER_HOME="/home/$USER_NAME"
CONTAINER_WORKDIR="$CONTAINER_USER_HOME/ros2_ws"   # ← CAMBIADO
CONTAINER_SCRIPTS="$CONTAINER_WORKDIR/scripts"

# Directorios de host
HOST_USER_HOME="$(pwd)/docker/container/$CONTAINER_ALIAS/home/$USER_NAME"
HOST_WORKDIR="$HOST_USER_HOME/ros2_ws"   # ← CAMBIADO
HOST_SCRIPTS="$(pwd)/scripts"

# Crear las carpetas necesarias en el host
mkdir -p "$HOST_USER_HOME"
mkdir -p "$HOST_WORKDIR"

#CAMBIAR EN LA RASP el device
# Ejecutar el contenedor Docker
docker run -it --rm \
    --entrypoint "" \
    --user "$(id -u):$(id -g)" \
    --device=/dev/ttyUSB0 \
    --group-add dialout \
    --env="DISPLAY=$DISPLAY" \
    --env="QT_X11_NO_MITSHM=1" \
    --network="host" \
    --ipc="host" \
    --oom-kill-disable \
    --privileged \
    --volume="$HOST_USER_HOME:$CONTAINER_USER_HOME:rw" \
    --volume="/etc/group:/etc/group:ro" \
    --volume="/etc/passwd:/etc/passwd:ro" \
    --volume="/etc/shadow:/etc/shadow:ro" \
    --volume="/etc/sudoers.d:/etc/sudoers.d:ro" \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    --volume="$HOME/.ssh:/home/$USER_NAME/.ssh:ro" \
    --volume="$HOST_WORKDIR:$CONTAINER_WORKDIR:rw" \
    --volume="$HOST_SCRIPTS:$CONTAINER_SCRIPTS:rw" \
    --name="$CONTAINER_LABEL" \
    --volume="/dev:/dev:rw" \
    --workdir="$CONTAINER_WORKDIR" \
    ros2_humble_bombero:$CONTAINER_ALIAS \
    bash
