
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: wallbricks_constraint.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declarations

#include "wallbricks_constraint.h"

#include <limits>

#include "algorithm\math3d_mobu.h"


//--- Registration defines
#define	ORCONSTRAINTWALLBRICKS__CLASS		ORCONSTRAINTWALLBRICKS__CLASSNAME
#define ORCONSTRAINTWALLBRICKS__NAME		"Wall Bricks"
#define ORCONSTRAINTWALLBRICKS__LABEL		"Wall Bricks Along a curve"
#define ORCONSTRAINTWALLBRICKS__DESC		"Wall Bricks Along a curve"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINTWALLBRICKS__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINTWALLBRICKS__NAME,
								ORCONSTRAINTWALLBRICKS__CLASS,
								ORCONSTRAINTWALLBRICKS__LABEL,
								ORCONSTRAINTWALLBRICKS__DESC,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)


////////////////////////////////////////
//


//

void AddPropertyViewForWallBricks(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(ORCONSTRAINTWALLBRICKS__CLASSSTR, pPropertyName, pHierarchy);
}
 
void ORConstraintWallBricks::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForWallBricks("Active", "");
	AddPropertyViewForWallBricks("Lock", "");
	AddPropertyViewForWallBricks("Snap", "");
	AddPropertyViewForWallBricks("Weight", "");
	AddPropertyViewForWallBricks("Progress", "");
	AddPropertyViewForWallBricks("Animate Progress", "");
	AddPropertyViewForWallBricks("Live Animation", "");
	AddPropertyViewForWallBricks("Brick Animation Time", "");

	AddPropertyViewForWallBricks("Snap Setup", "", true);
	AddPropertyViewForWallBricks("Curve Subdivisions", "Snap Setup");
	AddPropertyViewForWallBricks("Curve Segment Subdivisions", "Snap Setup");
	AddPropertyViewForWallBricks("Snap Threshold", "Snap Setup");
	AddPropertyViewForWallBricks("Auto Align Root", "Snap Setup");

	AddPropertyViewForWallBricks("Progress Setup", "", true);
	AddPropertyViewForWallBricks("Progress Inverse", "Progress Setup");
	AddPropertyViewForWallBricks("Progress Offset", "Progress Setup");
	AddPropertyViewForWallBricks("Show Only While Processing", "Progress Setup");
	
	AddPropertyViewForWallBricks("System Setup", "", true);
	AddPropertyViewForWallBricks("Reset Time", "System Setup");
	AddPropertyViewForWallBricks("Reset Action", "System Setup");
	AddPropertyViewForWallBricks("Static Curve", "System Setup");
	AddPropertyViewForWallBricks("Fix Instances", "System Setup");

	AddPropertyViewForWallBricks("Expressions Setup", "", true);
	AddPropertyViewForWallBricks("Expressions Reset", "Expressions Setup");
	AddPropertyViewForWallBricks("Expressions Load", "Expressions Setup");
	AddPropertyViewForWallBricks("Expressions Save", "Expressions Setup");

	AddPropertyViewForWallBricks("Errors Count", "Expressions Setup");
	AddPropertyViewForWallBricks("Last Error", "Expressions Setup");

	AddPropertyViewForWallBricks("Position X Expr", "Expressions Setup");
	AddPropertyViewForWallBricks("Position Y Expr", "Expressions Setup");
	AddPropertyViewForWallBricks("Position Z Expr", "Expressions Setup");

	AddPropertyViewForWallBricks("Rotation X Expr", "Expressions Setup");
	AddPropertyViewForWallBricks("Rotation Y Expr", "Expressions Setup");
	AddPropertyViewForWallBricks("Rotation Z Expr", "Expressions Setup");

	AddPropertyViewForWallBricks("Scaling X Expr", "Expressions Setup");
	AddPropertyViewForWallBricks("Scaling Y Expr", "Expressions Setup");
	AddPropertyViewForWallBricks("Scaling Z Expr", "Expressions Setup");

	AddPropertyViewForWallBricks("Trigger Setup", "", true);
	AddPropertyViewForWallBricks("Trigger Mode", "Trigger Setup");
	AddPropertyViewForWallBricks("Trigger", "Trigger Setup");
	AddPropertyViewForWallBricks("Turn Off After Processing", "Trigger Setup");
	AddPropertyViewForWallBricks("Auto Trigger Timer", "Trigger Setup");

	AddPropertyViewForWallBricks("Size Curve Setup", "", true);
	AddPropertyViewForWallBricks("Use Size Curve", "Size Curve Setup");
	AddPropertyViewForWallBricks("Size Curve", "Size Curve Setup");
	AddPropertyViewForWallBricks("SizeCurveHolder", "Size Curve Setup");
}

