# rviz_plugins_ws
**ROS workspace (catkin) for development of visualization plugins for [RVIZ](http://wiki.ros.org/rviz)**

Created based on examples from [ros-visualization/visualization_tutorials](https://github.com/ros-visualization/visualization_tutorials/tree/kinetic-devel/rviz_plugin_tutorials)

The following plugins are currently implemented:
## BatteryPanel plugin
Rviz panel for displaing statue of robot's battery, specifically:
* Charging status
* Battery Voltage
* Load Current
* Remaining Charge indicator
* Internal Resistance

![BatteryPanel](https://github.com/icboredman/rviz_plugins_ws/blob/master/src/rviz_plugins/BatteryPanel.png)

The information is obtained by subscribing to [sensor_msgs/BatteryState](http://docs.ros.org/melodic/api/sensor_msgs/html/msg/BatteryState.html) messages on a topic

## TeleopPanel plugin
Rviz panel for publishing [geometry_msgs/Twist](http://docs.ros.org/api/geometry_msgs/html/msg/Twist.html) messages on a topic

The only change made to the example code is a slighty different scale in `DriveWidget::DriveWidget()`

