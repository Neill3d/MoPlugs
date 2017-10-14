# MoPlugs
MotionBuilder Extensions Pack

## Content ##

### Pre compiled binaries ###

In progress

### Plugins ###

In progress

### Scripts ###

Full list of scripts you can find in the PythonScripts \ ReadMe.md

## Documentation ##

 You can find some articles and notes in repository wiki.

## Hardware requirenments ##

 I'm using many nvidia extensions, so to make the morenderer work properly, you should have nVidia card (with gpu chipset Kepler or later)

## Source Code Project ##

 I'm using Visual Studio 2010 64bit for compiling the source code and libraries.

### Dependencies ###

 There are several projects that I'm using in MoPlugs.

#### MoPlugs Framework ####

 Once I've done a code refactoring and move some part of code out of OR SDK (MotionBuilder) dependencies
 https://github.com/Neill3d/MoPlugs_Framework

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
