
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: spriteSheet_solver.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "spriteSheet_solver.h"
#include "algorithm\math3d_mobu.h"
#include "IO\tinyxml.h"
#include "IO\FileUtils.h"
#include "StringUtils.h"


//--- Registration defines
#define	SPRITESHEETSOLVER__CLASS	SPRITESHEETSOLVER__CLASSNAME
#define SPRITESHEETSOLVER__NAME		"SolverSpriteSheet"
#define SPRITESHEETSOLVER__LABEL	"SolverSpriteSheet"
#define SPRITESHEETSOLVER__DESC		"SolverSpriteSheet"

//--- implementation and registration

FBConstraintSolverImplementation (	SPRITESHEETSOLVER__CLASS		);
FBRegisterConstraintSolver		(	SPRITESHEETSOLVER__NAME,
									SPRITESHEETSOLVER__CLASS,
									SPRITESHEETSOLVER__LABEL,
									SPRITESHEETSOLVER__DESC,
									"character_solver.png"			);	// Icon filename (default=Open Reality icon)


FBApplyManagerRuleImplementation(KSpriteSolverAssociation);
FBRegisterApplyManagerRule( KSpriteSolverAssociation, "KSpriteSolverAssociation", "Apply Manager Rule for Sprite Sheet Solver");

///////////////////
//

#define		TEXTURE_PROPERTY_TYPE		"SpriteSheet Type"
#define		TEXTURE_PROPERTY_ROWS		"SpriteSheet Rows"
#define		TEXTURE_PROPERTY_COLS		"SpriteSheet Cols"

#define		MODEL_PROPERTY_FPS					"SpriteSheet FPS"
#define		MODEL_PROPERTY_LOCALPLAY			"SpriteSheet LocalPlay"			// property is out of date, use one in solver
#define		MODEL_PROPERTY_PLAYOFFSET			"SpriteSheet PlayOffset"
#define		MODEL_PROPERTY_LOOPPLAY				"SpriteSheet LoopPlay"
#define		MODEL_PROPERTY_RANGECOUNT			"SpriteSheet RangeCount"

#define		MODEL_PROPERTY_STARTFRAME			"SpriteSheet StartFrame"
#define		MODEL_PROPERTY_STOPFRAME			"SpriteSheet StopFrame"
#define		MODEL_PROPERTY_SPEED				"SpriteSheet Speed"

#define		MODEL_PROPERTY_USETRIGGER			"SpriteSheet UseTrigger"
#define		MODEL_PROPERTY_TRIGGER				"SpriteSheet Trigger"
#define		MODEL_PROPERTY_TRIGGERONSCALING		"SpriteSheet TriggerOnScaling"
#define		MODEL_PROPERTY_TRIGGERDELAY			"SpriteSheet TriggerDelay"

#define		MODEL_PROPERTY_TRIGGERCHANGEMODE	"SpriteSheet TriggerChangeMode"
#define		MODEL_PROPERTY_TRIGGERINDEX			"SpriteSheet TriggerIndex"
#define		MODEL_PROPERTY_TRIGGERRANDOMMODE	"SpriteSheet TriggerRandomMode"
// output properties
#define		MODEL_PROPERTY_TRANSLATION			"SpriteSheet Translation"
#define		MODEL_PROPERTY_SCALING				"SpriteSheet Scaling"
// debug property
#define		MODEL_PROPERTY_INDEX				"SpriteSheet Index"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//

bool KSpriteSolverAssociation::IsValidSrc( FBComponent *pSrc)
{
	return FBIS(pSrc, ObjectSpriteTextureProperties) || FBIS(pSrc, ObjectSpriteProperties);
}

bool KSpriteSolverAssociation::IsValidConnection( FBComponent* pSrc, FBComponent*& pDst, bool& pAllowMultiple)
{
	pAllowMultiple = false;
	if (FBIS(pSrc, ObjectSpriteProperties) && FBIS(pDst, FBModel) )
		return true;

	if (FBIS(pSrc, ObjectSpriteTextureProperties) && FBIS(pDst, FBTexture) )
		return true;

	pAllowMultiple = true;
	return FBIS(pDst, SolverSpriteSheet);		// add src into the solver components !!
}

bool KSpriteSolverAssociation::MenuBuild( FBAMMenu* pAMMenu, FBComponent* pFocusedObject)
{
	mAddTextureProps  = -1;
	mAddModelProps = -1;
	mAddModelEvaluation = -1;
	mRemoveModelEvaluation = -1;

	if(pFocusedObject)
	{
		if( FBIS(pFocusedObject, FBTexture) )
		{
			pAMMenu->AddOption( "" );
			mAddTextureProps = pAMMenu->AddOption("Add Sprite Sheet Properties", -1, true);
			
			pAMMenu->AddOption( "" );
			return true;
		}
		else if (FBIS(pFocusedObject, FBModel) )
		{
			pAMMenu->AddOption( "" );
			mAddModelProps = pAMMenu->AddOption( "Add Sprite Sheet Properties", -1, true);

			mAddModelEvaluation = pAMMenu->AddOption( "Register/UnRegister Model (Sprite Sheet Solver)", -1, true);
			//mRemoveModelEvaluation = pAMMenu->AddOption( "UnRegister Model (Sprite Sheet Solver)", -1, true);

			pAMMenu->AddOption( "" );
			return true;
		}
	}
	return false;
}

