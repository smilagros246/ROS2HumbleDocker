# ROS2 Humble Docker

This container contains the scripts to build and run a ROS2 Humble container.
This repository is inspired by Tim Dupont's ROSNoetic docker.
https://github.com/PXLAIRobotics/ROSNoeticDocker


## How to build the container

A bash script is provided to build the container, it can be executed by entering the following command:
```bash
./01_build_image.sh NombreTag usuario_local
```

Make sure you make the scripts executable:
```bash
chmod +x 01_build_image.sh
```
```bash
chmod +x 02_run_container_sh
```

## How to start the container

To start the container execute the script below:
```bash
./02_run_container.sh NombreTag
```
