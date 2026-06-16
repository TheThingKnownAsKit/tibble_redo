Controller plugins have four finite states:
1. Unconfigured
2. Inactive
3. Active
4. Finalized

robot_controller.yaml
- 

robot_controller.hpp
- 

robot_controller.cpp
- on_init
    - called only once when the plugin is loaded
    - the param values for joints, command_interfaces, and state_interfaces should be declared and accessed
    - this is where you would allocate memory that will exist for the full life of the controller, like declaring node parameters
- on_configure
    - called after the controller is set to the inactive state. occurs when controller STARTS for the first time, but also restarted. Unconfigured -> Inactive
    - Notably, NOT part of the realtime loop
    - reconfigurable parameters should be READ in this method. Declare them in on_init or hpp
        - often, they don't even need to be read here if they've already be set to local variables in on_init
    - create pubs/subs/clients/etc. reoccurring relationships that need to be configured anytime it is restarted
    - create lambda callback functions here
- command_interface_configuration
    - called after on_configure
    - returns a list of InterfaceConfiguration objects
- state_interface_configuration
    - called after command_interface_configuration
    - returns a list of InterfaceConfiguration objects representing state interfaces
- on_activate (RT)
    - called when the controller is activated, including after restarts. Inactive -> Active
        - the method should be able to handle restarts, such as setting vars to safe values if needed
    - perform controller specific safety checks here
    - command_interface_configuration and state_interface_configuration are called again after this
- update (RT)
    - realtime control loop
    - NOTE: subscriptions operate on a non-realtime thread. Because of this, a realtime buffer is used to transfer the message to the realtime thread
    - general loop: read from state interfaces, calculate control output, write to command interfaces
    - you can use helper functions, but they also must be realtime compliant
- on_deactivate (RT)
    - called when the controller stops running. Active -> Finalized
    - Anything that needs to happen to shutdown the controller happens here. Safety outputs to all motors, for example

## HOW DO NODE YAML FILES WORK ?????

Basically, it goes YAML -> controller_manager node -> tibble controller node -> auto_declare/get_parameter

Parameters under tibble_controller.ros__parameters are injected into the controller node BEFORE on_init() runs

You just have to use an auto_declare statement in on_init and toss the node parameter values to local variables.