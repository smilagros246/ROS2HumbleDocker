#!/bin/bash

# Verifica que se pase un nombre de contenedor y usuario
if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Uso: $0 <alias_del_contenedor> <nombre_usuario>"
    exit 1
else
    CONTAINER_ALIAS=$1
    USER_NAME=$2
    CONTAINER_HOME=docker/container/$CONTAINER_ALIAS/home/$USER_NAME
    WS_SRC_FOLDER=$CONTAINER_HOME/ros2_ws/src  # <- Ojo cambié a ros2_ws
    SPLITED_ONE=($(echo $1 | tr "-" "\n"))

    # Crea el directorio del workspace
    mkdir -p $WS_SRC_FOLDER

    # Configura el archivo .bashrc para ROS y colcon_cd
    echo "source /opt/ros/humble/setup.bash" >> $CONTAINER_HOME/.bashrc
    echo "source /usr/share/colcon_cd/function/colcon_cd.sh" >> $CONTAINER_HOME/.bashrc
    echo "export _colcon_cd_root=/home/$USER_NAME/ros2_ws" >> $CONTAINER_HOME/.bashrc
    echo "export ROS_DOMAIN_ID=42" >> $CONTAINER_HOME/.bashrc

    # Clonar tu repositorio de robot bombero
    echo "Clonando repositorio robot_bombero_uao..."
    git clone git@github.com:smilagros246/robot_bombero_uao.git $WS_SRC_FOLDER/robot_bombero_uao
    git clone https://github.com/crayzeewulf/libserial.git $CONTAINER_HOME/libserial

    # Construir la imagen Docker con el usuario personalizado
    echo "Construyendo imagen Docker con el alias $CONTAINER_ALIAS y usuario $USER_NAME..."
    docker build --build-arg USER_NAME=$USER_NAME -t ros2_humble_bombero:$CONTAINER_ALIAS .
fi
