## English version

# ROS2 Humble Docker

This repository contains scripts to build containers based on ROS 2 Humble images. Its content is based on the requirements for the [Robot bombero UAO](https://github.com/smilagros246/robot_bombero_uao.git) project, but you can fork it and adapt it to your project's needs.

The container runs with the `--rm` option, so it is automatically deleted upon exit.
However, **files and folders mounted from the local system via volumes (**``**) remain untouched**, since they reside outside the container and are stored directly on the host machine.

This repository is inspired by [Tim Dupont's ROSNoetic docker](https://github.com/PXLAIRobotics/ROSNoeticDocker) and [Legged\_ws](https://github.com/leggedrobotics-usp/legged_ws.git) from the University of São Paulo.

## Requirements

- OS used: Ubuntu 22.04 on PC, Raspberry Pi OS with desktop on the Pi.
- Git installed:
  ```bash
  sudo apt install -y git
  ```
- Docker installed (tested on Docker version 26.1.4, but other versions may work).
- It is recommended to have an SSH key set up on your PC and any embedded system (e.g., Raspberry Pi).

To create an SSH key, you can follow the [Linux Stuffs](https://github.com/lomcin/linux-stuffs) repository using these steps:

1. Clone the repository:
   
   ```bash
   git clone https://github.com/lomcin/linux-stuffs.git
   ```
2. Go to the appropriate folder:
   
   ```bash
   cd linux-stuffs/crypto/ssh
   ```
3. Generate the key:
   
   ```bash
   ./generate_key.sh rsa email_github@yourOrg.co
   ```
   
   > You'll be prompted for a password. Leave it empty for convenience.
4. Display your public key:
   
   ```bash
   cat ~/.ssh/id_rsa_pub
   ```
5. Copy the output and paste it into the **SSH and GPG keys** section of your GitHub profile.

- Installing GPU drivers is recommended if your computer has one.

## How to build the repository

The base project uses ROS 2 running simultaneously on both a Raspberry Pi 4 and a PC. It includes bash scripts to build Docker images tailored to each device:

- `01_build_image.sh` builds a Docker image based on ROS 2 Humble Desktop. It sets up locales and Python, allows dynamic selection of the Gazebo version, installs essential system tools, ROS libraries, and packages like controllers, Rviz2, and Gazebo ROS 2 Control. It also creates a user with appropriate permissions, manages SSH keys for GitHub, and configures a personalized environment with Vim and persistent ROS setup.
- `01_build_image_raspberry.sh` builds a lightweight image designed for Raspberry Pi, based on ROS 2 Humble Core. It includes basic development tools, control packages, I2C communication, and teleoperation. It creates a user, initializes `rosdep`, and sets up a Colcon/ROS 2 workspace.

To run either script:

```bash
./01_build_image.sh TagName local_user
```

```bash
./01_build_image_raspberry.sh TagName local_user
```

Ensure all scripts are executable: `chmod +x scriptName`.

## How to run the container

On the PC:

```bash
./02_run_container.sh TagName
```

On the Raspberry Pi:

```bash
./02_run_container_raspberry.sh TagName
```

Once inside the container, you can compile ROS 2 packages using `colcon build` from the `ros2_ws` workspace.

## Additional Resources

Inside the `Recursos útiles` folder, you’ll find:

- Arduino codes (`bomberoDebug.ino` and `bomberoTerminal.ino`) to establish serial communication between an Arduino Nano and the Raspberry Pi 4 for teleoperation.
- Test code for various components used in the robot. For more details, refer to the non-functional requirements in [Robot bombero UAO](https://github.com/smilagros246/robot_bombero_uao.git).
- Python scripts for terminal-based teleoperation (without ROS 2).

## Notes

If you prefer not to use SSH, you’ll need to change the Dockerfile to use HTTPS instead of SSH. Update this line in both files:

![1750086402340](images/README/1750086402340.png)

> Keep in mind that cloning via HTTPS may restrict Git contribution workflows and collaborative features.

