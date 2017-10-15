
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: facialRetargeting_association.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "facialRetargeting_association.h"
#include "facialRetargeting_constraint.h"

#include <Windows.h>

FBApplyManagerRuleImplementation(FacialRetargetingAssociation);
FBRegisterApplyManagerRule( FacialRetargetingAssociation, "FacialRetargetingAssociation", "Apply Manager Rule for context menu on FacialRetargeting constraint");


/////////////////////////////////////////////////////////////////////////////////////////////////
//

/** Return true if pSrc is the type of custom object which this rule to be defined for. 
*
*    @warning you should only return true if pSrc is the type of custom object you defined.
*/
bool FacialRetargetingAssociation::IsValidSrc( FBComponent* pSrc) 
{
	return FBIS(pSrc, ConstraintFacialRetargeting);
}

/** Return true if pDst is the type of custom or MB object which your custom object 
*    pSrc will be applied on. it's possible to return proper destination and return it by pDst. 
*    For example, pSrc is RagDoll Property, pDst is a one of IK/FK bone, then you can find
*    associated Character and replace.
*
*    \param    pSrc    the custom type of object you drag it to attach another object.
*    \retval    pDst    one of the selected destinations, it could be modified and return proper destination.
*    \retval    pAllowMultiple    return true if allow multiple objects of same type as pSrc to be connected
*                   to pDst.
*    \return            return true if connection between pSrc and pDst is allowed.
*/
bool FacialRetargetingAssociation::IsValidConnection( FBComponent* pSrc, FBComponent*& pDst, bool& pAllowMultiple)
{
	pAllowMultiple = true;

	if (FBIS(pSrc, ConstraintFacialRetargeting) && FBIS(pDst, FBCamera))
	{
		FBGenericMenu *pMenu = new FBGenericMenu();
		auto cammouth = pMenu->InsertLast( "Use Camera For A Mouth", 0 );
		auto camnose = pMenu->InsertLast( "Use Camera For A Nose", 1 );

		auto camlefteye = pMenu->InsertLast( "Use Camera For A Left Eye", 2 );
		auto camrighteye = pMenu->InsertLast( "Use Camera For A Right Eye", 3 );

		POINT p;
		if (TRUE == GetCursorPos(&p) )
		{
			auto pMenuItem = pMenu->Execute(p.x, p.y);
			if (pMenuItem != nullptr)
			{
				// do some action if user select something in a menu
				ConstraintFacialRetargeting *pcon = (ConstraintFacialRetargeting*) pSrc;

				if (pMenuItem == cammouth)
					pcon->AssignCameraMouth( (FBCamera*) pDst );
				else if (pMenuItem == camnose)
					pcon->AssignCameraNose( (FBCamera*) pDst );
				else if (pMenuItem == camlefteye)
					pcon->AssignCameraEyeLeft( (FBCamera*) pDst );
				else if (pMenuItem == camrighteye)
					pcon->AssignCameraEyeRight( (FBCamera*) pDst );
			}
		}

		delete pMenu;

		return false;
	}
	else if (FBIS(pSrc, ConstraintFacialRetargeting) && FBIS(pDst, FBModel))
	{
		FBGenericMenu *pMenu = new FBGenericMenu();
		auto srcmenu = pMenu->InsertLast( "Use Item As A Source", 0 );
		auto dstmenu = pMenu->InsertLast( "Use Item As A Destination", 1 );
		pMenu->InsertLast( "", 2 );
		auto headbacking = pMenu->InsertLast( "Use Item As A Head Backing", 3 );
		auto nosebacking = pMenu->InsertLast( "Use Item As A Nose Backing", 4 );
		auto eyesbacking = pMenu->InsertLast( "Use Item As An Eye Backing", 5 );
		auto jawbacking = pMenu->InsertLast( "Use Item As A Jaw Backing", 6 );
		auto lefteyecol = pMenu->InsertLast( "Use Item As A Left Eye Collision", 7 );
		auto righteyecol = pMenu->InsertLast( "Use Item As A Right Eye Collision", 8 );

		POINT p;
		if (TRUE == GetCursorPos(&p) )
		{
			auto pMenuItem = pMenu->Execute(p.x, p.y);
			if (pMenuItem != nullptr)
			{
				// do some action if user select something in a menu
				ConstraintFacialRetargeting *pcon = (ConstraintFacialRetargeting*) pSrc;

				if (pMenuItem == srcmenu)
					pcon->AssignSource( (FBModel*)pDst );
				else if (pMenuItem == dstmenu)
					pcon->AssignDestination( (FBModel*) pDst );
				else if (pMenuItem == headbacking)
					pcon->AssignHeadBacking( (FBModel*) pDst);
				else if (pMenuItem == jawbacking )
					pcon->AssignJawBacking( (FBModel*) pDst );
				else if (pMenuItem == nosebacking )
					pcon->AssignNoseBacking( (FBModel*) pDst );
				else if (pMenuItem == eyesbacking )
					pcon->AssignEyesBacking( (FBModel*) pDst );
				else if (pMenuItem == lefteyecol)
					pcon->AssignLeftEyeCollision( (FBModel*) pDst );
				else if (pMenuItem == righteyecol )
					pcon->AssignRightEyeCollision( (FBModel*) pDst );
			}
		}

		delete pMenu;

		return false;
	}

	return false;
}