/************************************************
 *	Creation function.
 ************************************************/
bool ORConstraintWallBricks::FBCreate()
{
	HasLayout = true;

	// Create reference groups
	mGroupCurve	= ReferenceGroupAdd( "Curve Object",	1 );
	mGroupConstrained	= ReferenceGroupAdd( "Constrain",		MAX_NUMBER_OF_BRICKS );

	mSourceCurve		= nullptr;
	mFirstTime			= true;
	mLastTime			= FBTime::Infinity;

	//
	//

	FBPropertyPublish( this, Progress, "Progress", nullptr, nullptr );
	FBPropertyPublish( this, AnimateProgress, "Animate Progress", nullptr, ActionAnimateProgress );

	FBPropertyPublish( this, ProgressOffset, "Progress Offset", nullptr, SetProgressOffset );
	FBPropertyPublish( this, ProgressInverse, "Progress Inverse", nullptr, SetProgressInverse );

	FBPropertyPublish( this, ShowOnlyWhileProcessing, "Show Only While Processing", nullptr, nullptr );

	FBPropertyPublish( this, TriggerMode, "Trigger Mode", nullptr, nullptr );
	FBPropertyPublish( this, Trigger, "Trigger", nullptr, nullptr );
	FBPropertyPublish( this, TurnOffAfterProcessing, "Turn Off After Processing", nullptr, nullptr );
	FBPropertyPublish( this, AutoTriggerTimer, "Auto Trigger Timer", nullptr, nullptr );

	//FBPropertyPublish( this, BrickSideLength, "Brick Side Length", nullptr, nullptr );
	FBPropertyPublish( this, BrickAnimationTime, "Brick Animation Time", nullptr, nullptr );

	FBPropertyPublish( this, CurveSubdivisions, "Curve Subdivisions", nullptr, nullptr );
	FBPropertyPublish( this, CurveSegmentSubdivisions, "Curve Segment Subdivisions", nullptr, nullptr );

	FBPropertyPublish( this, AutoAlignRoot, "Auto Align Root", nullptr, nullptr );
	FBPropertyPublish( this, SnapThreshold, "Snap Threshold", nullptr, nullptr );

	FBPropertyPublish( this, LiveAnimation, "Live Animation", nullptr, SetLiveAnimation );

	FBPropertyPublish( this, ResetTime, "Reset Time", nullptr, nullptr );
	FBPropertyPublish( this, ResetAction, "Reset Action", nullptr, nullptr );

	FBPropertyPublish( this, StaticCurve, "Static Curve", nullptr, ActionStaticCurve);

	FBPropertyPublish( this, ExpressionReset, "Expressions Reset", nullptr, ActionExpressionReset );
	FBPropertyPublish( this, ExpressionLoad, "Expressions Load", nullptr, ActionExpressionLoad );
	FBPropertyPublish( this, ExpressionSave, "Expressions Save", nullptr, ActionExpressionSave );

	FBPropertyPublish( this, ScriptErrorCount, "Errors Count", nullptr, nullptr );
	FBPropertyPublish( this, ScriptError, "Last Error", nullptr, nullptr );

	FBPropertyPublish( this, ScriptPositionX, "Position X Expr", nullptr, SetScriptPositionX );
	FBPropertyPublish( this, ScriptPositionY, "Position Y Expr", nullptr, SetScriptPositionY );
	FBPropertyPublish( this, ScriptPositionZ, "Position Z Expr", nullptr, SetScriptPositionZ );

	FBPropertyPublish( this, ScriptRotationX, "Rotation X Expr", nullptr, SetScriptRotationX );
	FBPropertyPublish( this, ScriptRotationY, "Rotation Y Expr", nullptr, SetScriptRotationY );
	FBPropertyPublish( this, ScriptRotationZ, "Rotation Z Expr", nullptr, SetScriptRotationZ );

	FBPropertyPublish( this, ScriptScalingX, "Scaling X Expr", nullptr, SetScriptScalingX );
	FBPropertyPublish( this, ScriptScalingY, "Scaling Y Expr", nullptr, SetScriptScalingY );
	FBPropertyPublish( this, ScriptScalingZ, "Scaling Z Expr", nullptr, SetScriptScalingZ );

	FBPropertyPublish( this, FixInstances, "Fix Instances", nullptr, ActionFixInstances );

	FBPropertyPublish( this, UseSizeCurve, "Use Size Curve", nullptr, nullptr );
	FBPropertyPublish( this, SizeCurve, "Size Curve", nullptr, ActionSizeCurve );
	FBPropertyPublish( this, SizeCurveHolder, "SizeCurveHolder", nullptr, nullptr );

	FBPropertyPublish( this, Help, "Help", nullptr, ActionHelp );
	FBPropertyPublish( this, About, "About", nullptr, ActionAbout );

	ScriptErrorCount.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );
	ScriptError.ModifyPropertyFlag( kFBPropertyFlagReadOnly, true );

	ScriptErrorCount = 0;
	ScriptError = "";

	UseSizeCurve = false;
	SizeCurveHolder = 0.0;
	SizeCurveHolder.SetAnimated(true);

	FBTime time2;
	time2.SetSecondDouble(1.0);
	
	double value = 0.0;
	SizeCurveHolder.GetAnimationNode()->KeyAdd( FBTime(0), &value );
	value = 100.0;
	SizeCurveHolder.GetAnimationNode()->KeyAdd( time2, &value );
	
	//SizeCurveHolder.ModifyPropertyFlag( kFBPropertyFlagHideProperty, true );

	Progress.SetMinMax(0.0, 100.0, true, true);
	Progress = 0.0;

	ProgressOffset.SetMinMax(-100.0, 100.0, true, true);
	ProgressOffset = 0.0;

	ProgressInverse = false;
	ShowOnlyWhileProcessing = false;

	TriggerMode = false;
	Trigger = 0.0;
	TurnOffAfterProcessing = true;

	AutoTriggerTimer = 0.0;

	Trigger.SetMinMax(0.0, 1.0, true, true);
	Trigger.SetAnimated(true);
	
	CurveSubdivisions.SetMinMax(1.0, 16192.0, true, true);
	CurveSubdivisions = 4096;

	CurveSegmentSubdivisions.SetMinMax(1.0, 8096.0, true, true);
	CurveSegmentSubdivisions = 32;

	//BrickSideLength = 5.0;
	BrickAnimationTime = 2.0;
	LiveAnimation = false;
	AutoAlignRoot = false;

	SnapThreshold = 5.0;

	ResetTime = FBTime(0);

	StaticCurve = true;

	mExpression.ExpressionInit();

	mExpressionFailed = false;
	mIsDistToCamUsed = false;

	mEvalCurve = true;

	mLastTrigger = 0;
	mTriggerElement = -1;

	//
	SetExpressionsToDefault();

	return true;
}

