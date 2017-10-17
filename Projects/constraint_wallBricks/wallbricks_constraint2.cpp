
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: wallbricks_constraint2.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "wallbricks_constraint.h"
#include "curveEditor_popup.h"
#include "IO\tinyxml.h"

#define	ORCONSTRAINTWALLBRICKSTEMP__CLASS		ORCONSTRAINTWALLBRICKSTEMP__CLASSNAME
#define ORCONSTRAINTWALLBRICKSTEMP__NAME		"Wall Bricks Temp"
#define ORCONSTRAINTWALLBRICKSTEMP__LABEL		"Wall Bricks Temp"
#define ORCONSTRAINTWALLBRICKSTEMP__DESC		"Wall Bricks Temp"

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


/////////////////////////////////////////////////////////////////////////////////////////////////////
// WALL BRICKS CONSTRAINT


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

void ORConstraintWallBricks::ActionAbout(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p && value)
	{
		p->OnAboutClick();
	}
}

void ORConstraintWallBricks::ActionHelp(HIObject pObject, bool value)
{
	ORConstraintWallBricks *p = FBCast<ORConstraintWallBricks>(pObject);
	if (p && value)
	{
		p->OnHelpClick();
	}
}

//////////////////////
//


void ORConstraintWallBricks::DoReset()
{
	for (auto iter=begin(mConstrainedBricks); iter!=end(mConstrainedBricks); ++iter)
	{
		iter->active = false;
		iter->localTime = 0.0;
		iter->animFactor = 0.0;
	}
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

void ORConstraintWallBricks::SetExpressionsToDefault()
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

void ORConstraintWallBricks::OnExpressionResetClick()
{
	if (1 == FBMessageBox( "Wall Bricks Constraint", "Do you want to reset expressions?", "Yes", "Cancel" ) )
	{
		SetExpressionsToDefault();	
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


void ORConstraintWallBricks::OnAboutClick()
{
	FBMessageBox( "Wall Bricks Constraint", 
		"MoPlugs Project (c) 2016-2017\n" 
		"Created by Sergey Solohin (Neill3d)\n"
		"  e-mail to: s@neill3d.com\n\n"
		" Special for Cam Christiansen", "Ok" );
}

void ORConstraintWallBricks::OnHelpClick()
{
	FBMessageBox( "Wall Bricks Constraint",
		"Please visit a MoPlugs/Documentation folder", "Ok" );
}