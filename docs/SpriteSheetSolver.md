[Back To Main Page](README.md)

[Back To Plugins Page](Plugins.md)

# Multi Aim (Aim Sprite) constraint and Sprite Sheet Solver

Constraint to make billboards for ton of objects


## How to make an atlas. Tips and Tricks

For compose images inside one atlas you can use free version of [TexturePacker](https://www.codeandweb.com/texturepacker)

Important to choose pack options:

In Texture group Png Opt Level should be set to zero in freeware version

**Note!** For faster MoBu texture load, better to convert texture into dds

[Photoshop plugin to work with dds](https://developer.nvidia.com/nvidia-texture-tools-adobe-photoshop)

Layout Max size W:16384, H: 16384

Size constraint better to choose as POT (Power of 2)

Then
- Algorithm Basic
- Sort by Name
- Turn off Allow Rotation and Detect identical sprites

In Sprites group you should turn off Trim mode, and set Extrude to zero.

The most important thing is to make a clear naming and frame enumeration

Wrong:
- head 1
- head 10
- head 2
- etc.

Correct:
- head_001
- head_002
- …
- head_010

Solver supports up to 40 different ranges inside one atlas. Default number of ranges is 3.