void ORConstraintWallBricks::InitBrick( BrickNode &node )
{
	node.posNode = nullptr;
	node.rotNode = nullptr;
	node.scaleNode = nullptr;

	node.active = false;
	node.creationPercent = 0.0;

	node.localTime = 0.0;
	node.animFactor = 0.0;
	node.initTM.Identity();
	node.curveTM.Identity();

	node.cachedAnimFactor = -1.0;
	node.cachedDistToCamera = -1.0;

	node.cachedPosx = 0.0;
	node.cachedPosy = 0.0;
	node.cachedPosz = 0.0;

	node.cachedRotx = 0.0;
	node.cachedRoty = 0.0;
	node.cachedRotz = 0.0;

	node.cachedSclx = 0.0;
	node.cachedScly = 0.0;
	node.cachedSclz = 0.0;
}

bool ORConstraintWallBricks::IsBrickCached( const BrickNode &node, double f, double distToCam )
{
	if (mIsDistToCamUsed)
		return ( abs(node.cachedAnimFactor-f) < 0.001 && abs(node.cachedDistToCamera-distToCam) < 0.001);
	else
		return (  abs(node.cachedAnimFactor-f) < 0.001 );
}

void ORConstraintWallBricks::GetBrickCache( const BrickNode &node, double &x, double &y, double &z, double &rx, double &ry, double &rz, double &sx, double &sy, double &sz )
{
	x = node.cachedPosx;
	y = node.cachedPosy;
	z = node.cachedPosz;

	rx = node.cachedRotx;
	ry = node.cachedRoty;
	rz = node.cachedRotz;

	sx = node.cachedSclx;
	sy = node.cachedScly;
	sz = node.cachedSclz;
}

