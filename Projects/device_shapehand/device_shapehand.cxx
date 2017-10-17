
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: device_shapehand.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>

#ifdef KARCH_ENV_WIN
	#include <windows.h>
#endif

//--- Library declaration
FBLibraryDeclare( deviceshapehand )
{
	FBLibraryRegister( DeviceShapeHand	);
	FBLibraryRegister( DeviceShapeHandLayout	);
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()	{ return true; }
bool FBLibrary::LibOpen()	{ return true; }
bool FBLibrary::LibReady()	{ return true; }
bool FBLibrary::LibClose()	{ return true; }
bool FBLibrary::LibRelease(){ return true; }

/**
*	\mainpage	Simple Skeleton Device
*	\section	intro	Introduction
*	This device serves as an example for any generic motion capture
*	device. This sample illustrates the use of dynamic allocation of
*	animation nodes/sensors. The UI illustrates the use of a spreadsheet.
*
*	\section	desc	Description
*	This tutorial covers a simple skeleton device, which simulates data
*	streaming in from hardware. In this example, a virtual hardware object
*	streams in the data corresponding to clock hands turning.
*	\section	walk	Walkthrough
*	Here are the steps to test the ORDeviceSkeleton.
*		<li>1. Create an instance of the device by dropping it from the asset browser onto the viewer
*		<li>2. Put the device online
*		<li>3. In the Model binding drop down choose "create ..."
*		<li>4. In the Setup tab click on the Characterize button
*		<li>5. Merge in the scene any characterized character like Mia or Aragor from the samples
*		<li>5. In the Character Controls select the input of the drop character as DeviceCharacter
*		<li>6. Go back to the Device, and put it live to see it rotating, animated by global data.
*	\subsection	func1	FBCreate()
*	<ol>
*		<li>Create a new ModelTemplate;
*		<li>Add the model template to the device model template;
*		<li>Assign the icon filename;
*		<li>Set the recording sampling mode;
*	</ol>
*	\subsection	func2	DeviceOperation()
*	<ol>
*		<li>Based on the requested operation, call the corresponding device function;
*		<li>Propagate the call to the parent class and return the result
*	</ol>
*	\subsection	func3	Stop()
*	<ol>
*		<li>Stop the data stream for the device;
*		<li>Close the hardware.
*		<li>Return false (the device is now offline).
*	</ol>
*	\subsection	func4	Start()
*	<ol>
*		<li>Set the network information (normally read from layout);
*		<li>Open hardware;
*		<li>Get the hardware setup;
*		<li>Begin channel set definition;
*		<li>Use the channels found;
*		<li>End the channel set definition;
*		<li>Define model template channel hierarchy;
*		<li>Start streaming data from device;
*		<li>Return true (device is now online).
*	</ol>
*	\subsection	func5	DefineHierarchy()
*	<ol>
*		<li>Build the hierarchy of model templates.
*	</ol>
*	\subsection	func6	Bind()
*	<ol>
*		<li>For all of the channels;
*		<ul>
*			<li>Create the translation and rotation animation nodes;
*			<li>Create and bind the model template for the channel.
*		</ul>
*	</ol>
*	\subsection	func7	UnBind()
*	<ol>
*		<li>Remove the binding and delete all animation nodes.
*	</ol>
*	\subsection	func8	AnimationNodeNotify()
*	<ol>
*		<li>Get data from hardware abstraction;
*		<li>Write data to animation nodes.
*	</ol>
*	\subsection	func9	DeviceIONotify()
*	<ol>
*		<li>For the number of packets waiting to be read;
*		<li>Read data packet (getting timestamp);
*		<li>Record packet(assigned to timestamp).
*	</ol>
*	\subsection	func10	DeviceRecordFrame()
*	<ol>
*		<li>For all the channels that are used;
*		<li>Get the animation node for recording from the node (translation or rotation);
*		<li>Get the data from the hardware abstraction;
*		<li>Based on the recording mode for the device;
*		<ul>
*			<li>Add a key for the data.
*		</ul>
*		<li>Acknowledge the receipt for statistics.
*	</ol>
*/
