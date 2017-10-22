[![MoPlugs](https://github.com/Neill3d/MoPlugs/blob/master/Documentation/Images/MoP.jpg)]()

MotionBuilder Extensions Pack

[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/neill3d)

## Content ##

## Documentation ##

User Guide - https://neill3d.github.io/MoPlugs/
Tech Notes and Dev Log you can find in repository wiki.

### Plugins ###

Precompiled version of plugins you can find in "bin" folder.

Source code is inside Projects folder.
 In projects I'm using shared static libraries
 - without OR SDK dependecies - https://github.com/Neill3d/MoPlugs_Framework
 - with OR SDK dependencies - MotionCodeLibrary folder

### Scripts ###

Full list of scripts you can find in the PythonScripts \ ReadMe.md

## How to Install ##

I have a very useful application to configurate motionbuilder startup paths for plugins and scripts - https://github.com/Neill3d/MoBu_ConfigApp

You have to clone a repository on your computer or just download a zip copy, then specify startup paths for plugins and scripts accroding to your motionbuilder version (plugins_2017 for motionbuilder 2017, etc.)

Scripts startup folders are:
- Lib
- Startup
- Startup_Physics (optional)

## Hardware requirenments ##

 I'm using many nvidia extensions, so to make the morenderer work properly, you should have nVidia card (with gpu chipset Kepler or later)

## Source Code Project ##

 I'm using Visual Studio 2010 64bit for compiling the source code and libraries.

### Dependencies ###

 There are several projects that I'm using in MoPlugs.

#### MoPlugs Framework ####

 Once I've done a code refactoring and move some part of code out of OR SDK (MotionBuilder) dependencies
 https://github.com/Neill3d/MoPlugs_Framework

#### Motion Code Library ####

 That's a shared classes based on MoPlugs Framework but extends it specificly for MotionBuilder tasks.
 
Code and project is inside MotionCodeLibrary folder in the repository.

#### glew ####
 The OpenGL Extension Wrangler Library
 https://github.com/nigels-com/glew

#### glm ####
 OpenGL Mathematics
 https://glm.g-truc.net/0.9.8/index.html

#### NVFX ####

 I like that project a lot. Very powerful, and it's a pity that Tristan stops updating it. That's what OpenGL was missing - to have a flexible and powerful effect system.
 I'm using nvFX statis libraries with some small modifications, you can find it in my repositories. 
 
 https://github.com/Neill3d/nvFX
 
#### OpenColor IO ####

 That extension is used by Composition Effect (3d LUT's)
 https://github.com/imageworks/OpenColorIO
 
#### Newton Physics Engine ####
 https://github.com/MADEAPPS/newton-dynamics

#### rapidjson and Streetview-Explorer ####

 That libraries are used by CubeMap from street view functionality.

 https://github.com/Tencent/rapidjson
 https://github.com/PaulWagener/Streetview-Explorer

## About ##

 Plugins and scripts have been developed during my work on The Wall film for Canadian Film Board.
 
 Author Sergey Solokhin (Neill3d)
 
  Author of ideas and supervising by Cam Christiansen (Anlanda Studio)
