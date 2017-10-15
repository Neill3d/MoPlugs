
/**	\file	orconstraint_template_constraint.cxx
*	Definition of a simple constraint class.
*	Simple constraint function definitions for the FBSimpleConstraint
*	class.
*/

//--- Class declarations
#include "orconstraint_RigidBodies_constraint.h"
#include "..\NewtonPhysicsLibrary\newton_interface.h"
#include "ordevicePhysics_device.h"

//--- Registration defines
#define	ORCONSTRAINTRB__CLASS		ORCONSTRAINTRB__CLASSNAME
#define ORCONSTRAINTRB__NAME		"Rigid Bodies"
#define	ORCONSTRAINTRB__LABEL		"Rigid Bodies"
#define ORCONSTRAINTRB__DESC		"Rigid bodies constraint based on Physics engine"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINTRB__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINTRB__NAME,
								ORCONSTRAINTRB__CLASS,
								ORCONSTRAINTRB__LABEL,
								ORCONSTRAINTRB__DESC,
								FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)


FBPhysicalPropertiesImplementation( ORTestPhysProperties );
FBRegisterPhysicalProperties( "Test Properties",
								ORTestPhysProperties,
								"Test Properties",
								"Test Properties Desc",
								FB_DEFAULT_SDK_ICON );


/************************************************
 *	Creation function.
 ************************************************/
bool ORConstraint_RigidBodies::FBCreate()
{
	/*
	*	1. Create Reference group & elements
	*	2. Set constraint variables (deformer,layout,description)
	*	3. Set pointers to NULL if necessary
	*/
	Deformer	= false;
	HasLayout	= false;
	Description = "Rigid Bodies constraint based on Newton physics engine";
    
	ReferenceGroupAdd( "Rigid Bodies", 100 );
	
	FBPropertyPublish( this, Mass, "Mass", nullptr, nullptr );
	FBPropertyPublish( this, Friction, "Friction", nullptr, nullptr );
	FBPropertyPublish( this, ConvexHullShape, "Convex Hull Shape", nullptr, nullptr );
	FBPropertyPublish( this, Device, "Device", nullptr, nullptr );

	Mass = 5.0;
	Friction = 0.5;
	ConvexHullShape = true;	// box collision or convex hull shape

	Device.SetFilter( ORDevicePhysics::GetInternalClassId() );

	for (int i=0; i<100; ++i)
	{
		mNodes[i].T = nullptr;
		mNodes[i].R = nullptr;
	}

	AssignDevice();

	mDummy_AnimationNode = nullptr;

	return true;
}

bool ORConstraint_RigidBodies::AssignDevice()
{
	if ( Device.GetCount() ) return true;

	// assign existing device or create a new one

	FBDevice *pDevice = nullptr;
	FBScene *pScene = FBSystem().Scene;
	for (int i=0; i<pScene->Devices.GetCount(); ++i)
	{
		if (FBIS( pScene->Devices[i], ORDevicePhysics) )
		{
			pDevice = pScene->Devices[i];
			break;
		}
	}

	if (pDevice != nullptr && (1==FBMessageBox("Physics", "Do you want to assign an existing device?", "Yes", "No")) )
	{
		Device.Add( pDevice );
		return true;
	}
	/*
	// ask to create a new one
	if ( 1 == FBMessageBox( "Physics", "Do you want to create a new device?", "Yes", "No" ) )
	{
		pDevice = new ORDevicePhysics( "Physics Device" );

		if (pDevice)
		{
			Device.Add( pDevice );
			return true;
		}
	}
	*/
	return false;
}

/************************************************
 *	Destruction function.
 ************************************************/
void ORConstraint_RigidBodies::FBDestroy()
{
	/*
	*	Free any user memory associated to constraint
	*/
}


/************************************************
 *	Refrence added notification.
 ************************************************/
bool ORConstraint_RigidBodies::ReferenceAddNotify( int pGroupIndex, FBModel* pModel )
{
	/*
	*	Perform action required when a refrence is added.
	*/

	

	return true;
}


/************************************************
 *	Reference removed notification.
 ************************************************/
bool ORConstraint_RigidBodies::ReferenceRemoveNotify( int pGroupIndex, FBModel* pModel )
{
	/*
	*	Perform action required when a refrence is removed.
	*/

	// TODO: remove all connected physics nodes

	return true;
}


/************************************************
 *	Setup all of the animation nodes.
 ************************************************/

void ORConstraint_RigidBodies::SetupAllAnimationNodes()
{
	/*
	*	- Based on the existence of the references (ReferenceGet() function),
	*	  create the required animation nodes.
	*	- A source will use an Out node, whereas a destination will use
	*	  an In Node.
	*	- If the node is with respect to a deformation, then Bind the reference
	*	  to the deformation notification ( DeformerBind() )
	*/

	int count = ReferenceGetCount(0);
	
	if (count > 0 && Device.GetCount() > 0)
	{
		mNodesCount = count;

		for (int i=0; i<count; ++i)
		{
			FBModel *pModel = ReferenceGet( 0, i );

			mNodes[i].T = AnimationNodeInCreate	( 1+i*2, pModel, ANIMATIONNODE_TYPE_TRANSLATION );
			mNodes[i].R = AnimationNodeInCreate ( 1+i*2+1, pModel, ANIMATIONNODE_TYPE_ROTATION );

			mNodes[i].geometry.Prep(pModel);
		}
	}
}


