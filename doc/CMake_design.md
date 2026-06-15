Reference the custom controller CMakeLists.txt at https://github.com/ros-controls/ros2_control_demos/blob/master/example_7/CMakeLists.txt and add in the phoenix6 stuff

Remember to install jazzy dev tools to get rosdep
Remember to use rosdep to install ros2_control_cmake

```sh
sudo apt update && sudo apt upgrade
sudo apt install ros-dev-tools
sudo rosdep init
rosdep update
rosdep install --from-paths src --ignore-src -r -y
source /opt/ros/jazzy/setup.sh
colcon build
```