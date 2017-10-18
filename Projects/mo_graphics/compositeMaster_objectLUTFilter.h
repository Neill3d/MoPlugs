
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectLUTFilter.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_object.h"
#include <vector>

#define FILTERLUT__CLASSNAME				ObjectFilterLUT
#define FILTERLUT__CLASSSTR					"ObjectFilterLUT"
#define FILTERLUT__ASSETNAME				"3d LUT Filter"

/////////////////////////////////////////////////////////////////////////////////////////
//
enum EFilterLUTPreset
{
	eFilterLUT_Custom,
	eFilterLUT_File1,
	eFilterLUT_File2
};

const char * FBPropertyBaseEnum<EFilterLUTPreset>::mStrings[] = {
	"Custom",
	"File1",
	"File2",
	0};

/////////////////////////////////////////////////////////////////////////////////
// FilterLUT

class ObjectFilterLUT : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilterLUT, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilterLUT);

public:
	//! a constructor
	ObjectFilterLUT(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

public:
	
	FBPropertyInt							LutSize;	//!< 3d texture edge size

	FBPropertyString						FileName;	//!< 3d LUT file path
	FBPropertyAction						Load;		//!< choose a custom LUT file

public:

	// render to buffers with render tree
	
	virtual bool ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo) override;
	
	void Cleanup();

	void DoLoad();

protected:

	virtual const char *MixedProgramLabel() override {
		return "3d LUT";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterLUT.cs";
	}
	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override 
	{
		if (stage == eBeforeProgramDispatch && mLut3dTexID > 0)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_3D, mLut3dTexID);
			glActiveTexture(GL_TEXTURE0);
		}
	}

protected:

	GLuint		mLut3dTexID;
	int			mLutEdgeSize;

	bool		mNeedUpdate;

	int			mLutEdgeSizeCache;
	std::string			mLutFileLoaded;
	std::string			mLut3dCacheid;
	std::string			mShaderCacheid;
	std::vector<float>	mLut3d;

	std::string			mGeneratedShaderCode;

	float		mExposureFStop;
	float		mDisplayGamma;

	void	InitParams();
	void	LoadFromFile(const char *filename);

	void	AllocateLut3D();

	bool	UpdateOCIOGLState();

	static void SetLoad(HIObject object, bool value);
	static void SetEdgeSize(HIObject object, int value);


	virtual const char *AssetNameString() override {
		return FILTERLUT__ASSETNAME;
	}
};