bool KSpriteSolverAssociation::MenuAction( int pMenuId, FBComponent* pFocusedObject)
{
	bool result = false;

	if (mAddTextureProps >= 0 && pMenuId == mAddTextureProps)
	{
		// DONE: add properties like number of rows, cols, sprite sheet type, etc.

		SolverSpriteSheet::AddTextureProperties( (FBTexture*)pFocusedObject );

	}
	else if (mAddModelProps >= 0 && pMenuId == mAddModelProps)
	{
		// DONE: add properties like local play, start frame, stop frame, trigger, etc.
		SolverSpriteSheet::AddModelProperties( (FBModel*) pFocusedObject );
	}
	else if (mAddModelEvaluation >= 0 && pMenuId == mAddModelEvaluation)
	{
		// DONE: automaticaly add properties to texture and model

		// try to find if model is already in some solver
		SolverSpriteSheet *pSolver = nullptr;
		FBScene *pScene = mSystem.Scene;
		for (int i=0, count=pScene->ConstraintSolvers.GetCount(); i<count; ++i)
		{	
			if (FBIS(pScene->ConstraintSolvers[i], SolverSpriteSheet) )
			{
				int index = pScene->ConstraintSolvers[i]->Components.Find(pFocusedObject);
				if (index >= 0)
				{
					pSolver = (SolverSpriteSheet*) pScene->ConstraintSolvers[i];
					break;
				}
			}
		}

		if (pSolver)
		{
			// model is already in the solver, let's do Unregister operation
			bool activeState = pSolver->Active;
			pSolver->Active = false;
			pSolver->Components.Remove(pFocusedObject);
			pSolver->Active = activeState;
		}
		else
		{
			// register model for the first model in the scene solvers
			FBModel *pModel = (FBModel*) pFocusedObject;
			SolverSpriteSheet::AddModelProperties( pModel );

			for (int i=0, count=pModel->Materials.GetCount(); i<count; ++i)
			{
				FBMaterial *pMaterial = pModel->Materials[i];
				FBTexture *pTexture = pMaterial->GetTexture();

				if (pTexture)
				{
					SolverSpriteSheet::AddTextureProperties( pTexture );
				}
			}


			for (int i=0, count=pScene->ConstraintSolvers.GetCount(); i<count; ++i)
			{	
				if (FBIS(pScene->ConstraintSolvers[i], SolverSpriteSheet) )
				{
					pSolver = (SolverSpriteSheet*) pScene->ConstraintSolvers[i];
					
					bool activeState = pSolver->Active;
					pSolver->Active = false;
					pSolver->Components.Add(pFocusedObject);
					pSolver->Active = activeState;
				}
			}

			if (pSolver == nullptr)
			{
				pSolver = (SolverSpriteSheet*) FBCreateObject( "Browsing/Templates/Solvers", "SolverSpriteSheet", "Sprite Sheet Solver" );

				if (pSolver)
				{
					pSolver->Components.Add(pFocusedObject);
				}
			}
		}
	}

	return result;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

/************************************************
 *	Creation function.
 ************************************************/

void SolverSpriteSheet::ActionManualReset(HIObject pObject, bool value)
{
	SolverSpriteSheet *p = FBCast<SolverSpriteSheet>(pObject);
	if (p && value)
	{
		p->OnManualResetClick();
	}
}

bool SolverSpriteSheet::FBCreate()
{
	Live = true;
	SyncAnimationPlay = false;
	SampleRecordingPrecision = 100;

	

	//
	FBPropertyPublish( this, LocalPlay, "Local Play", nullptr, nullptr );
	FBPropertyPublish( this, SpriteNodes, "Sprite Nodes Count", nullptr, nullptr );
	FBPropertyPublish( this, ResetTime, "Reset Time", nullptr, nullptr );
	FBPropertyPublish( this, ManualReset, "Manual Reset", nullptr, ActionManualReset );
	FBPropertyPublish( this, ZeroNonPlaying, "Zero Non Playing", nullptr, nullptr );
	FBPropertyPublish( this, UseLocalCache, "Use Local Cache", nullptr, nullptr );
	FBPropertyPublish( this, TriggerOnScalingThreshold, "Trigger OnScaling Threshold", nullptr, nullptr );

	LocalPlay = false;
	SpriteNodes = 0;
	SpriteNodes.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	ResetTime = FBTime(0);
	ZeroNonPlaying = false;
	UseLocalCache = true;
	TriggerOnScalingThreshold = 0.01;

	return true;
}


/************************************************
 *	Destruction function.
 ************************************************/
void SolverSpriteSheet::FBDestroy()
{
	ClearNodes();
	ClearSpriteInfoMap();
}

void SolverSpriteSheet::EmptyNode( SpriteSheetNode &node )
{
	node.inRows = nullptr;
	node.inCols = nullptr;
	node.inType = nullptr;

	node.inFPS = nullptr;
	node.inSpeed = nullptr;
	node.inLocalPlay = nullptr;
	node.inPlayOffset = nullptr;
	node.inLoopPlay = nullptr;

	for (int i=0; i<MAX_NUMBER_OF_FRAME_RANGE; ++i)
	{
		node.inFrameRange[i].inStartFrame = nullptr;
		node.inFrameRange[i].inStopFrame = nullptr;
	}

	node.inTrigger = nullptr;
	node.inTriggerOnScaling = nullptr;
	node.inTriggerDelay = nullptr;

	node.inTriggerChangeMode = nullptr;
	node.inTriggerIndex = nullptr;
	node.inTriggerRandomMode = nullptr;

	node.inXmlData = nullptr;

	node.modelScaling = nullptr;
	node.outScaling = nullptr;
	node.outTranslation = nullptr;
	node.outVisibility = nullptr;

	node.needReset = true;
	node.lastFrame = 0.0;
	node.lastTime = FBTime::MinusInfinity;
	node.lastTrigger = 0.0;
	node.lastModelScaling = 1.0;
	node.triggerTime = FBTime::MinusInfinity;
	node.offsetTime = FBTime(0);
	node.cachedFrame = FBTime::MinusInfinity;

	node.debugTexture = nullptr;
	node.inIndex = nullptr;
}

void SolverSpriteSheet::ComputeLocalOffsetTime( SpriteSheetNode &node )
{
	FBTime currTime = mSystem.LocalTime;
	FBTime resetTime = ResetTime;
	
	bool isTriggerAnimated = false;
	double trigger=0.0;

	if (node.inTriggerOnScaling)
	{
		node.inTriggerOnScaling->GetData( &isTriggerAnimated, sizeof(double) );
	}

	if (isTriggerAnimated == false && node.inTrigger)
	{
		node.inTrigger->ReadData( &trigger, currTime );
		isTriggerAnimated = node.inTrigger->KeyCount > 0;

		if (trigger > 0.0)
		{
			node.offsetTime = currTime;
		}
		else if (node.inPlayOffset)
		{
			double value=0.0;
			node.inPlayOffset->GetData(&value, sizeof(double) );

			FBTime triggerTime(resetTime);

			if (isTriggerAnimated)
			{
				int keyCount = node.inTrigger->FCurve->Keys.GetCount();
				for (int i=0; i<keyCount; ++i)
				{
					FBTime keyTime = node.inTrigger->FCurve->Keys[i].Time;

					if (keyTime > triggerTime)
					{
						triggerTime = keyTime;
						break;
					}
				}
			}

			node.offsetTime = triggerTime + FBTime(0,0,0,value);
		}
	}	
}

void SolverSpriteSheet::ResetNode( SpriteSheetNode &node )
{
	FBTime currTime = mSystem.LocalTime;
	FBTime resetTime = ResetTime;

	node.lastFrame = 0.0;
	node.lastTime = currTime;
	node.lastTrigger = 0.0;
	node.lastModelScaling = 1.0;
	node.triggerTime = FBTime::MinusInfinity;
	node.offsetTime = FBTime::MinusInfinity;
	node.cachedFrame = FBTime::MinusInfinity;

	double trigger=0.0;
	bool triggerOnScaling = false;
	double triggerDelay = 0.0;
	double modelScaling[3] = {1.0, 1.0, 1.0};
	
	bool isTriggerAnimated = false;

	if (node.inTrigger)
	{
		node.inTrigger->ReadData( &trigger, currTime );
		isTriggerAnimated = node.inTrigger->KeyCount > 0;
	}
	
	if (node.inTriggerOnScaling)
		node.inTriggerOnScaling->GetData( &triggerOnScaling, sizeof(bool) );
	if (node.inTriggerDelay)
		node.inTriggerDelay->GetData( &triggerDelay, sizeof(double) );

	if (triggerOnScaling && node.modelScaling)
	{
		if (false == node.modelScaling->ReadData( &modelScaling[0], currTime ) )
			triggerOnScaling = false;
	}

	double curModelScaling = 0.0;
	if (triggerOnScaling)
	{
		curModelScaling = modelScaling[0] + modelScaling[1] + modelScaling[2];
		if (curModelScaling < 0.01)
			curModelScaling = 0.0;

		if (node.lastModelScaling == 0.0 && curModelScaling > 0.0)
			triggerOnScaling = true;
		else
			triggerOnScaling = false;
	}

	node.lastModelScaling = curModelScaling;
	trigger = trigger + (triggerOnScaling) ? 1.0 : 0.0;

	node.lastTrigger = trigger;
	node.lastTime = currTime;

	ComputeLocalOffsetTime(node);
}

void SolverSpriteSheet::prepProp (FBComponent *pComp, const char *propName, double defValue, double minValue, double maxValue, FBProperty *& outNode, bool typeBool) 
{
	outNode = pComp->PropertyList.Find( propName );
	if (outNode == nullptr)
	{
		if (typeBool)
		{
			outNode= pComp->PropertyCreate(propName, kFBPT_bool, "Bool", false );
		}
		else
		{
			outNode = pComp->PropertyCreate(propName, kFBPT_double, "Number", false);
		}

		if (typeBool)
		{
			bool bValue = (defValue > 0.0);
			outNode->SetData( &bValue );
		}
		else
		{
			outNode->SetData( &defValue );
		}

		if (minValue != 0.0 && maxValue != 0.0)
		{
			outNode->SetMinMax(minValue, maxValue, true, true);
		}
	}
};

void SolverSpriteSheet::prepAnimProp (FBComponent *pComp, const char *propName, double defValue, double minValue, double maxValue, FBAnimationNode *& outNode) 
{
	FBPropertyAnimatable *pAnimProp = (FBPropertyAnimatable*) pComp->PropertyList.Find( propName );
	if (pAnimProp == nullptr)
	{
		pAnimProp = (FBPropertyAnimatable*) pComp->PropertyCreate(propName, kFBPT_double, "Number", true);
		pAnimProp->SetAnimated(true);
		pAnimProp->SetData( &defValue );

		if (minValue != 0.0 && maxValue != 0.0)
		{
			pAnimProp->SetMinMax(minValue, maxValue, true, true);
		}
	}
	else if ( false == pAnimProp->IsAnimated() )
	{
		pAnimProp->SetAnimated(true);
	}
				
	outNode = pAnimProp->GetAnimationNode();
};

void SolverSpriteSheet::AddTextureProperties(FBTexture *pTexture, SpriteSheetNode &node)
{
	prepProp(pTexture, TEXTURE_PROPERTY_TYPE, 1.0, 0.0, 1.0, node.inType, false);
	//next one only for basic texture type
	prepProp(pTexture, TEXTURE_PROPERTY_ROWS, 4.0, 1.0, 128.0, node.inRows, false);
	prepProp(pTexture, TEXTURE_PROPERTY_COLS, 4.0, 1.0, 128.0, node.inCols, false);
}

void SolverSpriteSheet::AddTextureProperties(FBTexture *pTexture)
{
	SpriteSheetNode node;
	AddTextureProperties(pTexture, node);
}


void SolverSpriteSheet::AddModelProperties(FBModel *pModel, const int rangeCount, SpriteSheetNode &node)
{
	prepProp(pModel, MODEL_PROPERTY_FPS, 12.0, 1.0, 360.0, node.inFPS, false);
			
	prepProp(pModel, MODEL_PROPERTY_LOCALPLAY, 0.0, 0.0, 1.0, node.inLocalPlay, true);
	prepProp(pModel, MODEL_PROPERTY_PLAYOFFSET, 0.0, 0.0, 0.0, node.inPlayOffset, false);
	prepProp(pModel, MODEL_PROPERTY_LOOPPLAY, 1.0, 0.0, 1.0, node.inLoopPlay, true);

	prepProp(pModel, MODEL_PROPERTY_RANGECOUNT, (double) rangeCount, 1.0, (double) MAX_NUMBER_OF_FRAME_RANGE, node.inRangeCount, false);

	char buffer[64];
	for (int theRange=0; theRange<rangeCount; ++theRange)
	{
		memset(buffer, 0, sizeof(char)*64);
		sprintf_s( buffer, sizeof(char)*64, MODEL_PROPERTY_STARTFRAME"%d", theRange );
		prepProp(pModel, buffer, 0.0, 0.0, 0.0, node.inFrameRange[theRange].inStartFrame, false);
				
		memset(buffer, 0, sizeof(char)*64);
		sprintf_s( buffer, sizeof(char)*64, MODEL_PROPERTY_STOPFRAME"%d", theRange );
		prepProp(pModel, buffer, 0.0, 0.0, 0.0, node.inFrameRange[theRange].inStopFrame, false);
	}

	prepProp(pModel, MODEL_PROPERTY_SPEED, 1.0, 0.0, 100.0, node.inSpeed, false);

	// triggers

	prepProp(pModel, MODEL_PROPERTY_USETRIGGER, 0.0, 0.0, 1.0, node.inUseTrigger, true);

	prepAnimProp(pModel, MODEL_PROPERTY_TRIGGER, 0.0, 0.0, 1.0, node.inTrigger);
	prepProp(pModel, MODEL_PROPERTY_TRIGGERONSCALING, 0.0, 0.0, 1.0, node.inTriggerOnScaling, true);
	prepProp(pModel, MODEL_PROPERTY_TRIGGERDELAY, 0.0, 0.0, 0.0, node.inTriggerDelay, false);

	prepProp(pModel, MODEL_PROPERTY_TRIGGERCHANGEMODE, 0.0, 0.0, 1.0, node.inTriggerChangeMode, true);
	prepProp(pModel, MODEL_PROPERTY_TRIGGERINDEX, 0.0, 0.0, 2.0, node.inTriggerIndex, false);
	prepProp(pModel, MODEL_PROPERTY_TRIGGERRANDOMMODE, 0.0, 0.0, 1.0, node.inTriggerRandomMode, true);

	prepProp(pModel, MODEL_PROPERTY_INDEX, 1.0, 0.0, 0.0, node.inIndex, false);
}

void SolverSpriteSheet::AddModelProperties(FBModel *pModel)
{
	SpriteSheetNode node;
	AddModelProperties(pModel, DEFAULT_NUMBER_OF_FRAME_RANGE, node); 
}

void SolverSpriteSheet::ReadSpriteSheetInfo( FBTexture *spriteTexture, SpriteSheetInfo *&newInfo )
{
	auto textureIter = mSpriteInfoMap.find(spriteTexture);
	if (textureIter != end(mSpriteInfoMap))
	{
		newInfo = textureIter->second;
	}
	else
	{
		double rows = 1.0;
		double cols = 1.0;

		if (nullptr != spriteTexture->PropertyList.Find(TEXTURE_PROPERTY_ROWS) )
			spriteTexture->PropertyList.Find(TEXTURE_PROPERTY_ROWS)->GetData( &rows, sizeof(double) );
		if (nullptr != spriteTexture->PropertyList.Find(TEXTURE_PROPERTY_COLS) )
			spriteTexture->PropertyList.Find(TEXTURE_PROPERTY_COLS)->GetData( &cols, sizeof(double) );

		int rowsInt = (int) floor(rows);
		if (rowsInt <= 0) rowsInt = 1;
		int colsInt = (int) floor(cols);
		if (colsInt <= 0) colsInt = 1;

		double inType = 0.0;
		if (nullptr != spriteTexture->PropertyList.Find(TEXTURE_PROPERTY_TYPE))
			spriteTexture->PropertyList.Find(TEXTURE_PROPERTY_TYPE)->GetData( &inType, sizeof(double) );
		int inTypeInt = (int) floor(inType);

		newInfo = new SpriteSheetInfo;

		FBString filename("");

		FBPropertyString *pPropString = (FBPropertyString*) spriteTexture->PropertyList.Find("SpriteSheet XMLData");
		if (pPropString)
		{
			filename = pPropString->AsString();
			inTypeInt = 1;
		}
		else
		{
			FBVideoClip *pVideo = (FBVideoClip*) (FBVideo*) spriteTexture->Video;
			if (pVideo)
			{
				filename = pVideo->Filename;
				ChangeFileExt(filename, FBString(".xml") );
						
				if (IsFileExists(filename) )
					inTypeInt = 1;
				else inTypeInt = 0;
			}
		}

		// if no xml filename, then switch to basic mode
		if (filename == "")
			inTypeInt = 0;

		switch(inTypeInt)
		{
		case 1:
			ReadXmlTextureInformation( filename, newInfo );
			break;
		default:
			ComputeBasicTextureInformation( rowsInt, colsInt, newInfo );
		}

		mSpriteInfoMap[spriteTexture] = newInfo;
	}
}

void SolverSpriteSheet::ActiveChanged()
{
	// DONE: add model input and output properties

	if (Active)
	{
		mNodeToSpriteInfoMap.clear();
		ClearSpriteInfoMap();

		mDummyNode = nullptr;

		//const double defValue[3] = {0.0, 0.0, 0.0};
		const int count = Components.GetCount();

		mNodes.resize(count);

		for (int i=0; i<count; ++i)
		{
			SpriteSheetNode &node = mNodes[i];
			EmptyNode(node);
			node.index = i;
			FBModel *pModel = (FBModel*) Components[i];
			/*
			if (mDummyNode == nullptr)
			{
				mDummyNode = AnimationNodeInCreate(0, pModel, ANIMATIONNODE_TYPE_TRANSLATION );
			}
			*/
			node.modelScaling = AnimationNodeOutCreate(i*4+4, pModel, ANIMATIONNODE_TYPE_SCALING);
			node.modelScaling->SetBufferType(true);
			if (node.modelScaling == nullptr)
			{
				FBTrace( "empty model scaling\n" );
			}


			FBPropertyAnimatable *pAnimProp = nullptr;


			//
			// INPUT TEXTURE

			FBTexture *spriteTexture = nullptr;

			for (int theMat=0, matCount=pModel->Materials.GetCount(); theMat<matCount; ++theMat)
			{
				FBMaterial *pMaterial = pModel->Materials[theMat];

				FBTexture *pTexture = pMaterial->GetTexture();
				if (pTexture)
				{
					AddTextureProperties(pTexture, node);

					node.debugTexture = pTexture;
					spriteTexture = pTexture;
				}
			}

			if (spriteTexture == nullptr)
			{
				continue;	// skip processing the model if there is no diffuse texture assigned  (means no any sprite)
			}

			// DONE: pre cache texture
			SpriteSheetInfo *newInfo = nullptr;
			ReadSpriteSheetInfo( spriteTexture, newInfo );

			//
			// INPUT MODEL

			int rangeCount = (newInfo->ranges.size() > DEFAULT_NUMBER_OF_FRAME_RANGE) ? (int)newInfo->ranges.size() : DEFAULT_NUMBER_OF_FRAME_RANGE;

			AddModelProperties(pModel, rangeCount, node);

			for (int theRange=0; theRange<rangeCount; ++theRange)
			{
				if (newInfo != nullptr && newInfo->ranges.size() > theRange && theRange > 0)
				{
					double frameIndex = (double) newInfo->ranges[theRange];
					node.inFrameRange[theRange].inStartFrame->SetData( &frameIndex );
					node.inFrameRange[theRange-1].inStopFrame->SetData( &frameIndex );
				}
			}

			//
			// OUTPUT MODEL

			pAnimProp = (FBPropertyAnimatable*) pModel->PropertyList.Find(MODEL_PROPERTY_TRANSLATION);
		
			if (pAnimProp == nullptr)
				pAnimProp = (FBPropertyAnimatable*) pModel->PropertyCreate(MODEL_PROPERTY_TRANSLATION, kFBPT_Vector3D, "Vector3D", true );

			pAnimProp->SetAnimated(true);
			node.outTranslation = AnimationNodeInCreate(i*4+1, pAnimProp);
			
			//
			pAnimProp = (FBPropertyAnimatable*) pModel->PropertyList.Find(MODEL_PROPERTY_SCALING);
		
			if (pAnimProp == nullptr)
				pAnimProp = (FBPropertyAnimatable*) pModel->PropertyCreate(MODEL_PROPERTY_SCALING, kFBPT_Vector3D, "Vector3D", true );

			pAnimProp->SetAnimated(true);
			node.outScaling = AnimationNodeInCreate(i*4+2, pAnimProp);

			//pModel->Visibility.SetAnimated(true);
			//node.outVisibility = AnimationNodeInCreate( i*4+3, &pModel->Visibility ); 

			if (node.outTranslation)
				mNodeToSpriteInfoMap[node.outTranslation] = &node;
			if (node.outScaling)
				mNodeToSpriteInfoMap[node.outScaling] = &node;
			if (node.outVisibility)
				mNodeToSpriteInfoMap[node.outVisibility] = &node;
		}

		// update UI

		SpriteNodes = (int) mNodes.size();
	}
	else
	{
		// GOES OFFLINE
		ClearNodes();
		mNodeToSpriteInfoMap.clear();
		mNodes.clear();
		ClearSpriteInfoMap();
		
		RemoveAllAnimationNodes();

		//mSystem.OnUIIdle.Add( this, (FBCallback) &SolverSpriteSheet::OnSystemIdle2 );

		for (int i=0, count=Components.GetCount(); i<count; ++i)
		{
			FBModel *pModel = (FBModel*) Components[i];

			FBPropertyAnimatable *pAnimProp = (FBPropertyAnimatable*) pModel->PropertyList.Find(MODEL_PROPERTY_SCALING);
			if (pAnimProp)
			{
				double defValue[3] = {1.0, 1.0, 1.0};
				pAnimProp->SetData( defValue );
			}
		}
	}
}

bool SolverSpriteSheet::ComputeTextureMatrix(	double *outTranslation, 
												double *outScaling, 
												FBTime &lastTime,
												FBTime &localOffsetTime,
												SpriteSheetNode &node,
												double		&lastFrame,
												double		&lastTrigger,
												double		&vis,
												const FBTime &systemTime,
												const SpriteSheetInfo *pSpriteInfo,
												const double inFPS,
												const bool	localPlay,
												const double playOffset,
												const double inStartFrame,
												const double inStopFrame,
												const double inTrigger,
												const bool inLoopPlay,
												const double inSpeed )
{

	if (pSpriteInfo == nullptr)
		return false;

	FBSVector scl(1.0, 1.0, 1.0);
	FBVector3d tr(0.0, 0.0, 0.0);

	const FBTime	timeLimit(0,0, 1);

	int fps = (int) floor(inFPS);
	int startFrame = (int) floor(inStartFrame);
	int stopFrame = (int) floor(inStopFrame);
	int numberOfFrames = (int) pSpriteInfo->frames.size();

	if (startFrame < 0) startFrame = 0;
	if (startFrame >= numberOfFrames)
		startFrame = numberOfFrames-1;
	if (stopFrame < 0) stopFrame = 0;
	if (stopFrame >= numberOfFrames)
		stopFrame = numberOfFrames-1;

	if (stopFrame == 0)
		stopFrame = numberOfFrames-1;
	else if (stopFrame < startFrame)
		stopFrame = startFrame;

	int localFrameCount = stopFrame - startFrame;

	if (localPlay)
	{
		// offset time should be in time slider local time
		FBTime currTime = systemTime;
		
		// TODO !!!!
		if (lastTime == FBTime::MinusInfinity
				//|| lastTime - currTime > timeLimit
				|| (inTrigger > 0.001 && lastTrigger < 0.001 && lastTrigger >= 0.0))
		{
			lastTime = currTime;
			lastFrame = startFrame;
			lastTrigger = inTrigger;

			// TODO: pass evaluation node for triggerOnScaling
			ComputeLocalOffsetTime( node );
			//localOffsetTime = currTime + FBTime(0,0,0, playOffset); // trigger is a local offset time now
		}

		if (localOffsetTime == FBTime::MinusInfinity)
		{
			lastTime = currTime;
			lastFrame = startFrame;
			lastTrigger = inTrigger;
			//localOffsetTime = currTime + FBTime(0,0,0, playOffset);
			ComputeLocalOffsetTime( node );
		}
		else
		{
			double timeElapsed = inSpeed * (currTime.GetSecondDouble() - localOffsetTime.GetSecondDouble());
			double framesElapsed = timeElapsed * fps;

			if (false == inLoopPlay)
			{
				if (framesElapsed < 0.0) framesElapsed = 0.0;
				else if (framesElapsed >= localFrameCount) framesElapsed = localFrameCount-1;
			}
			else
			{
				if (framesElapsed > 0.0)
				{
					while (framesElapsed >= localFrameCount)
						framesElapsed -= localFrameCount;
				}
				else if (framesElapsed < 0.0)
				{
					while (framesElapsed < 0.0)
						framesElapsed += localFrameCount;
				}
			}
		
			lastFrame = framesElapsed + startFrame;
		}
	}
	else
	{
		if (numberOfFrames > 1)
		{
			FBTime currTime = systemTime;

			if (lastTime == FBTime::MinusInfinity
				|| lastTime - currTime > timeLimit
				|| (inTrigger > 0.001 && lastTrigger < 0.001 && lastTrigger >= 0.0))
			{
				lastTime = currTime;
				lastFrame = startFrame;
				lastTrigger = inTrigger;
			}

			double timeElapsed = inSpeed * (currTime.GetSecondDouble() - lastTime.GetSecondDouble());
			double framesElapsed = timeElapsed * fps;
		
			//
			lastFrame += framesElapsed;

			if (inLoopPlay)
			{
				while (lastFrame >= stopFrame)
					lastFrame -= (stopFrame - startFrame);
			}
		
			if (lastFrame > stopFrame)
				lastFrame = stopFrame;
		}	
	}
	
	// convert frame index into paramU, paramV

	int nframe = (int) floor(lastFrame);
	
	vis = (nframe <= 0 || nframe >= localFrameCount-1) ? 0.0: 1.0;

	if (nframe < 0) nframe = 0;
	if (nframe >= numberOfFrames) nframe = numberOfFrames-1;

	const SpriteFrame &frameInfo = pSpriteInfo->frames[nframe];

	tr[0] = frameInfo.x;
	tr[1] = frameInfo.y;

	scl[0] = frameInfo.w;
	scl[1] = frameInfo.h;
	
	
	//

	outTranslation[0] = tr[0];
	outTranslation[1] = tr[1];
	outTranslation[2] = tr[2];

	outScaling[0] = scl[0];
	outScaling[1] = scl[1];
	outScaling[2] = scl[2];

	lastTrigger = inTrigger;
	lastTime = systemTime;

	return true;
}

bool SolverSpriteSheet::ComputeSpriteSheetNode( FBEvaluateInfo* pEvaluateInfo, SpriteSheetNode *iter,
	const bool localPlay, const bool useLocalCache, const bool zeroNonPlaying, FBTime &resetTime, 
	const double triggerOnScaleThreshold,
	double *outScl, double *outPos )
{
	FBTime systemTime = pEvaluateInfo->GetSystemTime();
	FBTime localTime = pEvaluateInfo->GetLocalTime();

	double modelScaling[3] = {1.0, 1.0, 1.0};
	/*
	bool localPlay = false;
	if (iter->inLocalPlay)
		iter->inLocalPlay->GetData( &localPlay, sizeof(bool), pEvaluateInfo );
		*/
	if (localPlay && useLocalCache && iter->cachedFrame == localTime)
	{
		memcpy( outScl, iter->cachedScl, sizeof(double)*3 );
		memcpy( outPos, iter->cachedPos, sizeof(double)*3 );

		// TODO: missing visibility write data !

		return true;
	}


	// current map key is a texture
	if (iter->debugTexture == nullptr)
	{
		return false;
	}

	//if (pLastTexture == nullptr || pLastTexture != iter->debugTexture)
	//{
//		FBTexture *pLastTexture = iter->debugTexture;
		auto infoIter = mSpriteInfoMap.find(iter->debugTexture);
	//}

	if (infoIter == end(mSpriteInfoMap))
		return false;

	int dstCount = iter->outTranslation->GetDstCount();
	if (dstCount == 0)
		return false;

	double fps = 12.0;
	double firstFrame = 0.0;
	double lastFrame = 0.0;
	double playOffset = 0.0;
	bool loopPlay = false;
	double speed = 1.0;
	double rangeCount = (double) DEFAULT_NUMBER_OF_FRAME_RANGE;
	double rangeIndex = 0.0;
	double trigger = 0.0;
	double rawTrigger = 0.0;
	bool triggerOnScaling = false;
	double triggerDelay = 0.0;
	bool triggerRandomMode = false;
	bool triggerChangeMode = false;
		
	// read input

	if (iter->inRangeCount)
		iter->inRangeCount->GetData( &rangeCount, sizeof(double), pEvaluateInfo );
	int rangeCountInt = (int) floor(rangeCount);
	if (rangeCountInt < 1) rangeCountInt = 1;
	if (rangeCountInt > MAX_NUMBER_OF_FRAME_RANGE) rangeCountInt = MAX_NUMBER_OF_FRAME_RANGE;

	if (iter->inFPS)
		iter->inFPS->GetData( &fps, sizeof(double), pEvaluateInfo );

	if (iter->inTriggerChangeMode)
		iter->inTriggerChangeMode->GetData( &triggerChangeMode, sizeof(bool), pEvaluateInfo ); 
	if (iter->inTriggerRandomMode)
		iter->inTriggerRandomMode->GetData( &triggerRandomMode, sizeof(bool), pEvaluateInfo ); 

		
	if (iter->inPlayOffset)
		iter->inPlayOffset->GetData( &playOffset, sizeof(double), pEvaluateInfo );
	if (iter->inLoopPlay)
		iter->inLoopPlay->GetData( &loopPlay, sizeof(bool), pEvaluateInfo );
	if (iter->inSpeed)
		iter->inSpeed->GetData( &speed, sizeof(double), pEvaluateInfo );
	if (iter->inTrigger && iter->inTrigger->FCurve != nullptr)
	{
		if (false == iter->inTrigger->ReadData( &trigger, pEvaluateInfo ) )
			trigger = 0.0;

		rawTrigger = trigger;
		trigger = 0.01 * trigger;
		if (trigger < 0.0) trigger = 0.0;
		if (trigger > 1.0) trigger = 1.0;
	}

	// do we need to reset ?!
	if (iter->needReset && localPlay && resetTime == localTime)
	{
		ResetNode( *iter );
		iter->needReset = false;
	}
	else if (localPlay && resetTime != localTime)
	{
		iter->needReset = true;
	}

	if (iter->inTriggerOnScaling)
		iter->inTriggerOnScaling->GetData( &triggerOnScaling, sizeof(bool), pEvaluateInfo );
	if (iter->inTriggerDelay)
		iter->inTriggerDelay->GetData( &triggerDelay, sizeof(double), pEvaluateInfo );

	//modelScaling[0] = modelScaling[1] = modelScaling[2] = 1.0;
	if (triggerOnScaling && iter->modelScaling)
		if (false == iter->modelScaling->ReadData( &modelScaling[0], pEvaluateInfo ) )
			triggerOnScaling = false;

	double curModelScaling = 0.0;
	if (triggerOnScaling)
	{
		//curModelScaling = std::max( std::max(modelScaling[0], modelScaling[1]), modelScaling[2]);
		curModelScaling = modelScaling[0] + modelScaling[1] + modelScaling[2];
		if (curModelScaling < triggerOnScaleThreshold)
			curModelScaling = 0.0;

		if (iter->lastModelScaling == 0.0 && curModelScaling > 0.0)
		{
			triggerOnScaling = true;


			if (localPlay)
			{
				iter->offsetTime = localTime + FBTime(0,0,0, (int) playOffset );
			}


		}
		else
			triggerOnScaling = false;

		iter->lastModelScaling = curModelScaling;
	}

	

	trigger = trigger + (triggerOnScaling) ? 1.0 : 0.0;
	FBTime curTime = (localPlay) ? localTime : systemTime;
	
	// do we need to choose a new range index on trigger
	int rangeIndexInt = 0;

	// DONE: change range Index from trigger value (in specified mode)

	if (trigger > 0.0 && iter->lastTrigger == 0.0)
	{
		//
		if (rawTrigger > 0.0 && triggerChangeMode && rawTrigger <= rangeCountInt)
		{
			double rangeIndex = floor(rawTrigger) - 1.0;
			rangeIndexInt = (int) rangeIndex;

			if (rangeIndexInt < 0) rangeIndexInt = 0;
			if (rangeIndexInt >= rangeCountInt) rangeIndexInt = rangeCountInt-1;
				
			if (iter->inTriggerIndex)
				iter->inTriggerIndex->SetData( &rangeIndex );
		}
		else if (triggerRandomMode)
		{
			double randomValue = floor(dist(e2) * rangeCountInt);
			rangeIndexInt = (int) randomValue;

			if (iter->inTriggerIndex)
				iter->inTriggerIndex->SetData( &randomValue );
		}
	}
	else
	{
		if (iter->inTriggerIndex)
			iter->inTriggerIndex->GetData( &rangeIndex, sizeof(double), pEvaluateInfo );
		rangeIndexInt = (int) rangeIndex;	
	}

	if (rangeIndexInt < 0) rangeIndexInt = 0;
	else if (rangeIndexInt >= rangeCountInt) rangeIndexInt = rangeCountInt-1;

	if (iter->inFrameRange[rangeIndexInt].inStartFrame)
		iter->inFrameRange[rangeIndexInt].inStartFrame->GetData( &firstFrame, sizeof(double), pEvaluateInfo );

	if (iter->inFrameRange[rangeIndexInt].inStopFrame)
		iter->inFrameRange[rangeIndexInt].inStopFrame->GetData( &lastFrame, sizeof(double), pEvaluateInfo );

	// compute values
	double vis = 1.0;

	if (iter->offsetTime == FBTime::MinusInfinity)
		ComputeLocalOffsetTime(*iter);

	ComputeTextureMatrix(outPos, outScl, iter->lastTime, iter->offsetTime, *iter, iter->lastFrame, iter->lastTrigger, vis,
		curTime, 
		infoIter->second, fps, localPlay, playOffset, firstFrame, lastFrame, trigger, loopPlay, speed );
		
	// write output
	if (zeroNonPlaying)
	{
		outScl[0] *= vis;
		outScl[1] *= vis;
		outScl[2] *= vis;
	}
	/*
	if (iter->outScaling)
		iter->outScaling->WriteData( scaling, pEvaluateInfo );
	if (iter->outTranslation)
		iter->outTranslation->WriteData( translation, pEvaluateInfo );

	if (iter->outVisibility)
		iter->outVisibility->WriteData( &vis, pEvaluateInfo );
		*/
	//
	if (useLocalCache)
	{
		iter->cachedFrame = localTime;
		memcpy( iter->cachedPos, outPos, sizeof(double) * 3 );
		memcpy( iter->cachedScl, outScl, sizeof(double) * 3 );
	}

	/*
	// output debug
	if (nullptr != iter->debugTexture)
	{
		iter->debugTexture->Translation.SetData( translation );
		iter->debugTexture->Scaling.SetData( scaling );
	}
	*/
	//if (pAnimationNode == iter->outTranslation || pAnimationNode == iter->outScaling)
	//	result = true;

	return true;
}

bool SolverSpriteSheet::AnimationNodeNotify ( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo )
{
	if (Active == false)
	{
		AnimationNodesOutDisableIfNotWritten(pEvaluateInfo);
		return false;
	}

	bool result = false;

	double translation[3] = {0.0, 5.0, 3.0};
	double scaling[3] = {1.0, 1.0, 1.0};
	
	FBTime resetTime = ResetTime;
	FBTime localTime = pEvaluateInfo->GetLocalTime();

//	FBTexture *pLastTexture = nullptr;
//	auto infoIter = mSpriteInfoMap.end();

	bool localPlay = LocalPlay;
	bool useLocalCache = UseLocalCache;
	bool zeroNonPlaying = ZeroNonPlaying;
	double triggerOnScalingThreshold = TriggerOnScalingThreshold;

	if (pAnimationNode == mDummyNode)
	{

		if (localPlay && resetTime == localTime)
		{
			for (auto iter=begin(mNodes); iter!=end(mNodes); ++iter)
			{
				if (iter->needReset)
				{
					ResetNode(*iter);
					iter->needReset = false;
				}
			}
		}

		mDummyNode->DisableIfNotWritten(pEvaluateInfo);
		return result;
	}


	auto iter = mNodeToSpriteInfoMap.find( pAnimationNode );
	if (iter != mNodeToSpriteInfoMap.end() )
	{
		SpriteSheetNode *pNode = iter->second;
		result = ComputeSpriteSheetNode( pEvaluateInfo, pNode, localPlay, useLocalCache, zeroNonPlaying, resetTime,
			triggerOnScalingThreshold,
			&scaling[0], &translation[0]);

		if (pNode->inIndex)
		{
			double value = (double) pNode->index;
			pNode->inIndex->SetData( &value );
		}

		if (result == true)
		{
			if (pNode->outScaling)
				pNode->outScaling->WriteData(scaling, pEvaluateInfo);
			if (pNode->outTranslation)
				pNode->outTranslation->WriteData(translation, pEvaluateInfo);
		}
		else
		{
			if (pNode->outScaling)
				pNode->outScaling->DisableIfNotWritten(pEvaluateInfo);
			if (pNode->outTranslation)
				pNode->outTranslation->DisableIfNotWritten(pEvaluateInfo);
		}

	}
	else
	{
		// failed to find a processing node
		pAnimationNode->DisableIfNotWritten(pEvaluateInfo);
		result = false;
	}

	

	/*
	if (mDummyNode)
	{
		mDummyNode->DisableIfNotWritten(pEvaluateInfo);
	}
	
	if (result == true)
	{
		AnimationNodesOutDisableIfNotWritten(pEvaluateInfo);
	}
	*/
	return result;
}


bool SolverSpriteSheet::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return ParentClass::FbxStore(pFbxObject, pStoreWhat);
}

