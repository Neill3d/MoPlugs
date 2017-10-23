[Back to main page](README.md)

# MoPlugs Projects #

Plugins are developed in VS 2010 SP1 and for running them you need to have installed Microsoft Visual C++ 2010 SP1 Redistributable Package (x64). If you don't have one, please check an official microsoft [download center](https://www.microsoft.com/en-us/download/details.aspx?id=13523)

To manage loading scripts and plugins from a custom directory, I have a useful [configuration application](https://github.com/Neill3d/MoBu_ConfigApp)

## Hardware requirements ##

Most of plugins are using nVidia oriented extensions, and it's better to use a video card on Kepler chipset or newer (for example, starting from GeForce family 6xx).

- Composition Toolkit is based on OpenGL Compute Shaders (OpenGL 4.3)
- GPU Particles shader is based on OpenGL Compute Shaders
- Dynamic Masks Tool uses nVidia "NV_path_rendering" extension.
- GPU Cache uses bindless textures extension.

## MoPlugs Solution ##

Topics
* [Aim Sprite Constraint And Sprite Sheet Solver](SpriteSheetSolver.md) - constraint to make billboards from planes and solver to animate atlas sprite textures
* [BlendShape Toolkit](BlendShapeToolkit.md) - Toolkit for adding and editing geometry blendshapes.
* [manager_CameraLinkVis](CameraLinkVisPlugin.md) - Plugin connect a group visibility to the specified camera. The group showing up when camera is current.
* [tool_RenderLayers](RenderLayersTool.md) - Tools that helps to manage layer of objects with assigned shaders, materials and textures. Similar to Maya display layers.
* [render from a command line](RenderFromCmdLine.md) - this is a part of tool_RenderLayers functionality, you can use command line to start a render process with a specified parameters
* [Wall Bricks](WallBricks.md) - constraint and scripts to make an animation of raising down blocks along a curve 

Folders
* cmdFBX
* constraint_ColumnPlay
* constraint_wallBricks
* device_shapeHand_test
* device_shapeHand
* manager_CameraLinkVis
* manager_ImageSequencePack
* misc_FileSizeChecker
* py_editing

## MoGraphics Solution ##

* [Composition Toolkit](CompositionToolkit.md) - real-time composing (for morenderer only)
* [Dynamic Masks](DynamicMasks.md) - Tool for working with dynamic textures, creating animated masks
* [GPU Geometry Cache](GPUGeometryCache.md)
* [GPU Particles Shader](GPUParticlesShader.md)
* [ProjTex Shader](ProjTexShader.md) - It's a shader plugin, that has been developed to support projected textures on a mesh with masking and special features of composition toolkit and moRenderer

Folders
* py_bakeProjectors
* render_colors
* shader_SkyBox
* texture_advanceBlend
* tool_bakeProjectors

## Interactive Solution ##

 Commands for story editor and boxes for relation constraints.
Plugin adds functinality to play camera effects (shake, zoom), to control timeline slider, etc. 

* [Interactive Pack](InteractivePack.md)

## Facial Solution ##

Import 2d tracked data from

- Campbridge features tracker
- Shake shade data (tracked in Mocha)
- Faceware landmarks data (from Analyzer)

Retargeting constraint to drive 3d face joints by 2d tracked data

Folders
* common_Projectors
* constraint_facialRetarget
* py_bakeProjectors
* test_stabilization
* tool_impFaceFeatures

## MoPhysics Solution ##

 Advanced car simulation
 
 [MoPhysics Solver](MoPhysicsSolver.md)
 
 
## Global roadmap ##

- finish MoRenderer render conveyer
- rewrite extended cubemap rendering to use new cubemap object
