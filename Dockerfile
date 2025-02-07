# Usar la imagen base de ROS 2 Foxy
FROM osrf/ros:foxy-desktop

# Configurar la localización y la variable de entorno para Python
ENV LANG C.UTF-8
ENV PYTHONUNBUFFERED 1

# Definir argumentos para usuario dinámico (usando el nombre de usuario del host)
ARG USER_NAME=${USER}

# Instalar herramientas adicionales necesarias
RUN apt-get update && apt-get upgrade -y \
    && apt-get install -y apt-utils mesa-utils iputils-ping apt-transport-https ca-certificates \
    && apt-get install -y openssh-server python3-pip exuberant-ctags \
    && apt-get install -y git vim tmux nano htop sudo curl wget gnupg2 \
    && apt-get install -y bash-completion python3-psycopg2 \
    && pip3 install powerline-shell \
    && rm -rf /var/lib/apt/lists/*

# Crear usuario dinámico con permisos sudo
RUN useradd -ms /bin/bash $USER_NAME \
    && echo "$USER_NAME:$USER_NAME" | chpasswd \
    && adduser $USER_NAME sudo \
    && echo "$USER_NAME ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Definir variables de entorno
ENV CONTAINER_USER_HOME=/home/$USER_NAME
ENV CONTAINER_WORKDIR=$CONTAINER_USER_HOME/robotics40_ws
ENV CONTAINER_SCRIPTS=$CONTAINER_WORKDIR/scripts

# Configuración de Python en ROS
RUN ln -s /usr/bin/python3 /usr/bin/python

# Crear carpetas necesarias con permisos correctos
RUN mkdir -p $CONTAINER_USER_HOME/.ssh && chmod 700 $CONTAINER_USER_HOME/.ssh \
    && mkdir -p $CONTAINER_USER_HOME/.vim/bundle \
    && chown -R $USER_NAME:$USER_NAME $CONTAINER_USER_HOME

# Copiar y configurar Vundle (gestor de plugins de Vim)
RUN git clone https://github.com/VundleVim/Vundle.vim.git $CONTAINER_USER_HOME/.vim/bundle/Vundle.vim
COPY --chown=$USER_NAME:$USER_NAME ./.vimrc $CONTAINER_USER_HOME/.vimrc

# Configurar ROS y CycloneDDS
RUN apt-get update \
    && apt-get install -y ros-foxy-rmw-cyclonedds-cpp ros-foxy-rosidl-generator-dds-idl \
    && apt-get install -y build-essential cmake pkg-config libssl-dev

# Configuración de ROS en el .bashrc
RUN echo 'source /opt/ros/foxy/setup.bash' >> /home/$USER_NAME/.bashrc \
    && echo "source /usr/share/colcon_cd/function/colcon_cd.sh" >> /home/$USER_NAME/.bashrc \
    && echo "export _colcon_cd_root=/home/$USER_NAME/robotics40_ws" >> /home/$USER_NAME/.bashrc

# Cambiar al usuario configurado dinámicamente
USER $USER_NAME

# Definir el directorio de trabajo
WORKDIR $CONTAINER_USER_HOME

# Configuración de SSH
# Copiar las claves SSH al contenedor
# Copiar las claves SSH al contenedor
COPY --chown=$USER_NAME:$USER_NAME .ssh/id_rsa /home/$USER_NAME/.ssh/id_rsa
COPY --chown=$USER_NAME:$USER_NAME .ssh/id_rsa.pub /home/$USER_NAME/.ssh/id_rsa.pub


# Configurar permisos para las claves SSH
RUN chmod 600 /home/$USER_NAME/.ssh/id_rsa \
    && chmod 644 /home/$USER_NAME/.ssh/id_rsa.pub \
    && chown -R $USER_NAME:$USER_NAME /home/$USER_NAME/.ssh

# Añadir la clave pública de GitHub a known_hosts para evitar problemas de verificación
RUN ssh-keyscan github.com >> /home/$USER_NAME/.ssh/known_hosts

# Asegúrate de que el repositorio se haya clonado correctamente usando SSH
RUN git clone git@github.com:dockerobotics40/unitree_sdk2.git /home/$USER_NAME/unitree_sdk2 && ls -l /home/$USER_NAME/unitree_sdk2
RUN cmake --version && make --version

# Usar la imagen base de ROS 2 Foxy
FROM osrf/ros:foxy-desktop

# Configurar la localización y la variable de entorno para Python
ENV LANG C.UTF-8
ENV PYTHONUNBUFFERED 1

# Definir argumentos para usuario dinámico (usando el nombre de usuario del host)
ARG USER_NAME=${USER}

# Instalar herramientas adicionales necesarias
RUN apt-get update && apt-get upgrade -y \
    && apt-get install -y apt-utils mesa-utils iputils-ping apt-transport-https ca-certificates \
    && apt-get install -y openssh-server python3-pip exuberant-ctags \
    && apt-get install -y git vim tmux nano htop sudo curl wget gnupg2 \
    && apt-get install -y bash-completion python3-psycopg2 \
    && apt-get install -y ros-foxy-cv-bridge python3-opencv \
    && apt-get install ros-foxt-plotjuggler-ros \
    && pip3 install numpy powerline-shell imageio[ffmpeg] \
    && rm -rf /var/lib/apt/lists/*

# Crear usuario dinámico con permisos sudo
RUN useradd -ms /bin/bash $USER_NAME \
    && echo "$USER_NAME:$USER_NAME" | chpasswd \
    && adduser $USER_NAME sudo \
    && echo "$USER_NAME ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Definir variables de entorno
ENV CONTAINER_USER_HOME=/home/$USER_NAME
ENV CONTAINER_WORKDIR=$CONTAINER_USER_HOME/robotics40_ws
ENV CONTAINER_SCRIPTS=$CONTAINER_WORKDIR/scripts

# Configuración de Python en ROS
RUN ln -s /usr/bin/python3 /usr/bin/python

# Crear carpetas necesarias con permisos correctos
RUN mkdir -p $CONTAINER_USER_HOME/.ssh && chmod 700 $CONTAINER_USER_HOME/.ssh \
    && mkdir -p $CONTAINER_USER_HOME/.vim/bundle \
    && chown -R $USER_NAME:$USER_NAME $CONTAINER_USER_HOME

# Copiar y configurar Vundle (gestor de plugins de Vim)
RUN git clone https://github.com/VundleVim/Vundle.vim.git $CONTAINER_USER_HOME/.vim/bundle/Vundle.vim
COPY --chown=$USER_NAME:$USER_NAME ./.vimrc $CONTAINER_USER_HOME/.vimrc

# Configurar ROS y CycloneDDS
RUN apt-get update \
    && apt-get install -y ros-foxy-rmw-cyclonedds-cpp ros-foxy-rosidl-generator-dds-idl \
    && apt-get install -y build-essential cmake pkg-config libssl-dev

# Configuración de ROS en el .bashrc
RUN echo 'source /opt/ros/foxy/setup.bash' >> /home/$USER_NAME/.bashrc \
    && echo "source /usr/share/colcon_cd/function/colcon_cd.sh" >> /home/$USER_NAME/.bashrc \
    && echo "export _colcon_cd_root=/home/$USER_NAME/robotics40_ws" >> /home/$USER_NAME/.bashrc

# Cambiar al usuario configurado dinámicamente
USER $USER_NAME

# Definir el directorio de trabajo
WORKDIR $CONTAINER_USER_HOME

# Configuración de SSH
# Copiar las claves SSH al contenedor
# Copiar las claves SSH al contenedor
COPY --chown=$USER_NAME:$USER_NAME .ssh/id_rsa /home/$USER_NAME/.ssh/id_rsa
COPY --chown=$USER_NAME:$USER_NAME .ssh/id_rsa.pub /home/$USER_NAME/.ssh/id_rsa.pub


# Configurar permisos para las claves SSH
RUN chmod 600 /home/$USER_NAME/.ssh/id_rsa \
    && chmod 644 /home/$USER_NAME/.ssh/id_rsa.pub \
    && chown -R $USER_NAME:$USER_NAME /home/$USER_NAME/.ssh

# Añadir la clave pública de GitHub a known_hosts para evitar problemas de verificación
RUN ssh-keyscan github.com >> /home/$USER_NAME/.ssh/known_hosts

# Asegúrate de que el repositorio se haya clonado correctamente usando SSH
RUN git clone git@github.com:dockerobotics40/unitree_sdk2.git /home/$USER_NAME/unitree_sdk2 && ls -l /home/$USER_NAME/unitree_sdk2
RUN cmake --version && make --version

# Verifica que la ruta sea correcta antes de intentar crear el directorio build
RUN mkdir -p /home/$USER_NAME/unitree_sdk2/build \
    && cd /home/$USER_NAME/unitree_sdk2/build \
    && cmake .. \
    && make \
    && sudo make install

RUN ls -l /home/$USER_NAME/unitree_sdk2
RUN ls -l /usr/local/include/unitree
RUN ls -l /usr/local/lib    


     
    

# Iniciar SSH y mantener el contenedor en ejecución
CMD sudo service ssh start && /bin/bash
