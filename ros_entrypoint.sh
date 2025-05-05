#!/bin/bash
# Fuente el setup.bash de ROS 2
source /opt/ros/humble/setup.bash

# Fuente el workspace del usuario (si existe)
if [ -f "/home/$USER_NAME/ros2_ws/install/setup.bash" ]; then
    source /home/$USER_NAME/ros2_ws/install/setup.bash
else
    echo "No se encontró el workspace: /home/$USER_NAME/ros2_ws/install/setup.bash"
fi

# Ejecutar el comando pasado o /bin/bash por defecto
exec "${@:-/bin/bash}"
