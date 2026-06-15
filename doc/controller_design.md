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
    - this is where you would allocate memory that will exist for the full life of the controller
- on_configure
    - called after the controller is set to the inactive state. occurs when controller STARTS for the first time, but also restarted. Unconfigured -> Inactive
    - reconfigurable parameters should be read in this method
    - create pubs/subs/clients/etc. reoccurring relationships that need to be configured anytime it is restarted
    - create lambda callback functions here
- command_interface_configuration
    - called after on_configure
    - returns a list of InterfaceConfiguration objects
- state_interface_configuration
    - called after command_interface_configuration
    - returns a list of InterfaceConfiguration objects representing state interfaces
- on_activate
    - called when the controller is activated, including after restarts. Inactive -> Active
        - the method should be able to handle restarts, such as setting vars to safe values if needed
    - perform controller specific safety checks here
    - command_interface_configuration and state_interface_configuration are called again after this
- update
    - realtime control loop
    - NOTE: subscriptions operate on a non-realtime thread. Because of this, a realtime buffer is used to transfer the message to the realtime thread
    - general loop: read from state interfaces, calculate control output, write to command interfaces
    - you can use helper functions, but they also must be realtime compliant
- on_deactivate
    - called when the controller stops running. Active -> Finalized
    - Anything that needs to happen to shutdown the controller happens here. Safety outputs to all motors, for example