void ORConstraintWallBricks::CacheBrick( BrickNode &node, double f, double distToCam, double x, double y, double z, double rx, double ry, double rz, double sx, double sy, double sz )
{
	node.cachedAnimFactor = f;
	node.cachedDistToCamera = distToCam;

	node.cachedPosx = x;
	node.cachedPosy = y;
	node.cachedPosz = z;

	node.cachedRotx = rx;
	node.cachedRoty = ry;
	node.cachedRotz = rz;

	node.cachedSclx = sx;
	node.cachedScly = sy;
	node.cachedSclz = sz;
}



/************************************************
 *	Destruction function.
 ************************************************/
void ORConstraintWallBricks::FBDestroy()
{
}


/************************************************
 *	Setup all of the animation nodes.
 ************************************************/
void ORConstraintWallBricks::SetupAllAnimationNodes()
{
	if (ReferenceGet( mGroupCurve, 0 ) && FBIS(ReferenceGet(mGroupCurve, 0), FBModelPath3D) && ReferenceGet( mGroupConstrained, 0 ) )
	{
		mSourceCurve		= AnimationNodeOutCreate( 0, ReferenceGet( mGroupCurve,	0 ), ANIMATIONNODE_TYPE_TRANSLATION );

		BrickNode val;
		InitBrick(val);
		mConstrainedBricks.resize( ReferenceGetCount(mGroupConstrained), val );

		FBConnect( ReferenceGet(mGroupCurve, 0), this );

		//FBModelPath3D *pCurve = (FBModelPath3D*) ReferenceGet(mGroupCurve, 0);
		
		int index = 1;
		for (int i=0, count=ReferenceGetCount(mGroupConstrained); i<count; ++i)
		{
			FBModel *pModel = ReferenceGet( mGroupConstrained, i ); 

			mConstrainedBricks[i].posNode = AnimationNodeInCreate	( i, pModel, ANIMATIONNODE_TYPE_TRANSLATION );
			mConstrainedBricks[i].rotNode = AnimationNodeInCreate	( i, pModel, ANIMATIONNODE_TYPE_ROTATION );
			mConstrainedBricks[i].scaleNode = AnimationNodeInCreate	( i, pModel, ANIMATIONNODE_TYPE_SCALING );

			index += 3;

			FBConnect( this, pModel );
		}
	}
}


/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void ORConstraintWallBricks::RemoveAllAnimationNodes()
{
	for (int i=GetSrcCount()-1; i>=0; --i)
	{
		if ( FBIS( GetSrc(i), FBModelPath3D ) )
			FBDisconnect( GetSrc(i), this );
	}

	for (int i=0, count=ReferenceGetCount(mGroupConstrained); i<count; ++i)
	{
		FBModel *pModel = ReferenceGet( mGroupConstrained, i ); 

		FBDisconnect( this, pModel );
	}
}


/************************************************
 *	FBX storage of constraint parameters.
 ************************************************/
bool ORConstraintWallBricks::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxStore(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAttributes)
	{
		pFbxObject->FieldWriteI( "count", (int) mConstrainedBricks.size() );

		char buffer[64];


		for (auto iter=begin(mConstrainedBricks); iter!=end(mConstrainedBricks); ++iter)
		{
			pFbxObject->FieldWriteD( "percent", iter->creationPercent );

			for (int i=0; i<16; ++i)
			{
				memset( buffer, 0, sizeof(char) * 64);
				sprintf_s( buffer, 64, "initTM%d", i );
				pFbxObject->FieldWriteD( buffer, iter->initTM[i]);
			}
		}
	}


	return true;
}


