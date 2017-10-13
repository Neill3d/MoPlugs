
# MoPlugs Scripts #

For more information, please have a look at wiki pages.
Note! Most of scripts and tools have been developed in MoBu 2014 

## Action ##

 Scripts that have a single execution scenario
 
* CarSim \ ReConnectJoystick.py - the script helps to fix an issue with a missing or stucked joystick device after scene reloading in mobu

### Scripts for the Wall Bricks Constraints and building blocks logic ###

* DuplicateAlongACurve \ ConvertPolySpline.py
* DuplicateAlongACurve \ DeformMeshByCurve.py
* DuplicateAlongACurve \ DeformMeshByCurve2.py
* DuplicateAlongACurve \ DuplicateAlongACurve_DeformMesh.py
* DuplicateAlongACurve \ DuplicateAlongACurve_HardPlot.py
* DuplicateAlongACurve \ DuplicateAlongACurve_RotateBy90.py
* DuplicateAlongACurve \ DuplicateAlongACurve_SelectModels.py
* DuplicateAlongACurve \ DuplicateAlongACurve_SelectNulls.py
* DuplicateAlongACurve \ DuplicateAlongACurve_SETUP.py
* DuplicateAlongACurve \ FixInstanceGeometry.py

### Scripts for work with FaceWare 2d tracked data and my retargeting rig ###
* FacialRIG \ CHECK_FacialData_VideoPlane.py
* FacialRIG \ FacialRIG_connectJoints.py
* FacialRIG \ FacialRIG_CreateVideoPlane.py
* FacialRIG \ FacialRIG_HardPlot.py
* FacialRIG \ FacialRIG_OpticalPostProcessing.py
* FacialRIG \ FacialRIG_SetupConstraints.py
* FacialRIG \ FacialRIG_SetupConstraints2.py
* FacialRIG \ StabilizationScenario.py

### Script to work with a DataGlove device and it's recorded result ###

* ShapeHand \ ShapeHand_CorrParentChildConstraints.py
* ShapeHand \ ShapeHand_CorrRelationConstraint.py
* ShapeHand \ ShapeHand_CreateAnIntermediateHandsChar.py
* ShapeHand \ ShapeHand_printHeirarchy.py
* ShapeHand \ ShapeHand_RelationConnections.py
* ShapeHand \ ShapeHand_Rename_DeviceSet_toBVH.py
* ShapeHand \ ShapeHand_RenameBVH_to_DeviceSet.py
* ShapeHand \ ShapeHand_RenameSkeleton.py

### Others ###

* AssignUISet.py
* ComputeAcceleration.py
* CopyBlendShapeAnimation.py
* DuplicateUnderTheRoot.py
* ExtractTrajectoryPath.py
* FromLayeredToSingleTexture.py
* GroupedUnderNull.py
* InsertParent.py
* Lights_HardTurnOnOff.py
* Lights_TurnOnOff.py
* MainCar_WindowsSetup.py
* MakeCircle.py
* ModelAndLTCDevice.py
* Physics_MakeJointChain.py
* RefreshModelsTextures.py
* RefreshTextures.py
* RefreshTextures2015.py
* RemoveGPUCacheDuplicates.py
* SaveLocalPositions.py
* SelCharAndCR_ByModelSelection.py
* SelectAllCharacterNodes_ByModelSelection.py
* StoreRelationInConnections.py
* Story_ReplaceMediaInTrackClips.py
* StoryTrackFromSelection.py

## Lib ##

 Special startup scripts with special logic classes

* libDuplicateAlongACurve
 
## Startup ##

 Scripts that have a UI and could be found under Python Tools menu
 
* ArrayTool
* BatchRender
* CameraAndGroupLinkingTool
* CameraRIGTool
* CharacterSelectionTool
* **CleanUpManager.py**
* **ExtendedRenderingTool.py**
* FacialRIG_HelperTool
* FacialRIG_PrepStarter
* FacialRIG_Selector
* GoogleStreetViewGrabberTool.py
* MoPlugsSettingsTool.py
* ProjectTool.py
* PropertyViewTracking.py
* Renamer.py
* StoreRelationInConnections.py
* StoryIOTool.py
* **TexturesManager.py**
* UnitTestsTool.py
* ViewportGrabberTool.py
 
## Startup_Physics ##

 Tool for a physics simulation
 
* Ball Toss Tool
* **Wheel Motion Tool** - computing wheel rolling and steering rotation by using a root motion and geometry radius
