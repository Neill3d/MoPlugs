[Back To Main Page](README.md)

[Back To Plugins Page](Plugins.md)

# MoPhysics Solver

## Car Properties object

Gear property
0 - back gear
1 - netral gear
2 - first gear (forward direction with the automatic gearbox)

Tips
* Wheel rotation around Y axis
* Wheel steering around X axis

To Calculate scaling - rule shows 84 cm on scale = 4.0

You can skip assignment for collision meshes for chassis and wheels in case when transformation mesh has a geometry that will be used for collision.

Geometry for static level collisions:
geometry polygons should have the same winding order. Means better not to use loft geometry or extrude geometry which produces triangle strips
better to have rational quads, not too small or not too big in places

Most Important setup steps:
* Setup transformation and collision objects
* Setup logical local axis for a car from the chassis transformation object
* Setup mass X, Y offset for the engine
* Setup global physics scaling in MoPhysics Solver
* Add static collisions and take care to have correct geometry

For high quality simulation you can setup more steps and frames per second in a solver.