bool SolverSpriteSheet::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) 
{
	if (pStoreWhat == kCleanup)
	{
		if (Active == true)
		{
			Active = false;
			mSystem.OnUIIdle.Add( this, (FBCallback) &SolverSpriteSheet::OnSystemIdle );
		}
	}

	return ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);
}

void SolverSpriteSheet::OnSystemIdle(HISender pSender, HKEvent pEvent)
{

	Active = true;
	mSystem.OnUIIdle.Remove( this, (FBCallback) &SolverSpriteSheet::OnSystemIdle );

}

void SolverSpriteSheet::OnSystemIdle2(HISender pSender, HKEvent pEvent)
{

	if (mDummyNode)
	{
		
		AnimationNodeDestroy(mDummyNode);
		mDummyNode = nullptr;
	}

	mSystem.OnUIIdle.Remove( this, (FBCallback) &SolverSpriteSheet::OnSystemIdle2 );

}

void SolverSpriteSheet::ClearSpriteInfoMap()
{
	for (auto iter=begin(mSpriteInfoMap); iter!=end(mSpriteInfoMap); ++iter)
	{
		SpriteSheetInfo *pInfo = iter->second;
		if (pInfo)
		{
			delete pInfo;
		}
	}
	mSpriteInfoMap.clear();
}