bool FacialRetargetingAssociation::MenuBuild( FBAMMenu* pAMMenu, FBComponent* pFocusedObject)
{
	if(pFocusedObject)
	{
		if( pFocusedObject->Is(ConstraintFacialRetargeting::TypeInfo) )
		{
			ConstraintFacialRetargeting *pcon = (ConstraintFacialRetargeting*) pFocusedObject;

			pAMMenu->AddOption("", -1);
			pAMMenu->AddOption("Characterize", mCharacterizeId, true);
			pAMMenu->AddOption("Grab Positions", mGrabPositionsId, true);

			//mMappingId = pAMMenu->AddOption("Mapping", -1, true);
			pAMMenu->AddOption("", -2);
			pAMMenu->AddOption("Clear Mapping", mClearMappingId, true);
			pAMMenu->AddOption("Load Mapping", mLoadMappingId, true);
			pAMMenu->AddOption("Save Mapping", mSaveMappingId, true);

			//mSelectId = pAMMenu->AddOption( "Select Cameras", -1, true);
			pAMMenu->AddOption("", -3);
			pAMMenu->AddOption("Select Mouth Camera", mSelectMouthId, pcon->CameraMouth.GetCount() > 0);
			pAMMenu->AddOption("Select Nose Camera", mSelectNoseId, pcon->CameraNose.GetCount() > 0);
			pAMMenu->AddOption("Select Left Eye Camera", mSelectLeftEyeId, pcon->CameraEyeLeft.GetCount() > 0);
			pAMMenu->AddOption("Select Right Eye Camera", mSelectRightEyeId, pcon->CameraEyeRight.GetCount() > 0);
			pAMMenu->AddOption("Apply Camera Settings", mApplyCameraSettingsId, 
				(pcon->CameraMouth.GetCount() > 0 
				|| pcon->CameraNose.GetCount() > 0 
				|| pcon->CameraEyeLeft.GetCount() > 0
				|| pcon->CameraEyeRight.GetCount() > 0));

			pAMMenu->AddOption("", -4);

			pAMMenu->AddOption("Show/Hide Backing Geometry", mShowHideBackingId, true);
			return true;
		}
	}
	return false;
}

bool FacialRetargetingAssociation::MenuAction( int pMenuId, FBComponent* pFocusedObject)
{
	ConstraintFacialRetargeting *pcon = (ConstraintFacialRetargeting*) pFocusedObject;

	//
	switch(pMenuId)
	{
	case mCharacterizeId: 
		pcon->DoCharacterize( (pcon->Active) ? false : true );
		break;

	case mGrabPositionsId:
		pcon->DoGrabPositions();
		break;

	case mClearMappingId:
		pcon->DoMappingClear();
		break;

	case mLoadMappingId:
		pcon->DoMappingLoad();
		break;

	case mSaveMappingId:
		pcon->DoMappingSave();
		break;

	case mSelectMouthId:
		pcon->DoSelectCameraMouth();
		break;

	case mSelectNoseId:
		pcon->DoSelectCameraNose();
		break;

	case mSelectLeftEyeId:
		pcon->DoSelectCameraEyeLeft();
		break;

	case mSelectRightEyeId:
		pcon->DoSelectCameraEyeRight();
		break;

	case mApplyCameraSettingsId:
		pcon->DoApplyCameraSettings();
		break;

	case mShowHideBackingId:
		pcon->ToggleBakingGeometry();
		break;

	default:
		return false;
	}

	
	return true;
}