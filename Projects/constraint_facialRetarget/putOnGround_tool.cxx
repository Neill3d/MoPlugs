
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: putOnGround.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "putOnGround_tool.h"

//--- Registration defines
#define TOOLPUTONGROUND__CLASS	TOOLPUTONGROUND__CLASSNAME
#define TOOLPUTONGROUND__LABEL	"Put On Ground"
#define TOOLPUTONGROUND__DESC	"Put On Ground Tool"

//--- FiLMBOX implementation and registration
FBToolImplementation(	TOOLPUTONGROUND__CLASS	);
FBRegisterTool		(	TOOLPUTONGROUND__CLASS,
						TOOLPUTONGROUND__LABEL,
						TOOLPUTONGROUND__DESC,
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool Tool_PutOnGround::FBCreate()
{
	// Tool options
	StartSize[0] = 160;
	StartSize[1] = 95;

    int lB = 10;
	//int lS = 4;
	int lW = 100;
	int lH = 18;
	
	// Configure layout

	AddRegion( "LabelInfo", "LabelInfo",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachTop,	"",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	AddRegion( "Container", "Container",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachBottom,"LabelInfo",	1.0,
										-lB,	kFBAttachRight,	"",	1.0,
										2*lH,	kFBAttachNone,	"",	1.0 );

	AddRegion( "ButtonOrient", "ButtonOrient",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachBottom,"Container",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );

	AddRegion( "ButtonProject", "ButtonProject",
										lB,	kFBAttachLeft,	"",	1.0	,
										lB,	kFBAttachBottom,"ButtonOrient",	1.0,
										lW,	kFBAttachNone,	"",	1.0,
										lH,	kFBAttachNone,	"",	1.0 );
	
	//
	SetControl( "LabelInfo", mLabelInfo );
	SetControl( "Container", mContainerGround );
	SetControl( "ButtonOrient", mButtonOrient );
	SetControl( "ButtonProject", mButtonProject );

	// Configure button

	mLabelInfo.Caption = "Drag ground models:";

	mContainerGround.OnDblClick.Add(this, (FBCallback) &Tool_PutOnGround::EventContainerDblClick);
	mContainerGround.OnDragAndDrop.Add(this, (FBCallback) &Tool_PutOnGround::EventContainerDragAndDrop);

	mButtonOrient.Style = kFBCheckbox;
	mButtonOrient.State = 0;
	mButtonOrient.Caption = "Orient models";

	mButtonProject.OnClick.Add( this, (FBCallback) &Tool_PutOnGround::EventButtonProjectClick );
	mButtonProject.Caption = "Project Selected!";

	// Add tool callbacks
	
	
	return true;
}


/************************************************
 *	FiLMBOX Destruction function.
 ************************************************/
void Tool_PutOnGround::FBDestroy()
{
	// Remove tool callbacks
	
	// Free user allocated memory
}


/************************************************
 *	Button click callback.
 ************************************************/

void Tool_PutOnGround::EventContainerDblClick( HISender pSender, HKEvent pEvent )
{
	mContainerGround.Items.Clear();
	
}

void Tool_PutOnGround::EventContainerDragAndDrop( HISender pSender, HKEvent pEvent )
{
	FBEventDragAndDrop levent(pEvent);
	
	switch (levent.State)
	{
	case kFBDragAndDropBegin:
		levent.Accept();
		break;
	case kFBDragAndDropDrag:
		levent.Accept();
		break;
	case kFBDragAndDropDrop:
		for (int i=0; i<levent.GetCount(); ++i)
		{
			if (FBIS(levent.Get(i), FBModel))
				mContainerGround.Items.Add( levent.Get(i)->Name, (kReference) levent.Get(i) );
		}
		break;
	}

}

FBModelNull *MakeNullNode(double *pos, const char *name="test")
{
	FBModelNull *pNewNode = new FBModelNull(name);
	pNewNode->SetVector( FBVector3d(pos) );
	pNewNode->Show = true;

	return pNewNode;
}

void Tool_PutOnGround::ProjectModel( FBModel *pModel, FBModelList *groundModels )
{
	FBVector3d min3, max3;

    pModel->GetBoundingBox(min3, max3);    

	//FBMatrix m, invM;
    //pModel->GetMatrix(m, kModelTransformation);
	//FBMatrixInverse(invM, m);

	FBMatrix m, invM;
	pModel->GetMatrix(m, kModelTransformation);
	pModel->GetMatrix(invM, kModelInverse_Transformation);
    
    FBVector4d min4(min3[0], min3[1], min3[2], 1.0);
    FBVector4d max4(max3[0], max3[1], max3[2], 1.0);

    FBVectorMatrixMult(min4, m, min4);
    FBVectorMatrixMult(max4, m, max4);

    // rays in world space

    FBVector4d rayOrigin(min4[0]+0.5*(max4[0]-min4[0]), min4[1], min4[2]+0.5*(max4[2]-min4[2]), 1.0);
    FBVector4d rayEnd(rayOrigin);
    rayEnd[1] -= 10.0;
    
	//MakeNullNode(rayOrigin, "rayOrigin");

    // convert rays into each ground model space
    double res = 0.0;

	double minDist = -1.0;

	for (int i=0; i<groundModels->GetCount(); ++i)
	{
		FBMatrix groundM, groundInvM;

		FBModel *pGroundModel = groundModels->GetAt(i);

		pGroundModel->GetMatrix(groundM);
		pGroundModel->GetMatrix(groundInvM, kModelInverse_Transformation);
        //FBMatrixInverse(groundInvM, groundM);
        
        FBVector4d modelRayOrigin;
        FBVector4d modelRayEnd;
        
        FBVectorMatrixMult(modelRayOrigin, groundInvM, rayOrigin);
        FBVectorMatrixMult(modelRayEnd, groundInvM, rayEnd);
        
		//MakeNullNode(modelRayOrigin, "modelRayOrigin");

        FBVector4d pos, n;
        
		if (true == pGroundModel->ClosestRayIntersection(modelRayOrigin, modelRayEnd, pos, (FBNormal&)n) )
		{
			FBVectorMatrixMult(pos, groundM, pos);

			double dist = modelRayOrigin[1] - pos[1];

			if (minDist < 0.0)
			{
				minDist = 0.0;
				res = pos[1];
			}
			else if (minDist > dist)
			{
				res = pos[1];
			}
		}
	}   
    
    // convert rays back into world space
    
    // use intersection point to move object to ground !

	if (res != 0.0)
	{
		FBVector3d v;
		pModel->GetVector(v);
		v[1] = res;
		pModel->SetVector(v);
	}

}

void Tool_PutOnGround::EventButtonProjectClick( HISender pSender, HKEvent pEvent )
{
	
	FBModelList	*myList = FBCreateModelList();
	FBModelList *groundList = FBCreateModelList();

    FBGetSelectedModels(*myList);
    
	for (int i=0; i<mContainerGround.Items.GetCount(); ++i)
		groundList->Add( (FBModel*) mContainerGround.Items.GetReferenceAt(i) );

	for (int i=0; i<myList->GetCount(); ++i)

		ProjectModel(myList->GetAt(i), groundList);


	FBDestroyModelList(myList);
	FBDestroyModelList(groundList);
}


