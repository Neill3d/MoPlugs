[Back To Main Page](README.md)

[Back To Plugins Page](Plugins.md)

# GPU Geometry Cache plugin

## Hardware requirenments

The plugins uses bindless textures extension - ARB_bindless_texture

## What is static geometry

Categories which doesnt support by gpu cache shader. Menu item “Select static geometry for caching” uses that list of options:

object should be:
- not a light
- not a camera
- not a null
- not a skeleton
- not a path object

- should be visible
- not deformable
- not constrained
- has existing geometry

- not animated on translation, rotation, scaling
- parent heirarchy should be static (not deformable, constrained or animated)

- no image sequences assigned
- no additive transparency in shaders assigned

## Standalone viewer

In MoPlugs Framework there is a ViewerApp to load and display geometry caches
