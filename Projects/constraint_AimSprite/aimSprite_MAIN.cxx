
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: aimSprite_MAIN.cpp
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
FBLibraryDeclare( constraintaimsprite )
{
	FBLibraryRegister(ObjectSpriteTextureProperties)
	FBLibraryRegisterElement(ObjectSpriteTextureProperties);
	FBLibraryRegister(ObjectSpriteProperties)
	FBLibraryRegisterElement(ObjectSpriteProperties);

	FBLibraryRegister( ConstraintAimSprite	);
	FBLibraryRegister( SolverSpriteSheet	);
	FBLibraryRegister( KSpriteSolverAssociation );
}
FBLibraryDeclareEnd;

/************************************************
 *	Library functions.
 ************************************************/
bool FBLibrary::LibInit()		{ return true; }
bool FBLibrary::LibOpen()		{ return true; }
bool FBLibrary::LibReady()		{ return true; }
bool FBLibrary::LibClose()		{ return true; }
bool FBLibrary::LibRelease()	{ return true; }

/**
*	\mainpage	Simple Positional Constraint
*	\section	intro	Introduction
*	The orconstraintposition sample code is a very basic constraint,
*	with no particular functionality for the moment. It shows the different
*	function calls that will be overloaded when developing a constraint,
*	and gives a basic idea of the real-time evaluation and deformation callbacks.
*	\section	walk	Walkthrough
*	Due to the complex nature of constraints, this walkthrough will
*	cover the very basics of a constraint, dealing with all of the non-intuitive
*	functions in the sample code.
*	\subsection	func1	FBCreate()
*	<ol>
*		<li>Add a reference group: "Constrained object";
*		<li>Set as a deformer with a customized UI;
*		<li>Send the animation nodes to NULL;
*		<li>The layout has not been created yet (to NULL);
*	</ol>
*	\subsection	func2	SetupAllAnimationNodes()
*	<ol>
*		<li>If there is a constrained object;
*		<li>Bind the object the deformation;
*		<li>Create the animation nodes (input/output);
*	</ol>
*	\subsection	func3	RemoveAllAnimationNodes()
*	<ol>
*		<li>If there is a constrained object;
*		<li>Removed the deformer bind from the object.
*	</ol>
*	\subsection	func4	AnimationNodeNotify()
*	<ol>
*		<li>Read the data from the input translation & scaling nodes;
*	</ol>
*	\subsection	func5	DeformerNotify()
*	<ol>
*		<li>Calculate the difference between the positions;
*		<li>Add delta to the displacement;
*		<li>Remove 6% of displacement;
*		<li>Change all the vertices to respect the constraint;
*		<li>Copy current position to old position.
*	</ol>
*	\subsection	func6	GetLayout()
*	<ol>
*		<li>Create new ConstraintLayout.
*	</ol>
*/
