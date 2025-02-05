#!/bin/bash

# Verifica que se pase un nombre de contenedor y usuario
if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Uso: $0 <alias_del_contenedor> <nombre_usuario>"
    exit 1
else
    CONTAINER_ALIAS=$1
    USER_NAME=$2
    CONTAINER_HOME=docker/container/$CONTAINER_ALIAS/home/$USER_NAME
    WS_SRC_FOLDER=$CONTAINER_HOME/robotics40_ws/src
    SPLITED_ONE=($(echo $1 | tr "-" "\n"))

    # Crea el directorio del workspace
    mkdir -p $WS_SRC_FOLDER

    # Configura el archivo .bashrc para ROS y colcon_cd
    echo "source /opt/ros/foxy/setup.bash" >> $CONTAINER_HOME/.bashrc
    echo "source /usr/share/colcon_cd/function/colcon_cd.sh" >> $CONTAINER_HOME/.bashrc
    echo "export _colcon_cd_root=/home/$USER_NAME/robotics40_ws" >> $CONTAINER_HOME/.bashrc

    # Clonaciones adicionales
    git clone git@github.com:dockerobotics40/unitree_sdk2_python.git $CONTAINER_HOME/unitree_sdk2_python
    git clone git@github.com:dockerobotics40/unitree_sdk2.git $CONTAINER_HOME/unitree_sdk2
    git clone git@github.com:dockerobotics40/unitree_ros2.git $CONTAINER_HOME/unitree_ros2
    git clone git@github.com:dockerobotics40/prueba_uno.git $CONTAINER_HOME/prueba_uno
    git clone https://github.com/ros2/rmw_cyclonedds -b foxy $CONTAINER_HOME/unitree_ros2/cyclonedds_ws/src/rmw_cyclonedds
    git clone https://github.com/eclipse-cyclonedds/cyclonedds -b releases/0.10.x $CONTAINER_HOME/unitree_ros2/cyclonedds_ws/src/cyclonedds

    # Construir la imagen Docker con el usuario personalizado
    echo "Construyendo imagen Docker con el alias $CONTAINER_ALIAS y usuario $USER_NAME..."
    docker build --build-arg USER_NAME=$USER_NAME -t ros2_foxy_docker:$CONTAINER_ALIAS .
fi
