[Back To Main Page](README.md)

# MoPlugs Scripts #

For more information, please have a look at wiki pages.
Note! Most of scripts and tools have been developed in MoBu 2014 

## Action ##

 Scripts that have a single execution scenario
 
* CarSim \ **ReConnectJoystick.py** - the script helps to fix an issue with a missing or stucked joystick device after scene reloading in mobu

### Scripts for the Wall Bricks Constraints and building blocks logic ###

[User Guide](WallBricks.md)

* DuplicateAlongACurve \ **ConvertPolySpline.py** - for imported from maya and c4d polysplines. Script helps to convert it into 3d Path Model. 
* DuplicateAlongACurve \ **DeformMeshByCurve.py** - to bend geometry model by 3d Path
* DuplicateAlongACurve \ **DeformMeshByCurve2.py**
* DuplicateAlongACurve \ DuplicateAlongACurve_DeformMesh.py
* DuplicateAlongACurve \ DuplicateAlongACurve_HardPlot.py
* DuplicateAlongACurve \ DuplicateAlongACurve_RotateBy90.py
* DuplicateAlongACurve \ DuplicateAlongACurve_SelectModels.py
* DuplicateAlongACurve \ DuplicateAlongACurve_SelectNulls.py
* DuplicateAlongACurve \ DuplicateAlongACurve_SETUP.py
* DuplicateAlongACurve \ FixInstanceGeometry.py

### Scripts for work with FaceWare 2d tracked data and my retargeting rig ###

[Facial Notes](Facial.md)

* FacialRIG \ CHECK_FacialData_VideoPlane.py
* FacialRIG \ FacialRIG_connectJoints.py
* FacialRIG \ FacialRIG_CreateVideoPlane.py
* FacialRIG \ FacialRIG_HardPlot.py
* FacialRIG \ FacialRIG_OpticalPostProcessing.py
* FacialRIG \ FacialRIG_SetupConstraints.py
* FacialRIG \ FacialRIG_SetupConstraints2.py
* FacialRIG \ StabilizationScenario.py

### Script to work with a Measurand ShapeHand glove and it's recorded animation result ###

* ShapeHand \ ShapeHand_CorrParentChildConstraints.py
* ShapeHand \ ShapeHand_CorrRelationConstraint.py
* ShapeHand \ ShapeHand_CreateAnIntermediateHandsChar.py
* ShapeHand \ ShapeHand_printHeirarchy.py
* ShapeHand \ ShapeHand_RelationConnections.py
* ShapeHand \ ShapeHand_Rename_DeviceSet_toBVH.py
* ShapeHand \ ShapeHand_RenameBVH_to_DeviceSet.py
* ShapeHand \ ShapeHand_RenameSkeleton.py

### Others ###

