
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: applymanagerrule.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "applymanagerrule.h"
#include "StringUtils.h"
#include "ProjTex_shader.h"

FBApplyManagerRuleImplementation(MoGraphicsAssociation);
FBRegisterApplyManagerRule( MoGraphicsAssociation, "MoGraphicsAssociation", "Apply Manager Rule for MoGraphics plugin");


bool MoGraphicsAssociation::IsValidSrc( FBComponent *pSrc) 
{ 
	if (pSrc == nullptr) 
		return false;

	//const char *modelName = pSrc->Name;

	return false;
}

bool MoGraphicsAssociation::IsValidConnection( FBComponent* pSrc, FBComponent*& pDst, bool& pAllowMultiple) 
{ 
//	const char *srcName = pSrc->Name;
//	const char *dstName = pDst->Name;

	return false;
}

bool MoGraphicsAssociation::MenuBuild( FBAMMenu* pAMMenu, FBComponent* pFocusedObject)
{
	mCopyTextureFilenameId = -1;
	mRevealTextureFilenameId = -1;

	mCopyVideoFilenameId = -1;
	mRevealVideoFilenameId = -1;

	mReplaceProjTexId = -1;

	mEditManipulatorId = -1;
	if(pFocusedObject)
	{
		if( pFocusedObject->Is(FBTexture::TypeInfo) )
		{
			FBTexture *pTexture = (FBTexture*) pFocusedObject;
			FBVideo *pVideo = pTexture->Video;

			if (FBIS(pVideo, FBVideoClip) )
			{
				FBVideoClip *pVideoClip = (FBVideoClip*) pVideo;
				FBString filename = pVideoClip->Filename;

				if (filename.GetLen() > 0)
				{
					pAMMenu->AddOption( "" );
					mCopyTextureFilenameId = pAMMenu->AddOption("Copy Filename to Clipboard", -1, true);
					mRevealTextureFilenameId = pAMMenu->AddOption("Reveal Filename in explorer", -1, true);
					pAMMenu->AddOption( "" );
				}
			}

			return true;
		}
		else if (pFocusedObject->Is(FBVideoClip::TypeInfo) )
		{
			FBVideoClip *pVideoClip = (FBVideoClip*) pFocusedObject;
			FBString filename = pVideoClip->Filename;

			if (filename.GetLen() > 0)
			{
				pAMMenu->AddOption( "" );
				mCopyVideoFilenameId = pAMMenu->AddOption("Copy Filename to Clipboard", -1, true);
				mRevealVideoFilenameId = pAMMenu->AddOption("Reveal Filename in explorer", -1, true);
				pAMMenu->AddOption( "" );
			}
		}
		else if (pFocusedObject->Is(FBModel::TypeInfo) )
		{
			pAMMenu->AddOption( "" );
			mReplaceProjTexId = pAMMenu->AddOption( "Replace With Projective Mapping Shader", -1, true );
			pAMMenu->AddOption( "" );
		}
	}
	return false;
}



bool MoGraphicsAssociation::MenuAction( int pMenuId, FBComponent* pFocusedObject)
{
	bool result = false;

	if (pMenuId == -1)
		return result;

	if (pMenuId == mCopyTextureFilenameId)
	{
		FBTexture *pTexture = (FBTexture*) pFocusedObject;
		FBVideo *pVideo = pTexture->Video;

		if (FBIS(pVideo, FBVideoClip) )
		{
			FBVideoClip *pVideoClip = (FBVideoClip*) pVideo;
			CopyVideoClipFilename(pVideoClip);
		}

		result = true;
	}
	else if (pMenuId == mRevealTextureFilenameId)
	{
		FBTexture *pTexture = (FBTexture*) pFocusedObject;
		FBVideo *pVideo = pTexture->Video;

		if (FBIS(pVideo, FBVideoClip) )
		{
			FBVideoClip *pVideoClip = (FBVideoClip*) pVideo;
			RevealVideoClipFilename(pVideoClip);
		}

		result = true;
	}
	else if (pMenuId == mCopyVideoFilenameId)
	{
		CopyVideoClipFilename( (FBVideoClip*) pFocusedObject );
		result = true;
	}
	else if (pMenuId == mRevealVideoFilenameId)
	{
		RevealVideoClipFilename( (FBVideoClip*) pFocusedObject );
		result = true;
	}
	else if (pMenuId == mReplaceProjTexId)
	{
		FBShader *pNewShader = (FBShader*) FBCreateObject( PROJTEX__ASSETPATH, PROJTEX__DESC, "Projective Mapping" );
		if (pNewShader)
		{
			FBModel *pModel = (FBModel*) pFocusedObject;
			pModel->Shaders.RemoveAll();
			pModel->Shaders.Add(pNewShader);

			pNewShader->HardSelect();
		}
	}

	return result;
}

void MoGraphicsAssociation::CopyVideoClipFilename(FBVideoClip *pVideoClip)
{
}

void MoGraphicsAssociation::RevealVideoClipFilename(FBVideoClip *pVideoClip)
{
	FBString filename = pVideoClip->Filename;
	FBString filepath = ExtractFilePath(filename);

	if (filename.GetLen() > 0)
	{
		FBString cmdline( "explorer ", filepath );
		system(cmdline);
	}
}