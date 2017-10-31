[Back To Main Page](README.md)

## Limitations
* plugins are under construction and could cause a crash
* extensions work with motionbuilder starting from version 2014
* graphics plugins are oriented on nvidia graphic extensions and could not work properly on AMD or Intel graphic cards

## How to Install ##

 In GitHub repository there is a bin folder with plugin binaries and system resources. There are sets of plugins for each version of motionbuilder starting from 2014, you have to use the one which suits your motionbuilder version.
 And PythonScripts folder contains all sort of scripts - startup tools, shared libraries and actions scripts. 

You have to download or clone a repository on your computer, then specify startup paths for plugins and scripts accroding to your motionbuilder version (plugins_2017 for motionbuilder 2017, etc.)
 To make motionbuilder run with plugins you can:
* copy plugins into common startup folder. For example, the default plugins folder is C:\Program Files\Autodesk\MotionBuilder 20xx\bin\x64\plugins
* manualy define Windows system variables for a startup plugins path (MOTIONBUILDER_PLUGIN_PATH) 
* use ConfigApp to specify startup paths

For scripts there is also a way to define startup paths with MOTIONBUILDER_PYTHON_STARTUP environment variable. 

I have a very useful application to configurate motionbuilder startup paths for plugins and scripts - [ConfigApp](ConfigApp.md)

## Hardware requirenments ##

 I'm using many nvidia extensions, so to make the morenderer work properly, you should have nVidia card (with gpu chipset Kepler or later)

## Video ##

Here is a video introduction to install process