void SolverSpriteSheet::ClearNodes()
{
	/*
	if (mDummyNode)
	{
		AnimationNodeDestroy(mDummyNode);
		mDummyNode = nullptr;
	}
	*/
	for (auto iter=begin(mNodes); iter!=end(mNodes); ++iter)
	{
		if (iter->modelScaling)
		{
			AnimationNodeDestroy(iter->modelScaling);
			iter->modelScaling = nullptr;
		}
		if (iter->outScaling)
		{
			AnimationNodeDestroy(iter->outScaling);
			iter->outScaling = nullptr;
		}
		if (iter->outTranslation)
		{
			AnimationNodeDestroy(iter->outTranslation);
			iter->outTranslation = nullptr;
		}
		if (iter->outVisibility)
		{
			AnimationNodeDestroy(iter->outVisibility);
			iter->outVisibility = nullptr;
		}
	}

}

bool SolverSpriteSheet::ComputeBasicTextureInformation( const int rows, const int cols, SpriteSheetInfo *outInfo )
{
	if (outInfo == nullptr)
		return false;

	int countU = cols;
	int countV = rows;

	if (countU <= 0)
		countU = 1;
	if (countV <= 0)
		countV = 1;

	int numberOfFrames = countU * countV;

	outInfo->frames.resize(numberOfFrames);

	double paramU = 0.0;
	double paramV = 0.0;

	FBVector3d tr;
	FBSVector scl;
	scl[0] = 1.0 / countU;
	scl[1] = 1.0 / countV;

	for (int nframe=0; nframe<numberOfFrames; ++nframe)
	{
		paramU = 1.0 * (nframe % countU) / countU;
		paramV = (1.0/countV) * nframe / countU;

		paramU = floor(paramU / scl[0]);
		paramV = floor(paramV / scl[1]);

		if (paramU >= countU)
			paramU = countU-1;
		if (paramV >= countV)
			paramV = countV-1;

		tr = FBVector3d(paramU/countU, paramV/countV, 0.0);
		tr[1] = 1.0 - tr[1] - scl[1];

		SpriteFrame &frameInfo = outInfo->frames[nframe];

		frameInfo.x = tr[0];
		frameInfo.y = tr[1];
		frameInfo.w = scl[0];
		frameInfo.h = scl[1];
	}

	return true;
}

