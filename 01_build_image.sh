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
    echo "source /opt/ros/jazzy/setup.bash" >> $CONTAINER_HOME/.bashrc
    echo "source /usr/share/colcon_cd/function/colcon_cd.sh" >> $CONTAINER_HOME/.bashrc
    echo "export _colcon_cd_root=/home/$USER_NAME/ros2_ws" >> $CONTAINER_HOME/.bashrc
    echo "export ROS_DOMAIN_ID=42" >> $CONTAINER_HOME/.bashrc

    # Clonar tu repositorio de robot bombero
    echo "Clonando repositorio demos de ros2_control..."
    git clone https://github.com/ros-controls/ros2_control_demos -b jazzy $WS_SRC_FOLDER/ros2_control_demos

    # Construir la imagen Docker con el usuario personalizado
    echo "Construyendo imagen Docker con el alias $CONTAINER_ALIAS y usuario $USER_NAME..."
    docker build --build-arg USER_NAME=$USER_NAME -t ros2_jazzy_bombero:$CONTAINER_ALIAS .
fi
