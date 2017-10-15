========================================================================
    STATIC LIBRARY : NewtonPhysicsLibrary Project Overview
========================================================================

Library is a wrapper around Newton Physics Engine to exchange with the application
using independent virtual interface which is in Physics_common.h


Physics_Common.h

 virtual classes to exchange information between phys engine and application

Newton_PUBLIC.h

 methods to make an instance of newton elements (world, body, car)



/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////


LOG

 14.07.2016
  added a ClosestRayCast method. Use a library in the gpu cache casting

 29.03.2015
  first stable car running. Problem was with collision geometry, polygon winding
  I've reversed the winding order, that seems to fix the problem

 15.03.2015
	making a static library for implementing all functions to simulate rigid bodies and cars using NewtonEngine.
	This time this implemention is based on some pre-defined interface for mobu constraint and it doesn't depend
	on fb sdk types.