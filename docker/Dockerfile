# Things to keep in mind:
# 1. Every apt-get update makes the build process slower and creates a larger image
# 2. Each RUN command creates a new layer and increases image size

ARG ROS_DISTRO=jazzy
ARG UBUNTU_VER=24.04

FROM ros:${ROS_DISTRO}-ros-base

#################### ENV SETUP ####################
ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8
ENV PYTHONPATH="${PYTHONPATH}:/usr/local/lib"

#################### SYSTEM + DEV TOOLS ####################
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Core system
    ca-certificates \
    curl \
    gnupg \
    gnupg2 \
    lsb-release \
    apt-transport-https \
    wget \
    sudo \
    udev \
    libusb-1.0-0 \
    libserial-dev \
    \
    # Build + Python
    build-essential \
    python3-pip \
    python3-venv \
    python3-colcon-common-extensions \
    python3-vcstool \
    python3-rosdep \
    python3-argcomplete \
    bash-completion \
    pipx \
    python-is-python3 \
    \
    # GUI libs
    libegl1-mesa-dev \
    libglu1-mesa-dev \
    libxv1 \
    libxtst6 \
    \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

#################### PLATFORMIO ####################
RUN pipx install platformio && pipx ensurepath

#################### ROS PACKAGES ####################
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-${ROS_DISTRO}-desktop \
    ros-${ROS_DISTRO}-ros2-control \
    ros-${ROS_DISTRO}-ros2-controllers \
    ros-${ROS_DISTRO}-xacro \
    ros-${ROS_DISTRO}-joint-state-publisher-gui \
    ros-${ROS_DISTRO}-robot-state-publisher \
    ros-${ROS_DISTRO}-rmw-zenoh-cpp \
    ros-${ROS_DISTRO}-foxglove-bridge \
    \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

#################### CTRE PHOENIX ####################
RUN curl -s --compressed -o /usr/share/keyrings/ctr-pubkey.gpg "https://deb.ctr-electronics.com/ctr-pubkey.gpg" && \
    curl -s --compressed -o /etc/apt/sources.list.d/ctr2026.list "https://deb.ctr-electronics.com/ctr2026.list" && \
    sed -i 's/signed-by=/arch=amd64 signed-by=/' /etc/apt/sources.list.d/ctr2026.list && \
    apt-get update && \
    apt-get install -y --no-install-recommends phoenix6

# Required for CTRE to build correctly with colcon
RUN CTRE_LIB_PATH=$(dirname $(find /usr -name "libCTRE_Phoenix6.so" | head -n 1)) && \
    echo "Found Phoenix6 at: $CTRE_LIB_PATH" && \
    echo "$CTRE_LIB_PATH" > /etc/ld.so.conf.d/ctre.conf && \
    ldconfig

#################### USER SETUP ####################
ARG USERNAME=ubuntu
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN if ! id -u $USER_UID >/dev/null 2>&1; then \
        groupadd --gid $USER_GID $USERNAME && \
        useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME; \
    fi && \
    echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME && \
    chmod 0440 /etc/sudoers.d/$USERNAME

# Fedora host serial devices use GID 18 so add yourself just in case
RUN groupadd -g 18 hostserial || true && \
    usermod -aG hostserial $USERNAME

#################### ROS ENV ####################
RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" \
    > /etc/profile.d/ros2.sh

#################### USER ENV ####################
USER $USERNAME
WORKDIR /workspaces/2026-Tibble-Bot/

RUN mkdir -p ~/.colcon && \
    printf "build:\n  symlink-install: true\n  cmake-args: [\"-DPython3_EXECUTABLE=/usr/bin/python3\"]\n" \
    > ~/.colcon/defaults.yaml

RUN printf "\nsource /opt/ros/${ROS_DISTRO}/setup.bash\n" >> ~/.bashrc && \
    printf "if [ -f /workspaces/2026-Tibble-Bot/install/setup.bash ]; then source /workspaces/2026-Tibble-Bot/install/setup.bash; fi\n" >> ~/.bashrc && \
    printf "alias use_fastdds='export RMW_IMPLEMENTATION=rmw_fastrtps_cpp && echo \"[ROS 2] Middleware swapped to FastDDS\"'\n" >> ~/.bashrc && \
    printf "alias use_zenoh='export RMW_IMPLEMENTATION=rmw_zenoh_cpp && echo \"[ROS 2] Middleware swapped to Zenoh\"'\n" >> ~/.bashrc