/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void ORConstraint_RigidBodies::RemoveAllAnimationNodes()
{
	/*
	*	If nodes have been bound to deformations, remove the binding.
	*/
	int count = 100; // mNodesCount;

	for (int i=0; i<count; ++i)
	{
		mNodes[i].body.reset(nullptr);
	}
}


/************************************************
 *	Suggest a snap.
 ************************************************/
void ORConstraint_RigidBodies::SnapSuggested()
{
	/*
	*	Perform any pre-snap operations.
	*/
	
}

void ORConstraint_RigidBodies::SaveState()
{
	for (int i=0; i<mNodesCount; ++i)
	{
		if (mNodes[i].body.get())
			mNodes[i].body->SaveState();
	}
}

void ORConstraint_RigidBodies::RestoreState()
{
	for (int i=0; i<mNodesCount; ++i)
	{
		if (mNodes[i].body.get())
			mNodes[i].body->RestoreState();
	}
}

/************************************************
 *	Suggest a freeze.
 ************************************************/
void ORConstraint_RigidBodies::FreezeSuggested()
{
	/*
	*	Perform any pre-freeze operations
	*/
	/*
	int count = ReferenceGetCount(0);

	for (int i=0; i<count; ++i)
	{
		FBModel *pModel = ReferenceGet(0, i);
		RestoreModelState( pModel );

		FBMatrix m;
		pModel->GetMatrix(m);

		dMatrix matrix;

		for (int ii=0; ii<4; ++ii)
			for (int jj=0; jj<4; ++jj)
				matrix[ii][jj] = (dFloat) m(ii,jj);

		NewtonBodySetMatrix( mNodes[i].body, &matrix[0][0] );
	}

	NewtonInvalidateCache( GetWorldPtr() );
	*/
}


/************************************************
 *	Disable the constraint on pModel.
 ************************************************/
bool ORConstraint_RigidBodies::Disable(FBModel* pModel)
{
	/*
	*	Perform any operations to disable model
	*	Must call FBConstraint::Disable()
	*/
	return FBConstraint::Disable( pModel );
}

/************************************************
 *	Real-Time Engine Evaluation.
 ************************************************/
bool ORConstraint_RigidBodies::AnimationNodeNotify(FBAnimationNode* pConnector,FBEvaluateInfo* pEvaluateInfo,FBConstraintInfo* pConstraintInfo)
{
	/*
	*	Constraint Evaluation
	*	1. Read data from sources
	*	2. Calculate necessary operations.
	*	3. Write output to destinations
	*	Note: Not the deformation operations !
	*/

	if (Active == false) return false;

	ORDevicePhysics *pDevice = (Device.GetCount() ) ? (ORDevicePhysics*) Device[0] : nullptr; 
	
	if (pDevice == nullptr)
	{
		/*
		if (false == AssignDevice())
		{
			//FBMessageBox( "Rigid bodies collection", "You should assign newton device first", "Ok" );
			Active = false;
			return false;
		}
		*/

		for (int i=0; i<mNodesCount; ++i)
		{
			mNodes[i].body.reset(nullptr);
		}

		return false;
	}
	

	//
	// Update animation nodes
	//

	FBMatrix m;

	FBTVector T;
	FBRVector R;
	FBSVector S;

	for (int i=0; i<mNodesCount; ++i)
	{
		if (mNodes[i].body.get() == nullptr) continue;

		m.Set( mNodes[i].body->GetMatrix(pDevice->GetWorldPtr()) );
		FBMatrixToTRS( T, R, S, m );

		mNodes[i].T->WriteData(T, pEvaluateInfo);
		mNodes[i].R->WriteData(R, pEvaluateInfo);
	}

	return true;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool ORConstraint_RigidBodies::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	FBX Storage of constraint parameters.
	*/
	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORConstraint_RigidBodies::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	FBX Retrieval of constraint parameters.
	*/
	return true;
}


/************************************************
 *	Freeze SRT for the constraint.
 ************************************************/
void ORConstraint_RigidBodies::FreezeSRT	( FBModel* pModel, bool pS, bool pR, bool pT )
{
	/*
	*	Freeze the model's SRT parameters (if true,true,true)
	*	Must call FBConstraint::FreezeSRT
	*/
	FBConstraint::FreezeSRT( pModel, pS, pR, pT );
}



void ORConstraint_RigidBodies::EnterOnline()
{

	AssignDevice();
	ORDevicePhysics *pDevice = (Device.GetCount() > 0) ? (ORDevicePhysics*) Device.GetAt(0) : nullptr;

	if (pDevice == nullptr)
		return;

	PHYSICS_INTERFACE::BodyOptions	options;
	options.mass = Mass;
	options.friction = Friction;

	for (int i=0; i<mNodesCount; ++i)
	{
		if (pDevice && mNodes[i].body.get() == nullptr)
		{
			mNodes[i].body.reset( pDevice->CreateNewBody(&options, &mNodes[i].geometry, ConvexHullShape) );
		}

		if (mNodes[i].body.get() )
			mNodes[i].body->Snap();
	}
}

void ORConstraint_RigidBodies::LeaveOnline()
{
}