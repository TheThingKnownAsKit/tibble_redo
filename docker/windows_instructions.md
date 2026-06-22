# IF YOU ARE ON A WINDOWS DEVICE AND WANT TO RUN THIS IN A DOCKER CONTAINER, THESE ARE YOUR INSTRUCTIONS

First of all, because this project requires access to USB input, GPU, and GUI, it is really annoying to get that to work on windows. You have essentially two options:
1. build the container in Windows and configure it to work with a WSL layer underneath
2. build the container in WSL and it'll still work with a WSL layer underneath

It is HIGHLY recommended that you pick option 2. Option 1 really has no benefit. You still have to set up WSL, configure docker to work with it, and it'll all still run in WSL under the hood. All linux docker containers run in WSL under the hood anyways. Picking option 2 means less configuration, the ability to work on bare metal in Linux still, and higher performance. It does mean you will have to work more with Linux, which may be an uncomfortable or new experience, so this guide will walk you through the process.

## Installing up WSL

What is WSL? It stands for Windows Subsystem for Linux. It basically allows you to install an entire Linux operating system on your computer that you can access as a subsystem. Your primary operating system will still be Windows, you can just access Linux through a command line and develop in it as if it were a real Linux machine. This has it's limitations, but it should work enough for ROS2 development kinda sorta.

You can follow the [official Windows WSL install instructions](https://learn.microsoft.com/en-us/windows/wsl/install) at the link, but I'll repeat it here for convenience.

1. Open PowerShell in administrator mode
    - Press the Windows button on your keyboard or by pressing the menu in the taskbar
    - Type in PowerShell
    - Click on the Run as administrator option
2. Type in the command `wsl --install` to install WSL itself
3. Restart your computer
4. Open PowerShell again (admin mode not required)
5. Type in the command `wsl --list --online` to view the available Linux distributions
6. Type in the command `wsl --install --Ubuntu-24.04`
7. Type in your Unix username (doesn't really matter)
8. Type in your Unix password (remember this you will be typing it a lot, or write it down)

**After initially installing a Linux distribution, close that PowerShell window and do not continuing working in it.** For some reason, PowerShell  will automatically send you into the system directory on the Linux distribution and not the user's home directory. That's like automatically sending someone to the System32 directory on Windows instead of Documents or something. It is extremely not advised to do any work there.

**INSTEAD, to open WSL from now one, press the Windows button on your keyboard or in the taskbar and type WSL.** Click on that executable (do not run it as an administrator) and it will take you to your user home directory instead. I recommend pinning this app to your taskbar or start page or whatever you prefer.

## WSL Setup

Now, we are going to configure WSL for you to be able to work in it regularly. This will also let you work bare metal and not necessarily need the docker container, if you'd prefer.

1. Open WSL using the app NOT PowerShell (see above)
2. Set up a GitHub SSH key so you can pull/push in WSL
    - Type the command `ssh-keygen -t ed25519 -C "your_email@example.com"`
    - Press enter to accept default file location
    - Optionally enter a passphrase
    - Type the command `cat ~/.ssh/id_ed25519.pub`
    - Highlight the entire output, starting with ssh-ed25519 and ending with your email address, and copy it
    - Go to https://github.com/settings/keys
    - Click the New SSH key green button in the topish right
    - Put whatever title you want (ex. WSL_Ubuntu-24.04 or something)
    - Paste the contents of the cat command into the Key box
    - Click the Add SSH key at the bottom
3. Type the command `mkdir repos && cd repos`
4. Clone the repository via SSH
    - Go to the repository on github.com
    - Click the green button that says Code
    - Switch over to the SSH tab
    - Copy that
    - In the WSL terminal, type `git clone <whatyoucopied>`

The repository is now fully downloaded into WSL. You can now work directly in this installation. I recommend setting up VSCode to work with this.

DO THIS TO RUN DOCKER IN WSL WITHOUT DOCKER DESKTOP: https://medium.com/geekculture/run-docker-in-windows-10-11-wsl-without-docker-desktop-a2a7eb90556d
DO THIS TO RUN DOCKER IN WSL WITH DOCKER DESKTOP: https://docs.docker.com/desktop/features/wsl/ 

## USB Passthrough

Follow these instructions to set up usb pass through to WSL (https://learn.microsoft.com/en-us/windows/wsl/connect-usb). Otherwise, nothing connected to your computer by USB will be accessible by WSL.

**Note: the USB stack from Windows to WSL is terrible and barely works, especially for gamepads.** Because it is a Windows product, the USB passthrough is filled with redundancy that introduces latency. This is fine for things like serial communication, but for USBs with high input streams and required low latency (like a gamepad), this more often than not breaks it completely. It is unlikely that you will be able to get a gamepad to work with WSL unless you run the joy node on Windows and pass it through to WSL but that is more trouble than it's worth. Instead, use [teleop_twist_keyboard](https://docs.ros.org/en/ros2_packages/rolling/api/teleop_twist_keyboard/) to generate cmd_vel. Logic for keyboard based attachment control is a heavy maybe.

## VSCode Setup

1. Open VSCode, it can be an empty window
2. Install the WSL extension
3. Do ctrl + shift + p
4. Type WSL: Connect to WSL
5. You should now be in the WSL filesystem. Navigate to your project repository and open it
6. Install the Remote Development and Container Tools extensions
7. Right click on the compose.yaml and select Compose Up