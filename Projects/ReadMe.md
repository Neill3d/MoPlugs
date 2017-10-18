# MoPlugs Projects #

## You should specify environment variables with your mobu version and path value ##

For example, for motionbuilder 2014 the variable name is **ADSK_MOBU_2014_64**

And variable value for a default motionbuilder installation path is this - C:\Program Files\Autodesk\MotionBuilder 2014

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