* **AssignUISet.py** - by default, in a texture UVSet name is locked in motionbuilder UI, but you can assign a value by using the script
* **ComputeAcceleration.py** - compute translation acceleration, helps to see issues on the motion
* **CopyBlendShapeAnimation.py** - select source model, then destination model, execute the script and it will copy animation FCurves for the same named shapes. 
* **DuplicateUnderTheRoot.py** - helpful for the SpriteSheet Solver sprites preparation, populate models under the root (sprite planes)
* **ExtractTrajectoryPath.py** - make a 3D Curve model from selected model translation trajectory
* **FromLayeredToSingleTexture.py** - helpful for scene, imported from maya. A lot of textures with single layer are represented as FBLayeredTexture. To optimize performance and memory usage, you can use the script to replace layered textures with simple one.
* **GroupedUnderNull.py** - this simular to Maya grouping for selection
* **InsertParent.py** - insert parent for each selected model and keep the same animation trajectory
* **Lights_HardTurnOnOff.py** - script to convert lights into template objects and do a back convertation
* **Lights_TurnOnOff.py** - turn on/off "CastLightOnObject" property
* **MainCar_WindowsSetup.py** - example how to setup ProjTex shader for each car window element 
* **MakeCircle.py** - 4 knots 3d Path model
* **ModelAndLTCDevice.py** - small and test script (nothing interesting)
* **Physics_MakeJointChain.py** - script helps to do a physics setup for joints chain (make nulls, marker capsule for collision and physics joint and connections)
* **RefreshModelsTextures.py** - reload selected models textures
* **RefreshTextures.py** - reload all scene video clips
* **RefreshTextures2015.py** - 
* **RemoveGPUCacheDuplicates.py** - check for GPUCache Shader path and remove dublicates
* **SaveLocalPositions.py** - for all the timeline frames, save selected models local translation into a text file 
* **SelCharAndCR_ByModelSelection.py** - from selected geometry, try to find to which character it's belong to
* **SelectAllCharacterNodes_ByModelSelection.py** - similar to SelCharAndCR_ByModelSelection, but this one script selects all connected resources (textures, materials, constraints, etc.)
* **StoreRelationInConnections.py** - store/retrive connections between relation constraint boxes 
* **Story_ReplaceMediaInTrackClips.py** - XML store/retrieve Story tracks and clips
* **StoryTrackFromSelection.py** - script to create a generic animation story track from selected models

## Lib ##

 Special startup scripts with special logic classes

* **libDuplicateAlongACurve**
 
## Startup ##

 Scripts that have a UI and could be found under Python Tools menu
 
* **ArrayTool** - something like Array Tool in 3dsMax, clone selected models or merge copies from file
* **BatchRender** - render file by file from a specified file list
* [CameraAndGroupLinkingTool](CameraLinkVisTool.md) - for camera linkvis plugin, UI to manage camera and group visibility connection
* **CameraRIGTool** - UI for making an animatable camera RIG
* **CharacterSelectionTool** - from selected models, try to select an affected character
* **CleanUpManager.py** - check for dublicated, unused resources (shaders, materials, textures, video clips)
* **CmdLineRenderingTool.py** - UI to prepare a bat file for a mobu command line rendering
* **CompositionPropertyView.py** - property views for composition toolkit objects
* [CompositionTree.py](CompositionToolkit.md) - UI for Composition Toolkit plugin
* [OffscreenRenderingTool.py](OffscreenRenderingTool.md) - for mo_graphics plugin, UI for making render from camera into a texture and render into cubemap
* **FacialRIG_HelperTool** - this is for 2d into 3d points pipeline, script for manage switch between Stance (live facial retargeting) and Animation (plot on joints)
* **FacialRIG_PrepStarter** - this is for 2d into 3d points pipeline, prepare facial starter scene
* **FacialRIG_Selector** - this is for 2d into 3d points pipeline, UI to select facial rig control elements
* **GoogleStreetViewGrabberTool.py** - needs a pyskybox plugin, script to extract google streetview cubemap
* **LightVisorTool.py** - information about scene lights and affected light lists for each ProjTex shader (local lights)
* **MoPlugsSettingsTool.py** - add more items to main menu
* **Renamer.py** - advanced rename tool, like in 3dsMax
* **StoreRelationInConnections.py** - text file store/retrieve relation boxes connections
* **StoryIOTool.py** - Story XML IO Tool
* [TexturesManager.py](TexturesManagerTool.md) - rellocate textures and video clips, collect them into some specified folder
* [UnitTestsTool.py](UnitTestsTool.md) - run a list of scene files and save stats, do a sceenshot and a video preview
* **ViewportGrabberTool.py** - needs a pyskybox plugin, grabs a skybox from current camera position
 
## Startup_Physics ##

 Tool for a physics simulation
 
* [Ball Toss Tool](ComputeBallTossTool.md) - compute a ball toss trajectory between two keyframes (for example, girl drops up a ball)
* [Wheel Motion Tool](WheelMotionTool.md) - compute wheel rolling and steering rotation by using a root motion and geometry radius
