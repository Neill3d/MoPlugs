
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_association.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "compositeMaster_association.h"
#include "compositeMaster_common.h"
#include "compositeMaster_objectFinal.h"
#include "model_3d_decal.h"
#include "model_3d_fog.h"

FBApplyManagerRuleImplementation(CompositeAssociation);
FBRegisterApplyManagerRule( CompositeAssociation, "CompositeAssociation", "Apply Manager Rule for Composite Master plugin");


bool CompositeAssociation::IsValidSrc( FBComponent *pSrc) 
{ 
	if (pSrc == nullptr) 
		return false;

	if ( IsNodeFilter(pSrc) || IsNodeFinal(pSrc) || IsNodeSource(pSrc) || FBIS(pSrc, FBTexture) )
		return true;

	return false;
}

bool CompositeAssociation::IsValidConnection( FBComponent* pSrc, FBComponent*& pDst, bool& pAllowMultiple) 
{ 
	pAllowMultiple = false;

	if ( FBIS(pDst, ModelDecal) )
	{
		if (FBIS(pSrc, FBTexture) )
		{
			FBConnect( pSrc, &( (ModelDecal*) pDst)->Texture );
			pAllowMultiple = false;
			return false;
		}
	}

	if ( IsNodeFinal(pSrc) )
	{
		if (FBIS(pDst, FBCamera) )
		{
			FBConnect(pDst, &( (ObjectComposition*) pSrc)->Cameras );
			pAllowMultiple = true;
			return false;
		}
		else if (FBIS(pDst, FBLight) )
		{
			FBConnect(pDst, &( (ObjectComposition*) pSrc)->Lights );
			pAllowMultiple = true;
			return false;
		}
		else if (FBIS(pDst, FBGroup) )
		{
			FBConnect(pDst, &( (ObjectComposition*) pSrc)->Groups );
			pAllowMultiple = true;
			return false;
		}
	}
	else if ( IsNodeFinal(pDst) && (IsNodeSource(pSrc) || IsNodeFilter(pSrc)) )
	{
		pAllowMultiple = true;
		return true;
	}
	else if ( IsNodeFinal(pDst) && FBIS(pSrc, FBTexture) )
	{
		FBConnect( pSrc, ( (ObjectComposition*) pDst)->GetCompositeBackgroundPtr()->BackgroundTexture );
		/*
		FBVideo *pVideo = ((FBTexture*)pSrc)->Video;

		// connect for batch processing
		if (pVideo)
		{
			FBConnect(pVideo, &( (ObjectComposition*) pDst)->BatchInput );
			pAllowMultiple = true;
		}
		*/
		return false;
	}
	else if ( IsNodeFinal(pDst) && FBIS(pSrc, FBVideo) )
	{
		FBConnect(pSrc, &( (ObjectComposition*) pDst)->BatchInput );
		pAllowMultiple = true;
		return false;
	}
	else if ( IsNodeSource(pDst) && IsNodeFilter(pSrc) )
	{
		pAllowMultiple = true;
		return true;
	}
	return false;
}

bool CompositeAssociation::MenuBuild( FBAMMenu* pAMMenu, FBComponent* pFocusedObject)
{
	mEditManipulatorId = -1;
	mDuplicateId = -1;
	/*
	if(pFocusedObject)
	{
		if( pFocusedObject->Is(FBModel::TypeInfo) && (((FBModel*)pFocusedObject)->Geometry != nullptr) )
		{
			pAMMenu->AddOption( "" );
			mEditManipulatorId = pAMMenu->AddOption("Connect Camera As Projector", -1, true);
			//mCloseManipulatorId = pAMMenu->AddOption("Close sculpt brush", -1, true);
			pAMMenu->AddOption( "" );
			return true;
		}
	}
	*/

	if (pFocusedObject)
	{
		if (FBIS(pFocusedObject, ModelDecal) || FBIS(pFocusedObject, ModelFogTarget)
			|| FBIS(pFocusedObject, ModelFogVolume) )
		{
			mDuplicateId = pAMMenu->AddOption( "Duplicate", -1, true );
			return true;
		}
	}

	return false;
}

bool CompositeAssociation::MenuAction( int pMenuId, FBComponent* pFocusedObject)
{
	bool result = false;

	if (pMenuId == -1)
		return result;

	if (pMenuId == mDuplicateId)
	{
		FBModel *pNewModel = ( (FBModel*)pFocusedObject)->Clone();

		// DONE: should be repeat the source model connections ?!
		//pNewModel->DisconnectAllDst();
		for (int i=0, count=pFocusedObject->GetDstCount(); i<count; ++i)
		{
			FBPlug *pPlug = pFocusedObject->GetDst(i);
			if ( FBIS(pPlug, FBProperty) )
			{
				FBConnect(pNewModel, pPlug);
			}
		}

		if (pNewModel)
		{
			pFocusedObject->Selected = false;
			pNewModel->Selected = true;
		}
	}

	/*
	if ( (mEditManipulatorId >= 0) && (pMenuId == mEditManipulatorId) )
	{
		printf("connect camera\n");
	}
	*/
	return result;
}