/************************************************
 *	FBX retrieval of constraint parameters.
 ************************************************/
bool ORConstraintWallBricks::FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxStore(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAttributes)
	{
		int count = pFbxObject->FieldReadI( "count" );

		if (mConstrainedBricks.size() != count)
			mConstrainedBricks.resize(count);

		char buffer[64];

		for (auto iter=begin(mConstrainedBricks); iter!=end(mConstrainedBricks); ++iter)
		{
			iter->creationPercent = pFbxObject->FieldReadD( "percent" );

			for (int i=0; i<16; ++i)
			{
				memset( buffer, 0, sizeof(char) * 64);
				sprintf_s( buffer, 64, "initTM%d", i );
				iter->initTM[i] = pFbxObject->FieldReadD( buffer );
			}
		}

		Active = false;
	}


	return true;
}

void ORConstraintWallBricks::SnapSuggested()
{
	if (Active == false)
	{
		if (ReferenceGet( mGroupCurve, 0 ) 
			&& FBIS(ReferenceGet(mGroupCurve, 0), FBModelPath3D) 
			&& mConstrainedBricks.size() > 0 )
		{
			mSourceCurve		= AnimationNodeOutCreate( 0, ReferenceGet( mGroupCurve,	0 ), ANIMATIONNODE_TYPE_TRANSLATION );

			FBModelPath3D *pCurve = (FBModelPath3D*) ReferenceGet(mGroupCurve, 0);
			
			FBMatrix curveTM;
			pCurve->GetMatrix(curveTM);

			double percent;
			FBVector4d pointOnCurve;
			FBVector3d pos;
			int numberOfSubdivs = CurveSubdivisions;
			int segmentSubdivs = CurveSegmentSubdivisions;

			if (numberOfSubdivs <= 0 || segmentSubdivs <= 0)
				return;

			// TODO: build k-d tree and find a closest point

			std::vector<KDTree>			mTreePages;	// k-d tree
			std::vector<FBVector3d>		positions;

			const size_t pageLimit = 65535;
			const size_t positionsCount = numberOfSubdivs * segmentSubdivs;
			double step = 100.0 / (positionsCount);

			// do it in pages, each page contains 65535 positions
			const int numberOfPages = (int) (1 + positionsCount / pageLimit);

			if (numberOfPages == 1)
				positions.resize(positionsCount);
			else
				positions.resize(pageLimit);
			mTreePages.resize(numberOfPages);

			//
			percent = 0.0;
			size_t restCount = positionsCount;

			for (int npage=0; npage<numberOfPages; ++npage)
			{
				size_t localCount = (npage==numberOfPages-1) ? restCount : pageLimit;

				for (size_t i=0; i<localCount; ++i)
				{
					pointOnCurve = pCurve->Total_GlobalPathEvaluate(percent);
					positions[i] = FBVector3d(pointOnCurve);

					percent += step;
				}

				mTreePages[npage].BuildKDTree( (int) localCount, (double*) positions.data() );

				restCount -= pageLimit;
			}

			

			double lthres = SnapThreshold;

			for (int i=0, count=ReferenceGetCount(mGroupConstrained); i<count; ++i)
			{
				FBModel *pModel = ReferenceGet( mGroupConstrained, i ); 
				pModel->GetVector(pos);
				pModel->GetMatrix(mConstrainedBricks[i].initTM);
				
				FBGetLocalMatrix( mConstrainedBricks[i].initTM, curveTM, mConstrainedBricks[i].initTM );
			
				
				double minDist = -1.0;
				mConstrainedBricks[i].creationPercent = 0.0;

				for (int npage=0; npage<numberOfPages; ++npage)
				{
					FBVector3d nearestPos;
					
					// hack because method returns index OR nearest pos
					mTreePages[npage].SearchForNearest( pos, lthres, nearestPos );
					int index = mTreePages[npage].SearchForNearest( pos, lthres );

					if (index > 0)
					{
						double dist = VectorLength(VectorSubtract(pos, nearestPos));

						if (minDist < 0.0)
							minDist = dist;

						if (dist <= minDist)
						{
							mConstrainedBricks[i].creationPercent = step * ((npage * pageLimit) + index);
							minDist = dist;
						}
					}
				}
				
			}

			mEvalCurve = true;
		}
	}
}

