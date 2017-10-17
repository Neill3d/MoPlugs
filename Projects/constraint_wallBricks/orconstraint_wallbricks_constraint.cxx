
/**	\file	orconstraintwallbricks_constraint.cxx
*	Definition of a simple constraint class.
*	Simple constraint function definitions for the ORConstraintWallBricks
*	class.
*/

//--- Class declarations

#include "orconstraint_wallbricks_constraint.h"
#include "curveEditor_popup.h"

#include <limits>


#include "algorithm\math3d_mobu.h"
#include "IO\tinyxml.h"

//--- Registration defines
#define	ORCONSTRAINTWALLBRICKS__CLASS		ORCONSTRAINTWALLBRICKS__CLASSNAME
#define ORCONSTRAINTWALLBRICKS__NAME		"Wall Bricks"
#define ORCONSTRAINTWALLBRICKS__LABEL		"Wall Bricks Along a curve"
#define ORCONSTRAINTWALLBRICKS__DESC		"Wall Bricks Along a curve"

#define	ORCONSTRAINTWALLBRICKSTEMP__CLASS		ORCONSTRAINTWALLBRICKSTEMP__CLASSNAME
#define ORCONSTRAINTWALLBRICKSTEMP__NAME		"Wall Bricks Temp"
#define ORCONSTRAINTWALLBRICKSTEMP__LABEL		"Wall Bricks Temp"
#define ORCONSTRAINTWALLBRICKSTEMP__DESC		"Wall Bricks Temp"

//--- implementation and registration
FBConstraintImplementation	(	ORCONSTRAINTWALLBRICKS__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINTWALLBRICKS__NAME,
								ORCONSTRAINTWALLBRICKS__CLASS,
								ORCONSTRAINTWALLBRICKS__LABEL,
								ORCONSTRAINTWALLBRICKS__DESC,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)

FBConstraintImplementation	(	ORCONSTRAINTWALLBRICKSTEMP__CLASS		);
FBRegisterConstraint		(	ORCONSTRAINTWALLBRICKSTEMP__NAME,
								ORCONSTRAINTWALLBRICKSTEMP__CLASS,
								ORCONSTRAINTWALLBRICKSTEMP__LABEL,
								ORCONSTRAINTWALLBRICKSTEMP__DESC,
								FB_DEFAULT_SDK_ICON				);	// Icon filename (default=Open Reality icon)



///////////////////////////////////////////////////////////////////////////////////////////////////
// KDTree
///////////////////////////////////////////////////////////////////////////////////////////////////

KDTree::KDTree()
{
	mTree = nullptr;
}

KDTree::~KDTree()
{
	FreeKDTree();
}

// build K-D Tree for this frame
int KDTree::BuildKDTree(int count, double *positions)
{
	FreeKDTree();
	
	if (count <= 0) return 0;

	double *ref = new double[count * 3];
	for (int i=0; i<count; i++)
	{
		ref[i] = positions[i*3];
		ref[count + i] = positions[i*3+1];
		ref[count*2 + i] = positions[i*3+2];
	}

	int *index = new int[count];
	for (int i=0; i<count; i++)	index[i] = i;
	mTree = build_kdtree( ref, count, 3, index, count, 0 );
	if (!mTree)
	{
		if(index!=nullptr)
		{
			delete [] index;
			index = nullptr;
		}
		if (ref!=nullptr)
		{
			delete [] ref;
			ref = nullptr;
		}
		return 0;
	}
	mTree->dims = 3;

	if(index!=nullptr)
	{
		delete [] index;
		index = nullptr;
	}
	if (ref!=nullptr)
	{
		delete [] ref;
		ref = nullptr;
	}
	return 1;
}

// free kd-tree
int KDTree::FreeKDTree() {
	
	if (mTree) {
		free_tree(mTree->rootptr);
		free(mTree);
		mTree = nullptr;
	}
	return 1;
}

