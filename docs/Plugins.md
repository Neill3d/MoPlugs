[Back to main page](Main.md)

# MoPlugs Projects #

All projects I've developed in VS 2010 SP1.

## You should specify environment variables ##

That will help projects to locate your OR SDK.

For example, for motionbuilder 2014 the variable name is **ADSK_MOBU_2014_64**

And variable value for a default motionbuilder installation path is this - C:\Program Files\Autodesk\MotionBuilder 2014

## Hardware requirenments ##

Most of plugins are using nVidia oriented extensions, and it's better to use a video card on Kepler chipset or newer (for example, starting from GeForce family 6xx).

- Composition Toolkit is based on OpenGL Compute Shaders (OpenGL 4.3)
- GPU Particles shader is based on OpenGL Compute Shaders
- Dynamic Masks Tool uses nVidia "NV_path_rendering" extension.
- GPU Cache uses bindless textures extension.

## MoPlugs Solution ##

Solution - moplugs.sln

Folders
* cmdFBX
* constraint_AimSprite
* constraint_ColumnPlay
* constraint_wallBricks
* device_shapeHand_test
* device_shapeHand
* manager_CameraLinkVis
* manager_ImageSequencePack
* misc_FileSizeChecker
* py_editing
* tool_BlendShape
* tool_RenderLayers

## MoGraphics Solution ##

Solution - mo_graphics.sln

Folders
* ImportGeomCache
* **mo_graphics**
* py_bakeProjectors
* render_colors
* shader_GPU_Particles
* shader_SkyBox
* texture_advanceBlend

## Interactive Solution ##

 Commands for story editor and boxes for relation constraints.
Plugin adds functinality to play camera effects (shake, zoom), to control timeline slider, etc. 

Solution - interactive_pack.sln

Folders
* interactive_pack

## Facial Solution ##

Import 2d tracked data from

- Campbridge features tracker
- Shake shade data (tracked in Mocha)
- Faceware landmarks data (from Analyzer)

Retargeting constraint to drive 3d face joints by 2d tracked data

Solution - facial.sln

Folders
* common_Projectors
* constraint_facialRetarget
* py_bakeProjectors
* test_stabilization
* tool_bakeProjectors
* tool_impFaceFeatures

## MoPhysics Solution ##

 Advanced car simulation
 
 Solution - mophysics.sln
 
 Folders
 * Common_Physics
 * library_bulletPhysics (depricated)
 * library_newtonPhysics
 * solver_MoPhysics
 
 
## Global roadmap ##

- finish MoRenderer render conveyer
- rewrite extended cubemap rendering to use new cubemap object
