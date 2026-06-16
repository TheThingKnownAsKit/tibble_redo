GOAL: write better ros2 control architecture

IGNORING: Sim, autonomy
NOT IGNORING: ros2 control and related files, including all of the interfaces/ package

## Files of concern

- tibble_controller.yaml
    - MORE things should be in here. It's the easiest thing to edit. Look into if it matters where private variables are declared
    - Hardware interface variables are declared in the URDF rn. Do they HAVE to be? It'd be better to be in the yaml
    - Can we put the controller button schema in here too?
    - The dream is to have no config variables in any cpp/hpp file
- control/CMakeLists.txt and control/package.xml
    - The current CMakeLists.txt is a mess of slop. It should be edited to remove redundancy, make it more template-y, and comment better
    - The package.xml is fine but I'd still like to look at it
- SetState.srv
    - Clean up a little bit and make things more clear
- set_state_client.cpp
    - This is a mess of made in a rush AI slope, rewrite the whole thing
    - Should more things be a service?
- tibble_controller.cpp
    - This is a mess of made in a rush AI slope, rewrite the whole thing
- tibble_hardware_interface.cpp
    - This is fine for the most part. Clean up a bit and template-ify
    - More clear divison of responsibilities between the controller, this, and the _comms files
        - The controller decides what to do (including limit checks)
        - The HWC translates that into units things will understand (only conversions should happen here)
        - The _comms files do nothing but ferry what the HWC wants down the pipeline (no calculations should happen here)
            - Limit checks in other files besides the controller is permitted if the limit is INCREDIBLY hardware specific or other misc. scenarios
- tibble_controller.hpp
    - See what was said about yaml and the source file
- tibble_hardware_interface.cpp
- microcontroller_comms.hpp
    - Relatively fine. Need to look into the specific implementation of variable passing
    - Implement a mock microcontroller comms feature that just prints outputs to console? would be nice for testing
- can_comms.hpp
    - Kind of not fine. Need to look into more about CTR's API and how this should be written
    - Mock can feature that just prints output to console would also be useful here

Broadcasters are used for sensor data from hardware. Might need to write a custom one for a LiDAR? Depends on how the LiDAR as odometry library works

Will need a launch file too but who cares

## Interesting docs

https://design.ros2.org/articles/node_lifecycle.html

MoveIt is an intersting arm manipulator that might be useful for the senior design project but is probably wildly unnecessary for lunabotics. https://moveit.picknik.ai/main/index.html 

## Interesting things

Someone's projects that have a LOT of custom controllers:
- https://github.com/legubiao/quadruped_ros2_control
- https://github.com/fiveages-sim/arms_ros2_control

Another someone
- https://github.com/mateusmenezes95/omnidirectional_controllers
    - this is particularly useful since it is a mobile platform robot that publishes odom and tf

Commonplace Robotics
- https://github.com/CommonplaceRobotics/iRC_ROS

## Maybes

- Care about launch files
- Care about sim
- Investigate micro ros (not really a summer thing)