int KDTree::SearchForNearest(const double *pos, const double dist)
{
	if (!mTree) return -1;

	double M=0.0;
	run_queries( mTree->rootptr, (double*) pos, 1, 3, &M, (double*) &dist, 102 ); // RETURN_INDEX - 102
	return (int) M;
}

int KDTree::SearchForNearest(const double *pos, const double dist, double *res)
{
	if (!mTree) return 0;

	run_queries( mTree->rootptr, (double*)pos, 1, 3, res, (double*) &dist, 101 ); // RETURN_POSITION - 102
	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////
// temp deformer constraint

bool ORConstraintWallBricksTemp::FBCreate()
{
	// Create reference groups
	mGroupDeformed	= ReferenceGroupAdd( "Deformed",	MAX_NUMBER_OF_BRICKS );
	
	Deformer = true;

	return true;
}

void ORConstraintWallBricksTemp::FBDestroy()
{
}

bool ORConstraintWallBricksTemp::ReferenceAddNotify		( int pGroupIndex, FBModel* pModel )
{
	return DeformerBind(pModel);
}

bool ORConstraintWallBricksTemp::ReferenceRemoveNotify	( int pGroupIndex, FBModel* pModel )
{
	return DeformerUnBind(pModel);
}

////////////////////////////////////////
//

void ORConstraintWallBricks::SetScriptPositionX(HIObject pObject, const char *value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->DoExpressionParse();
		p->ScriptPositionX.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::SetScriptPositionY(HIObject pObject, const char *value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->DoExpressionParse();
		p->ScriptPositionY.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::SetScriptPositionZ(HIObject pObject, const char *value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->DoExpressionParse();
		p->ScriptPositionZ.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::SetScriptRotationX(HIObject pObject, const char *value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->DoExpressionParse();
		p->ScriptRotationX.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::SetScriptRotationY(HIObject pObject, const char *value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->DoExpressionParse();
		p->ScriptRotationY.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::SetScriptRotationZ(HIObject pObject, const char *value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->DoExpressionParse();
		p->ScriptRotationZ.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::SetScriptScalingX(HIObject pObject, const char *value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->DoExpressionParse();
		p->ScriptScalingX.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::SetScriptScalingY(HIObject pObject, const char *value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->DoExpressionParse();
		p->ScriptScalingY.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::SetScriptScalingZ(HIObject pObject, const char *value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->DoExpressionParse();
		p->ScriptScalingZ.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::SetLiveAnimation(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->DoSetLiveAnimation();
		p->LiveAnimation.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::ActionFixInstances(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p && value)
	{
		p->DoFixInstances();
	}
}

void ORConstraintWallBricks::ActionSizeCurve(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p && value)
	{
		p->OnButtonSizeCurve();
	}
}

void ORConstraintWallBricks::ActionStaticCurve(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->StaticCurve.SetPropertyValue(value);
		p->OnChangeStaticCurve();
	}
}

void ORConstraintWallBricks::ActionAnimateProgress(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p && value)
	{
		p->OnAnimateProgressClick();
	}
}

void ORConstraintWallBricks::ActionExpressionReset(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p && value)
	{
		p->OnExpressionResetClick();
	}
}

void ORConstraintWallBricks::ActionExpressionLoad(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p && value)
	{
		p->OnExpressionLoadClick();
	}
}

void ORConstraintWallBricks::ActionExpressionSave(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p && value)
	{
		p->OnExpressionSaveClick();
	}
}

void ORConstraintWallBricks::SetProgressOffset(HIObject pObject, double value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->OnProgressOffsetChange();
		p->ProgressOffset.SetPropertyValue(value);
	}
}

void ORConstraintWallBricks::SetProgressInverse(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p)
	{
		p->OnProgressInverseChange();
		p->ProgressInverse.SetPropertyValue(value);
	}
}

/************************************************
 *	Creation function.
 ************************************************/
bool ORConstraintWallBricks::FBCreate()
{
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

	ScriptPositionX = "0.0";
	ScriptPositionY = "(1.0-f)*50.0";
	ScriptPositionZ = "(1.0-f)*(1.0-f)*50.0";

	ScriptRotationX = "0.0";
	ScriptRotationY = "0.0";
	ScriptRotationZ = "0.0";

	ScriptScalingX = "f";
	ScriptScalingY = "f";
	ScriptScalingZ = "f";

	StaticCurve = true;

	mExpression.ExpressionInit();

	mNeedExpressionParsing = true;
	mExpressionFailed = false;
	mIsDistToCamUsed = false;

	mEvalCurve = true;

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


void ORConstraintWallBricks::DoReset()
{
	// TODO: ...
}

void ORConstraintWallBricks::OnButtonSizeCurve()
{
	ORPopup_CurveEditor	SizeCurveEditor;

	SizeCurveEditor.Caption = FBString( Name, " - Size curve editor" );
	SizeCurveEditor.Modal = true;
	SizeCurveEditor.SetUp( nullptr, &SizeCurveHolder );
	SizeCurveEditor.Show();

	//mNeedUpdatePropertyTexture = true;
	//mParticleSystem.UpdatePropertyTextures();
}

void ORConstraintWallBricks::OnChangeStaticCurve()
{
	mEvalCurve = true;
}

void ORConstraintWallBricks::OnAnimateProgressClick()
{
	if (1 == FBMessageBox( "Wall Bricks Constraint", "Do you want to add a simple animation to the progress property?", "Yes", "Cancel") )
	{

		Progress.SetAnimated(false);
		Progress.SetAnimated(true);

		FBTime startTime = FBPlayerControl::TheOne().LoopStart;
		FBTime stopTime = FBPlayerControl::TheOne().LoopStop;

		FBAnimationNode *pnode = Progress.GetAnimationNode();

		double startValue = 0.0;
		double stopValue = 100.0;

		pnode->KeyAdd(startTime, &startValue);
		pnode->KeyAdd(stopTime, &stopValue);
	}
}

void ORConstraintWallBricks::OnProgressOffsetChange()
{
	mEvalCurve = true;
}

void ORConstraintWallBricks::OnProgressInverseChange()
{
	mEvalCurve = true;
}

void ORConstraintWallBricks::OnExpressionResetClick()
{
	if (1 == FBMessageBox( "Wall Bricks Constraint", "Do you want to reset expressions?", "Yes", "Cancel" ) )
	{
		ScriptPositionX = "0.0";
		ScriptPositionY = "(1.0-f)*50.0";
		ScriptPositionZ = "(1.0-f)*(1.0-f)*50.0";

		ScriptRotationX = "0.0";
		ScriptRotationY = "0.0";
		ScriptRotationZ = "0.0";

		ScriptScalingX = "f";
		ScriptScalingY = "f";
		ScriptScalingZ = "f";

		mNeedExpressionParsing = true;
	}
}

void ORConstraintWallBricks::OnExpressionLoadClick()
{
	FBFilePopup	lDialog;

	lDialog.Style = kFBFilePopupOpen;
	lDialog.Caption = "Choose a file to load a preset";
	lDialog.Filter = "*.xml";

	if (lDialog.Execute() )
	{
		FBString fullFilename(lDialog.FullFilename);

		// TODO: load from xml file

		TiXmlDocument	doc;

		if (doc.LoadFile( fullFilename ) == false)
		{
			return;
		}

		TiXmlNode *node = nullptr;
		TiXmlElement *element = nullptr;
		TiXmlAttribute  *attrib = nullptr;

		
		node = doc.FirstChild("Expressions");
		if (node == nullptr)
			return;

		element = node->ToElement();
		if (element)
		{
			
			// enumerate attribs
			for( attrib = element->FirstAttribute();
				 attrib;
				 attrib = attrib->Next() )
			{
				const char * attribName = attrib->Name();
				if ( strcmp(attribName, "PositionX") == 0 )
					ScriptPositionX.SetString( attrib->Value() );
				else if ( strcmp(attribName, "PositionY") == 0 )
					ScriptPositionY.SetString( attrib->Value() );
				else if ( strcmp(attribName, "PositionZ") == 0 )
					ScriptPositionZ.SetString( attrib->Value() );
				else if ( strcmp(attribName, "RotationX") == 0 )
					ScriptRotationX.SetString( attrib->Value() );
				else if ( strcmp(attribName, "RotationY") == 0 )
					ScriptRotationY.SetString( attrib->Value() );
				else if ( strcmp(attribName, "RotationZ") == 0 )
					ScriptRotationZ.SetString( attrib->Value() );
				else if ( strcmp(attribName, "ScalingX") == 0 )
					ScriptScalingX.SetString( attrib->Value() );
				else if ( strcmp(attribName, "ScalingY") == 0 )
					ScriptScalingY.SetString( attrib->Value() );
				else if ( strcmp(attribName, "ScalingZ") == 0 )
					ScriptScalingZ.SetString( attrib->Value() );
			}

			//
			mNeedExpressionParsing = true;
		}

	}
}

void ORConstraintWallBricks::OnExpressionSaveClick()
{
	FBFilePopup	lDialog;

	lDialog.Style = kFBFilePopupSave;
	lDialog.Caption = "Choose a file to save a preset";
	lDialog.Filter = "*.xml";

	if (lDialog.Execute() )
	{
		FBString fullFilename(lDialog.FullFilename);

		// save into the xml file

		TiXmlDocument doc;

		TiXmlElement	head("Header");
		head.SetAttribute( "comments", "Saved from Wall Bricks constraint" );
		head.SetAttribute( "version", 1 );

		TiXmlElement	expressions("Expressions");

		expressions.SetAttribute( "PositionX", ScriptPositionX.AsString() );
		expressions.SetAttribute( "PositionY", ScriptPositionY.AsString() );
		expressions.SetAttribute( "PositionZ", ScriptPositionZ.AsString() );

		expressions.SetAttribute( "RotationX", ScriptRotationX.AsString() );
		expressions.SetAttribute( "RotationY", ScriptRotationY.AsString() );
		expressions.SetAttribute( "RotationZ", ScriptRotationZ.AsString() );

		expressions.SetAttribute( "ScalingX", ScriptScalingX.AsString() );
		expressions.SetAttribute( "ScalingY", ScriptScalingY.AsString() );
		expressions.SetAttribute( "ScalingZ", ScriptScalingZ.AsString() );

		doc.InsertEndChild( head );
		doc.InsertEndChild( expressions );
		doc.SaveFile(fullFilename);

		if (doc.Error() )
		{
			printf( doc.ErrorDesc() );	
		}
	}
}

void ORConstraintWallBricks::DoExpressionParse()
{
	for (auto iter=begin(mConstrainedBricks); iter!=end(mConstrainedBricks); ++iter)
	{
		iter->cachedAnimFactor = -1.0;
	}

	mNeedExpressionParsing = true;
}

void ORConstraintWallBricks::DoSetLiveAnimation()
{
	mLastTime = FBTime::Infinity;
}


void ORConstraintWallBricks::DoFixInstances()
{

	FBConstraint *pConstraint = FBConstraintManager::TheOne().TypeCreateConstraint( "Wall Bricks Temp" );

	// TODO: use deform constraint to Fix that !!!

	if (pConstraint)
	{
		for (int i=0, count=ReferenceGetCount(mGroupConstrained); i<count; ++i)
		{
			FBModel *pNull = ReferenceGet(mGroupConstrained, i);

			if (pNull->Children.GetCount() > 0)
			{
				FBModel *pInstance = pNull->Children[0];
				pConstraint->ReferenceAdd(0, pInstance);
			}
		}

		pConstraint->Active = true;
		pConstraint->Active = false;
	}
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

		//FBModelPath3D *pCurve = (FBModelPath3D*) ReferenceGet(mGroupCurve, 0);
		
		int index = 1;
		for (int i=0, count=ReferenceGetCount(mGroupConstrained); i<count; ++i)
		{
			FBModel *pModel = ReferenceGet( mGroupConstrained, i ); 

			mConstrainedBricks[i].posNode = AnimationNodeInCreate	( i, pModel, ANIMATIONNODE_TYPE_TRANSLATION );
			mConstrainedBricks[i].rotNode = AnimationNodeInCreate	( i, pModel, ANIMATIONNODE_TYPE_ROTATION );
			mConstrainedBricks[i].scaleNode = AnimationNodeInCreate	( i, pModel, ANIMATIONNODE_TYPE_SCALING );

			index += 3;
		}
	}
}


/************************************************
 *	Removed all of the animation nodes.
 ************************************************/
void ORConstraintWallBricks::RemoveAllAnimationNodes()
{

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

			double step = 100.0 / (numberOfSubdivs * segmentSubdivs);

			// TODO: build k-d tree and find a closest point

			KDTree						mTree;	// k-d tree

			std::vector<FBVector3d>		positions;

			positions.resize(numberOfSubdivs * segmentSubdivs);

			percent = 0.0;
			for (size_t i=0; i<positions.size(); ++i)
			{
				pointOnCurve = pCurve->Total_GlobalPathEvaluate(percent);
				positions[i] = FBVector3d(pointOnCurve);

				percent += step;
			}

			mTree.BuildKDTree( (int) positions.size(), (double*) positions.data() );

			double lthres = SnapThreshold;

			for (int i=0, count=ReferenceGetCount(mGroupConstrained); i<count; ++i)
			{
				FBModel *pModel = ReferenceGet( mGroupConstrained, i ); 
				pModel->GetVector(pos);
				pModel->GetMatrix(mConstrainedBricks[i].initTM);
				
				FBGetLocalMatrix( mConstrainedBricks[i].initTM, curveTM, mConstrainedBricks[i].initTM );
				/*
				mConstrainedBricks[i].initTM[12] = 0.0;
				mConstrainedBricks[i].initTM[13] = 0.0;
				mConstrainedBricks[i].initTM[14] = 0.0;
				*/

				/*
				if (true == FindClosestPointOnCurve(pointOnCurve, percent, pCurve, FBVector4d(pos[0], pos[1], pos[2], 1.0), numberOfSubdivs, segmentSubdivs) )
					mConstrainedBricks[i].creationPercent = percent;
				else
					mConstrainedBricks[i].creationPercent = 0.0;
					*/

				int index = mTree.SearchForNearest( pos, lthres );
				if (index >= 0)
					mConstrainedBricks[i].creationPercent = step * index;
				else
					mConstrainedBricks[i].creationPercent = 0.0;
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

			for (auto iter=begin(mConstrainedBricks); iter!=end(mConstrainedBricks); ++iter)
			{
				if ( (lProgress == 0.0)
					|| (iter->active == true && lProgress < iter->creationPercent) )
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

					if (iter->animFactor < 0.0) iter->animFactor = 0.0;
					if (iter->animFactor > 1.0) iter->animFactor = 1.0;
				}
				else if (lProgress >= iter->creationPercent)
				{
					iter->active = true;
					iter->localTime = 0.0;
					iter->animFactor = 0.0;
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

					if (lProgressInverse)
						creationPercent = 100.0 - creationPercent;
					if (lProgressOffset != 0.0)
					{
						creationPercent += lProgressOffset;
						while (creationPercent < 0.0)
							creationPercent += 100.0;
						while (creationPercent > 100.0)
							creationPercent -= 100.0;
					}

					curvePos = pCurve->Total_LocalPathEvaluate( creationPercent );
					curveDer = pCurve->Total_LocalPathEvaluateDerivative( creationPercent );
    
					double angle = 90.0 + 180.0 / 3.1415 * atan2(curveDer[0], curveDer[2]);
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

				if (iter->active)
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

	return true;
}