/************************************************
 *	Suggest a freeze.
 ************************************************/
void ORConstraintWallBricks::FreezeSuggested()
{
	mFirstTime = true;
	FBConstraint::FreezeSuggested();

	if( ReferenceGet( 0,0 ) )
	{
		FreezeSRT( (FBModel*)ReferenceGet( 0, 0), true, true, true );
	}
}


/************************************************
 *	Real-Time Engine Evaluation
 ************************************************/
bool ORConstraintWallBricks::AnimationNodeNotify(FBAnimationNode* pConnector,FBEvaluateInfo* pEvaluateInfo,FBConstraintInfo* pConstraintInfo)
{
	if (Active == false)
		return false;

	double lPosition[3];
	double lDefRotation[3] = {0.0, 0.0, 0.0};
	double lDefScale[3] = {1.0, 1.0, 1.0};

	//pConnector->WriteData( lDefScale, pEvaluateInfo );
	//return true;

	if( mSourceCurve && mConstrainedBricks.size() > 0 )
	{
		int lStatus = mSourceCurve->ReadData		( lPosition, pEvaluateInfo );

		FBModelPath3D *pCurve = nullptr;
		
		// If the read was not from a dead node.
		if( lStatus != 0 )
		{
			int count = mSourceCurve->GetSrcCount();
			for (int i=0; i<count; ++i)
			{
				FBPlug *pPlug = mSourceCurve->GetSrc(i);
				const char *sourceClass = pPlug->ClassName();
				pPlug = pPlug->GetOwner();
				sourceClass = pPlug->ClassName();
			
				if (pPlug->Is( FBModelPath3D::TypeInfo ) )
				{
					pCurve = (FBModelPath3D*) (FBModel*) pPlug;	
				}
			}
		}

		if (pCurve != nullptr)
		{

			bool lProgressInverse = ProgressInverse;
			double lProgressOffset = ProgressOffset;

			FBCamera *pCamera = FBSystem::TheOne().Renderer->CurrentCamera;
			if (FBIS(pCamera, FBCameraSwitcher) )
				pCamera = ((FBCameraSwitcher*) pCamera)->CurrentCamera;

			FBVector3d cameraPos(0.0, 0.0, 0.0);
			if (pCamera)
				pCamera->GetVector(cameraPos, kModelTranslation, true, pEvaluateInfo);

			if (mNeedExpressionParsing)
			{
				mExpression.ExpressionParse( ScriptPositionX.AsString(), ScriptPositionY.AsString(), ScriptPositionZ.AsString(),
					ScriptRotationX.AsString(), ScriptRotationY.AsString(), ScriptRotationZ.AsString(),
					ScriptScalingX.AsString(), ScriptScalingY.AsString(), ScriptScalingZ.AsString() );

				ScriptErrorCount = (int) mExpression.GetErrorCount();
				ScriptError.SetString( mExpression.GetLastError() );

				mExpressionFailed = (mExpression.GetErrorCount() > 0);
				mNeedExpressionParsing = false;

				mIsDistToCamUsed = false;
				if ( (strstr(ScriptPositionX.AsString(), "distToCam") != nullptr) 
					|| (strstr(ScriptPositionY.AsString(), "distToCam") != nullptr)
					|| (strstr(ScriptPositionZ.AsString(), "distToCam") != nullptr)
					|| (strstr(ScriptRotationX.AsString(), "distToCam") != nullptr)
					|| (strstr(ScriptRotationY.AsString(), "distToCam") != nullptr)
					|| (strstr(ScriptRotationZ.AsString(), "distToCam") != nullptr)
					|| (strstr(ScriptScalingX.AsString(), "distToCam") != nullptr)
					|| (strstr(ScriptScalingY.AsString(), "distToCam") != nullptr)
					|| (strstr(ScriptScalingZ.AsString(), "distToCam") != nullptr) )
				{
					mIsDistToCamUsed = true;
				}

				// prepare random values

				for(auto iter=begin(mConstrainedBricks); iter!=end(mConstrainedBricks); ++iter)
				{
					iter->randomValue = dist(e2);
				}
			}

			if (mExpressionFailed)
				return false;


			double lProgress=0.0;
			Progress.GetData(&lProgress, sizeof(double), pEvaluateInfo);

			// for now, let's reset all the bricks when progress is zero
			
			double animTime = BrickAnimationTime;
			if (animTime <= 0.0)
				animTime = 1.0;

			FBTime currTime = (LiveAnimation) ? pEvaluateInfo->GetSystemTime() : pEvaluateInfo->GetLocalTime();
			FBTime resetTime = ResetTime;
			if ( (currTime == resetTime) || (mLastTime == FBTime::Infinity) )
				mLastTime = currTime;

			double deltaTime = currTime.GetSecondDouble() - mLastTime.GetSecondDouble();
			mLastTime = currTime;


			if (TriggerMode)
			{
				// trigger some random element

				double trigger=0.0;
			
				// check for trigger timer
				double triggerTime;
				AutoTriggerTimer.GetData( &triggerTime, sizeof(double), pEvaluateInfo );

				if (triggerTime > 0.0)
				{
					if ( (!LiveAnimation && currTime == resetTime) || mLastAutoTriggerTime == FBTime::Infinity)
						mLastAutoTriggerTime = currTime;

					double autoTriggerDelta = currTime.GetSecondDouble() - mLastAutoTriggerTime.GetSecondDouble();
					
					if (autoTriggerDelta > triggerTime)
					{
						trigger = 1.0;
						mLastAutoTriggerTime = currTime;
					}
				}
				
				// get animation trigger
				if (trigger == 0.0)
				{
					Trigger.GetData(&trigger, sizeof(double), pEvaluateInfo);
				}

				if (trigger > 0.0 && mLastTrigger==0.0)
				{
					// select any random element and trigger it
					double randomValue = dist(e2);
					randomValue = floor(randomValue * mConstrainedBricks.size());

					mTriggerElement = (int) randomValue;
				}
			
				mLastTrigger = trigger;


				int elemIndex = 0;
				for (auto iter=begin(mConstrainedBricks); iter!=end(mConstrainedBricks); ++iter, ++elemIndex)
				{
					if (LiveAnimation == false && currTime == resetTime)
					{
						iter->active = false;
						iter->localTime = 0.0;
						iter->animFactor = 0.0;
					}
					else if (iter->active==false && elemIndex == mTriggerElement)
					{
						iter->active = true;
						iter->localTime = 0.0;
						iter->animFactor = 0.0;

						mTriggerElement = -1;
					}
					else if (iter->active == true)
					{
						iter->localTime += deltaTime;
						iter->animFactor = iter->localTime / animTime;
						iter->animFactor = clamp01(iter->animFactor);

						if (true == TurnOffAfterProcessing && 1.0 == iter->animFactor)
						{
							iter->active = false;
							iter->localTime = 0.0;
							iter->animFactor = 0.0;
						}
					}
				}
			}
			else
			{

				// process each element ACTIVE state

				for (auto iter=begin(mConstrainedBricks); iter!=end(mConstrainedBricks); ++iter)
				{
					double creationPercent = iter->creationPercent;
					
					if (lProgressOffset != 0.0)
					{
						creationPercent += lProgressOffset;
						while (creationPercent < 0.0)
							creationPercent += 100.0;
						while (creationPercent > 100.0)
							creationPercent -= 100.0;
					}
					if (lProgressInverse)
						creationPercent = 100.0 - creationPercent;
					

					if ( (lProgress == 0.0)
						|| (iter->active == true && lProgress < creationPercent) )
					{
						iter->active = false;
						iter->localTime = 0.0;
						iter->animFactor = 0.0;
					}
				
					//

					if (iter->active == true)
					{
						iter->localTime += deltaTime;
						iter->animFactor = iter->localTime / animTime;
						iter->animFactor = clamp01(iter->animFactor);
					}
					else if (lProgress > 0.0 && lProgress >= creationPercent)
					{
						iter->active = true;
						iter->localTime = 0.0;
						iter->animFactor = 0.0;
					}
				}
			}

			FBMatrix curveTM;
			pCurve->GetMatrix(curveTM, kModelTransformation_Geometry, true, pEvaluateInfo);

			FBTVector pos, curvePos, curveDer;
			FBMatrix initTM;

			FBRVector rot(lDefRotation);
			FBSVector scaleZero(0.0, 0.0, 0.0);
			FBSVector scaleOne(lDefScale);
			FBSVector scale( scaleZero );

			FBTime totalTime(0,0,1);
			FBTime brickTime = totalTime / mConstrainedBricks.size();

			int elementIndex = 0;
			int totalElementCount = (int) mConstrainedBricks.size();

			FBTime currBrickTime(0);
			for(auto iter=begin(mConstrainedBricks); iter!=end(mConstrainedBricks); ++iter, currBrickTime+=brickTime, ++elementIndex)
			{
				if (mEvalCurve)
				{
					double creationPercent = iter->creationPercent;
				
					curvePos = pCurve->Total_LocalPathEvaluate( creationPercent );
					curveDer = pCurve->Total_LocalPathEvaluateDerivative( creationPercent );
					
					double angle = 90.0 + M_RAD2DEG * atan2(curveDer[0], curveDer[2]);
					rot = FBRVector(0.0, angle, 0.0);

					FBTRSToMatrix( iter->curveTM, curvePos, rot, scaleOne );
				}
				else if (mIsDistToCamUsed)
				{
					curvePos = FBTVector( iter->curveTM[12], iter->curveTM[13], iter->curveTM[14], 1.0 );
				}

				FBMatrixMult( initTM, curveTM, iter->initTM );

				FBMatrix rootTM;
				FBMatrixMult( rootTM, curveTM, iter->curveTM );
				
				FBMatrix animTM;
				animTM.Identity();

				double distToCamera = 0.0;
				if (mIsDistToCamUsed)
				{
					FBTVector dif;
					FBSub(dif, curvePos, FBTVector(cameraPos[0], cameraPos[1], cameraPos[2], 1.0) );
					distToCamera = FBLength(dif);
				}

				if (iter->active && !(ShowOnlyWhileProcessing && iter->animFactor==1.0))
				{
					//mAnimation.Evaluate(animTM, iter->animFactor);

					if (IsBrickCached(*iter, iter->animFactor, distToCamera) )
					{
						GetBrickCache( *iter,
							pos[0], pos[1], pos[2],
							rot[0], rot[1], rot[2],
							scale[0], scale[1], scale[2] );
					}
					else
					{

						mExpression.ExpressionValue( elementIndex, totalElementCount, iter->randomValue, iter->animFactor,
							distToCamera,
							pos[0], pos[1], pos[2],
							rot[0], rot[1], rot[2],
							scale[0], scale[1], scale[2] );

						CacheBrick(*iter, iter->animFactor, distToCamera,
							pos[0], pos[1], pos[2],
							rot[0], rot[1], rot[2],
							scale[0], scale[1], scale[2] );
					}

					if (UseSizeCurve)
					{
						double sizefactor = 1.0;

						SizeCurveHolder.GetAnimationNode()->Evaluate( &sizefactor, currBrickTime );
						scale[1] *= 0.01 * sizefactor;
					}

					FBTRSToMatrix( animTM, pos, rot, scale );

				}
				else
				{
					FBScalingToMatrix( animTM, scaleZero );
				}

				if (AutoAlignRoot == false)
				{
					FBVector3d temp(rootTM[12], rootTM[13], rootTM[14]);
				
					rootTM = initTM;
				
					rootTM[12] = temp[0];
					rootTM[13] = temp[1];
					rootTM[14] = temp[2];
				}
				
				FBMatrixMult( animTM, rootTM, animTM );
				FBMatrixToTRS( pos, rot, scale, animTM );

				if (iter->posNode)
					iter->posNode->WriteData( pos, pEvaluateInfo );
				if (iter->rotNode)
					iter->rotNode->WriteData( rot, pEvaluateInfo );
				if (iter->scaleNode)
					iter->scaleNode->WriteData( scale, pEvaluateInfo );
			}

			if ( StaticCurve == true)
				mEvalCurve = false;
		}
	}
	mPosition = lPosition;

	pConnector->DisableIfNotWritten(pEvaluateInfo);

	return true;
}

