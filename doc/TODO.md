1. rewrite the state change service
2. create a button press service to abstract controller configs
    - im thinking a YAML file where the control schema's are written down and there is a variable of the name of which one to use
    - maybe use nested YAML files? like include this yaml file if the chosen controller variable is named this
3. joint_state_broadcaster does the wheel transforms for me i think?? look into this, might not need to handle transforms at all. decide how to do transforms
4. decide how im doing odom. multiple options for open loop and for actual encoder feedback. is there a more efficient way to do it than calculate it by hand?
5. look into [kinematics_interface](https://github.com/ros-controls/kinematics_interface) to see if it's possible to not have to do kinematics by hand either
6. with all the above in mind, design the control flow for the update() function
7. implement the update() function
8. redesign the can_comms.hpp file. look to see if theres existing code
9. redesign the microcontroller_comms.hpp file. Look at similar resources (including AR) for reference. Decide on comms schema
    - it might be possible to make this require less manual editing. like having more dynamic amounts of motors
    - is there a more secure way to pass information by serial than string parsing that is also worth the extra effort?
    - overall id like for this file to be way more flexible and plug and play
10. redesign the hardware interface file
    - overall is just a little sloppy
    - should utilize the new improvements made in points 8 and 9
11. add better baremetal support
    - rosdep info and dependency install script maybe


STRETCH GOALS (not in order of importance)
1. foxglove implementation
2. mujoco implementation
3. launch file redesign?
4. theoretical sensor implementation with broadcasters
    - otherwise assume only encoders
5. zenoh implementation
6. nav2 implementation
7. docker rewrite
8. look into ignition by inductive automation (the telemetry service NOT gazebo)
9. reduce bandwidth?