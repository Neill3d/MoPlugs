
[Back To Main Page](README.md)

[Back To Scripts Page](Scripts.md)

Plugin to control groups visibility depends on current camera in the viewport.

Python Script tool - CameraAndGroupLinkingTool.py

Manager plugin - 

## Plugin Dev Notes ##
 
 Plugin manager is tracking UI Idle event to check for a current active camera and assigned group to control it's visibility.

 Plugin is looking for a property "LinkedGroup" in a camera. The property should contain a group name to link them together. If there is a group with such a name, then a group will be visible only when camera is active.

 Python script helps to add a camera property and assign a group name from a list of scene groups.
