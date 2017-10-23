[Back to Main Page](README.md)

[Back to Plugins Page](Plugins.md)

[Back to Scripts Page](Scripts.md)

# Configurate Plugins and Scripts paths

The application that could help to manage plugins and scripts startup paths.
When MotionBuilder starts, it will use selected paths to load plugins and scripts.

![](https://github.com/Neill3d/MoBu/blob/master/Documentation/Images/ConfigApp.jpg)

_NOTE!_ To work properly, the application should be opened with administration rights.

The App works with environment variables 
* MOTIONBUILDER_PLUGIN_PATH
* MOTIONBUILDER_PYTHON_STARTUP

### UI

**Refresh** - button to check current environment variables set and to update list boxes

**Apply** - checked paths will be added to environment variables, so that motionbuilder will load plugins and script on next launch

### Download

 Source code and compiled windows binary you can find in [ConfigApp repository](https://github.com/Neill3d/MoBu_ConfigApp)
