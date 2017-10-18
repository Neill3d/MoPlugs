
	SkyBox shader for Autodesk MotionBuilder


TODO:

 - check for context changing (GLSLFX shader could cause a crash in that case)
 - use one global instance of shader for drawing all the skyboxes, panoramas, don't allocate shader per model

 - fix heap corruption on panorama save


LOG

29.07.2017
 * move cubemap into a separate cubemap user object

17.04.2015
 + add mipmaps to skybox cubemaps

13.10.2014
 + expose interface for python usage

10.10.2014
 * some fixes and addons for the SkyBox shader
 + add rendering a cubemap texture

04.09.2014
 + skybox shader now is used for drawing element always in background (for skydome objects in the scene)

25.06.2014

 + first version of a google street view panorama shader