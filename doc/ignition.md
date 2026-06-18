ROS2 OPC UA hybrid nodes: https://github.com/Mariunil/ros2-opcua
ROS2 OPC UA bridge: https://github.com/sequenceplanner/opcua_ros2_bridge
ROS2 OPC UA gateway (production): https://docs.ros.org/en/humble/p/ros2_medkit_opcua/index.html
Research paper: https://cea.hal.science/cea-03870393/document

OPC UA overview: https://docs.inductiveautomation.com/docs/8.3/ignition-modules/opc-ua
OPC UA client configuration: https://docs.inductiveautomation.com/docs/8.1/ignition-modules/opc-ua/opc-ua-client-connection-settings

ROS2 -> OPC UA bridge/gateway -> Ignition
(more specifically ROS2 topics -> OPC UA server -> Ignition OPC UA client -> dashboards)

MQTT for telemetry streaming? This is for SCADA stuff (whatever that means also what are PLCs). Somewhat common to do ROS2 -> MQTT bridge -> ignition mqtt module

This is wildly overkill for an after school club is basically what I'm gathering. You'd implement Ignition if you had an entire fleet of warehouse bots you needed to track