# ROS2 Humble Docker

Este repositorio contiene los scripts para contruir contenedores a partir de imagenes de ROS2 humble. Su contenido esta basado en los requerimientos para el uso en el proyecto de [Robot bombero UAO](https://github.com/smilagros246/robot_bombero_uao.git), sin embargo puedes hacer un fork y adecuarlo a los requerimeintos de tu proyecto.
El contenedor se ejecuta con la opción `--rm`, por lo que se elimina automáticamente al finalizar.
Sin embargo, **los archivos y carpetas montados desde el sistema local mediante volúmenes (`-v`) permanecen intactos**, ya que están fuera del contenedor y se almacenan directamente en el PC local.
Este repositorio esta inspirado por [Tim Dupont's ROSNoetic docker](https://github.com/PXLAIRobotics/ROSNoeticDocker ) y  [Legged_ws](https://github.com/leggedrobotics-usp/legged_ws.git) de la universidad de Sao Paulo.

## Requerimientos para la ejecución adecuada

- Sistema operativo en el que se realizó: Ubuntu 22.04 para el PC, en la raspberry Pi se uso el sistema Raspberry Pi OS con desktop.
- Tener instalado git con:
  
  ```bash
  sudo apt install -y git
  ```
- Tener la instalación de Docker (este repositorio fue probado con la versión Docker version 26.1.4 pero puedes usar otra si gustas).
- Se recomienda contar con el registro de tu llave SSH local en tu PC y otro elemento embeido (ej. raspberry Pi).

Para crearla puedes guiarte del repositorio [Linux Stuffs](https://github.com/lomcin/linux-stuffs), siguiendo estos pasos:

1. Clona el repositorio:
   
   ```bash
   https://github.com/lomcin/linux-stuffs.git
   ```
2. Accede a la carpeta correspondiente:
   
   ```bash
   linux-stuffs/crypto/ssh
   ```
3. Genera tu llave SSH ejecutando el siguiente comando (reemplaza el correo con el asociado a tu cuenta de GitHub):
   
   ```bash
   ./generate_key.sh rsa email_github@tuOrganizacion.co
   ```
   
   > Se te pedirá una contraseña, la cual deberás ingresar cada vez que uses la llave. (se recomienda dejar en blanco)
4. Visualiza tu llave pública con:
   
   ```bash
   cat  ~/.ssh/id_rsa_pub
   ```
5. Copia el contenido mostrado y agrégalo en la sección **SSH and GPG keys** de tu perfil de GitHub.

- También se recomienda contar con la instalación de los drivers de GPU si tu computador cuenta con ella.

## Como construir el repositorio

Considerando que el proyecto base consta de la ejecución simultanea en ROS2 usando una raspberry Pi4 y el PC, se proporcionan varios scripts tipo bash para constuir la imagen del contenedor según la funcionalidad dada en cada dispositivo:

- `01_build_image.sh`para construir una  imagen Docker basada en ROS 2 Humble Desktop . Configura localización y Python, permite la selección dinámica de versiones de Gazebo, instala un amplio conjunto de herramientas del sistema, bibliotecas de ROS y paquetes esenciales como controladores, Rviz2 y Gazebo ROS 2 Control. Además, crea un usuario dinámico con permisos adecuados, gestiona claves SSH para conexión con GitHub, y establece un entorno de trabajo personalizado con Vim y configuración persistente de ROS.
- `01_build_image_raspberry.sh`Para construir una imagen para usar en la raspberry Pi. Esta imagen Docker liviana está basada en ROS 2 Humble Core y está diseñada para ejecutarse en Raspberry Pi. Incluye herramientas esenciales de desarrollo, paquetes ROS 2 para control de hardware, comunicación I2C y teleoperación. Configura un usuario con permisos adecuados, inicializa `rosdep` y deja preparado el entorno de trabajo con Colcon y ROS 2.

Para la ejecución de cada una se debe hacer:

```bash
./01_build_image.sh NombreTag usuario_local
```

```bash
./01_build_image_raspberry.sh NombreTag usuario_local
```

Asegurate de tener todos los scripts ejecutable usando `chmod +x nombreArchivo` .

## Como iniciar el contenedor

Para ejecutar el script en el PC ejecutar:

```bash
./02_run_container.sh NombreTag
```

Para ejecutar el script en la raspberry ejecutar:

```bash
./02_run_container._raspberrysh NombreTag
```

Una vez dentro, puedes compilar los paquetes de ros2 usando colcon build desde el workspace ros2_ws.

## Recuersos adicionales

En la carpeta Recursos útiles pueden encontrar:

- Los códigos de arduino usados( `bomberoDebug.ino`y `bomberoTerminal.ino`) para establecer la comunicación serial entre un Arduino nano y la raspberry Pi4 para lograr el control de teleoperación.
- Códigos de pruebas de los diferentes elementos usados en el robot, para más información consultar en los requerimientos no funcionales de [Robot bombero UAO](https://github.com/smilagros246/robot_bombero_uao.git)
- Códigos de python para pruebas de ejecución de los elementos con manejo teleoperado desde el terminal (sin uso de ROS2).