bool SolverSpriteSheet::IsNewRangeStarted( const std::string &prevName, const std::string &newName )
{
	if (newName.length() == 0)
		return false;
	if (prevName.length() == 0 && newName.length() > 0)
		return true;

	// remove extension
	size_t lastPrevIndex = prevName.find_last_of(".");
	size_t lastNewIndex = newName.find_last_of("."); 
	//std::string rawname = newName.substr(0, lastindex); 

	if (lastPrevIndex == 0)
		lastPrevIndex = prevName.size();
	if (lastNewIndex == 0)
		lastNewIndex = newName.size();

	if (lastPrevIndex != lastNewIndex)
		return true;

	// skip digit part (diff part)
	size_t lastMainIndex = lastNewIndex-1;
	while (lastMainIndex > 0 && isdigit(newName[lastMainIndex]) )
	{
		lastMainIndex--;
	}

	// check main part, should be equal
	for (int i=0; i<lastMainIndex; ++i)
		if (prevName[i] != newName[i])
			return true;

	return false;
}

bool SolverSpriteSheet::ReadXmlTextureInformation( const char *filename, SpriteSheetInfo *outInfo )
{
	if (outInfo == nullptr)
		return false;

	// 
	TiXmlDocument	doc;

	if (doc.LoadFile( filename ) == false)
	{
		return false;
	}

	TiXmlNode *node = nullptr;
	TiXmlElement *headElement = nullptr;
	TiXmlElement *spriteElement = nullptr;
	TiXmlAttribute  *attrib = nullptr;

	node = doc.FirstChild("TextureAtlas");
	if (node == nullptr)
	{
		return false;
	}

	int width = 1;
	int height = 1;

	headElement = node->ToElement();
	if (headElement)
	{
		// enumerate attribs
		for( attrib = headElement->FirstAttribute();
			 attrib;
			 attrib = attrib->Next() )
		{
			const char * attribName = attrib->Name();
			if ( strcmp(attribName, "width") == 0 )
				width = attrib->IntValue();
			else if ( strcmp(attribName, "height") == 0 )
				height = attrib->IntValue();
		}

		spriteElement = node->FirstChildElement("sprite");

		// compute number of frames

		int numberOfFrames = 0;
		while(spriteElement)
		{
			numberOfFrames += 1;
			spriteElement = spriteElement->NextSiblingElement();
		}

		outInfo->frames.resize( numberOfFrames );
		outInfo->ranges.clear();
		numberOfFrames = 0;

		// fill with the data
		int x=0;
		int y=0;
		int w = 1;
		int h = 1;

		int rangeStart = 0;
		std::string frameName("");

		spriteElement = node->FirstChildElement("sprite");
		while(spriteElement)
		{

			// enumerate attribs
			for( attrib = spriteElement->FirstAttribute();
				 attrib;
				 attrib = attrib->Next() )
			{
				const char * attribName = attrib->Name();
				if ( strcmp(attribName, "n") == 0 )
				{
					const std::string &name = attrib->ValueStr();

					// determine a start of a new range
					if ( IsNewRangeStarted(frameName, name) )
					{
						rangeStart = numberOfFrames;
						outInfo->ranges.push_back(rangeStart);
						
						// DONE: add prev range and start counting a new one
						frameName = name;
					}

				}
				else if ( strcmp(attribName, "x") == 0 )
					x = attrib->IntValue();
				else if ( strcmp(attribName, "y") == 0 )
					y = attrib->IntValue();
				else if ( strcmp(attribName, "w") == 0 )
					w = attrib->IntValue();
				else if ( strcmp(attribName, "h") == 0 )
					h = attrib->IntValue();
			}

			SpriteFrame &frameInfo = outInfo->frames[numberOfFrames];

			frameInfo.x = 1.0 * x / width;
			frameInfo.y = 1.0 - 1.0 * y / height - 1.0 * h / height;
			frameInfo.w = 1.0 * w / width;
			frameInfo.h = 1.0 * h / height;

			numberOfFrames += 1;
			spriteElement = spriteElement->NextSiblingElement();
		}
	}

	return true;
}

void SolverSpriteSheet::OnManualResetClick()
{
	// TODO: reset all solver nodes
	for(auto iter=begin(mNodes); iter!=end(mNodes); ++iter)
	{
		ResetNode( *iter );
	}
}