# Imagen base ROS 2 jazzy Desktop
FROM osrf/ros:jazzy-desktop-full

# Configurar la localización y Python
ENV LANG C.UTF-8
ENV PYTHONUNBUFFERED 1

# Definir argumentos para el usuario dinámico
ARG USER_NAME=${USER}

# Elegir la versión de Gazebo: fortress, garden, harmonic
ARG GZ_VERSION=harmonic

# Instalar herramientas básicas y dependencias
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
    apt-utils \
    mesa-utils \
    iputils-ping \
    apt-transport-https \
    ca-certificates \
    lsb-release \
    wget \
    gnupg2 \
    sudo \
    curl \
    git \
    tree \
    vim \
    tmux \
    nano \
    htop \
    bash-completion \
    build-essential \
    cmake \
    python3-pip \
    python3-opencv \
    python3-smbus \
    i2c-tools \
    ros-jazzy-rmw-cyclonedds-cpp \
    ros-jazzy-rosidl-generator-dds-idl \
    ros-jazzy-joint-state-publisher-gui \
    ros-jazzy-joy \
    ros-jazzy-teleop-twist-joy \
    ros-jazzy-ros2-control \
    ros-jazzy-realtime-tools \
    ros-jazzy-xacro \
    ros-jazzy-ros2-controllers \
    ros-jazzy-rviz2 \
    ros-jazzy-cv-bridge \
    ros-jazzy-gazebo-ros \
    ros-jazzy-gazebo-ros2-control \
    openssh-server \
 && pip3 install numpy powerline-shell imageio[ffmpeg] \
 && rosdep init || true


# Instalar dependencias de libserial
RUN apt-get update && apt-get install -y \
    g++ git autogen autoconf build-essential cmake graphviz \
    libboost-dev libboost-test-dev libgtest-dev libtool \
    python3-sip-dev doxygen python3-sphinx pkg-config \
    python3-sphinx-rtd-theme

# Limpiar apt cache
RUN rm -rf /var/lib/apt/lists/*

# Crear usuario dinámico con permisos sudo y acceso a dialout
RUN useradd -ms /bin/bash $USER_NAME \
    && echo "$USER_NAME:$USER_NAME" | chpasswd \
    && adduser $USER_NAME sudo \
    && usermod -aG dialout $USER_NAME \
    && echo "$USER_NAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Definir variables de entorno
ENV CONTAINER_USER_HOME=/home/$USER_NAME
ENV CONTAINER_WORKDIR=$CONTAINER_USER_HOME/ros2_ws
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

# Configurar ROS 2 y CycloneDDS para jazzy
RUN apt-get update \
    && apt-get install -y ros-jazzy-rmw-cyclonedds-cpp ros-jazzy-rosidl-generator-dds-idl \
    && apt-get install -y build-essential cmake pkg-config libssl-dev

# Configuración de ROS en el .bashrc
RUN echo 'source /opt/ros/jazzy/setup.bash' >> /home/$USER_NAME/.bashrc \
    && echo "source /usr/share/colcon_cd/function/colcon_cd.sh" >> /home/$USER_NAME/.bashrc \
    && echo "export _colcon_cd_root=/home/$USER_NAME/ros2_ws" >> /home/$USER_NAME/.bashrc

# Cambiar al usuario configurado dinámicamente
USER $USER_NAME

# Definir el directorio de trabajo
WORKDIR $CONTAINER_USER_HOME


# Configuración de SSH
# Copiar las claves SSH al contenedor
COPY --chown=$USER_NAME:$USER_NAME .ssh/id_rsa /home/$USER_NAME/.ssh/id_rsa
COPY --chown=$USER_NAME:$USER_NAME .ssh/id_rsa.pub /home/$USER_NAME/.ssh/id_rsa.pub

# Configurar permisos para las claves SSH
RUN chmod 600 /home/$USER_NAME/.ssh/id_rsa \
    && chmod 644 /home/$USER_NAME/.ssh/id_rsa.pub \
    && chown -R $USER_NAME:$USER_NAME /home/$USER_NAME/.ssh

RUN eval "$(ssh-agent -s)" && ssh-add /home/$USER_NAME/.ssh/id_rsa </dev/null

# Añadir la clave pública de GitHub a known_hosts para evitar problemas de verificación
RUN ssh-keyscan github.com >> /home/$USER_NAME/.ssh/known_hosts

# Iniciar el contenedor con un shell o comando por defecto
CMD ["/bin/bash"]
