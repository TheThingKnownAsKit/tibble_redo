GOALS:
1. no hardcoded indices in the code
2. separation between
    - drivetrain inputs 
    - non-drivetrain inputs
3. ability to swap controllers via YAML

SOLUTION: Replace the built in teleop_twist_joy node with a custom input mapper node
joy -> input mapper -> /cmd vel and /user input

STEPS:
1. Create an input mapping node (can be Python or C++)
2. Define a YAML mapping scheme
3. treat EVERYTHING like a float32 value so digital and analog inputs can be swapped easy (including axes)
4. define a custom gamepad_input message and publish an array of it, controller subscribes to this
    - itll publish both cmd_vel and other inputs

MISC:
1. Use SDL naming (not supported by everything but get a better controller lol)
2. I think you can implement mode switching in the gamepad logic and not even the controller logic ???
3. Device auto mapping might be feasible