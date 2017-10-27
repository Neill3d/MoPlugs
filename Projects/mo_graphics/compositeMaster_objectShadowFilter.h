
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectShadowFilter.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "compositeMaster_object.h"

#include <map>
#include "algorithm\nv_math.h"
#include "graphics\OGL_Utils.h"
#include "graphics\UniformBuffer.h"

#define FILTER3DSHADOW__CLASSNAME				ObjectFilter3dShadow
#define FILTER3DSHADOW__CLASSSTR				"ObjectFilter3dShadow"
#define FILTER3DSHADOW__ASSETNAME				"3d Shadow Filter"

#define FILTER3DSHADOW__LABEL					"3d Shadow"
#define FILTER3DSHADOW__SHADERPATH				"\\GLSL_CS\\FilterShadow.cs"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 3D SHADOW - post precessing shadow mapping

//////////////////////////////////////////////////////////////////////
//
class ObjectFilter3dShadow : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilter3dShadow, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilter3dShadow);

public:
	ObjectFilter3dShadow(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertiesToPropertyViewManager();

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

public:
	
	FBPropertyAnimatableColor		Color;		//!< main shadow color
	FBPropertyAnimatableDouble		Density;	//!< shadow density
	FBPropertyAnimatableDouble		Feather;

	FBPropertyBaseEnum<ECompositeBlendType>	BlendMode;		//!< type of blending two images

	FBPropertyAction				CreateZone;	// create a new shadow volume

	FBPropertyBool					UseModelProperties;	// color, composite mode ?!
	FBPropertyListObject			ZoneObjects;		// could be multiple objects with multipass effect applying
	
	FBPropertyBool					SortZones;

	FBPropertyAction				UpdateAllZones;

	static void SetCreateZone(HIObject object, bool value);
	static void SetUpdateAllZonesAction(HIObject object, bool value);

protected:

	virtual const char *MixedProgramLabel() override {
		return FILTER3DSHADOW__LABEL;
	}
	virtual const char *MixedProgramPath() override {
		return FILTER3DSHADOW__SHADERPATH;
	}
	virtual void OnSetProgramDefines( const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, FBString &defineLine ) override
	{
		const ECompositeBlendType currMode = BlendMode;

		FBString strBlendMode = CompositeBlendTypeToString(currMode);
		defineLine += strBlendMode;

		if (pInfo->IsShadowMultisampling() )
		{
			defineLine += "#define ENABLE_MS\n";
		}

		/*
		const int volumeCount = ComputeNumberOfVolumeModels();
		if (volumeCount > 0)
		{
			defineLine += "#define USE_VOLUME\n";
		}
		*/
	}

	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override 
	{
		if (stage == eBeforeProgramDispatch)
		{
			pGPUFBScene = &CGPUFBScene::instance();

			PrepShadowData(pInfo);
			mGPUShadowBuffer.Bind(0);

			const int volumeCount = ComputeNumberOfZoneModels();
			if (volumeCount > 0)
			{
				PrepAllZones( ((CCompositionInfo*)pInfo)->GetRenderCamera() );
				mGPUZoneBuffer.Bind(1);

				glActiveTexture(GL_TEXTURE3);
#ifdef SHADOW_MSAA
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, pGPUFBScene->GetShadowTextureId() );
#else
				glBindTexture(GL_TEXTURE_2D_ARRAY, pGPUFBScene->GetShadowTextureId() );
#endif
				// prep subroutines for support each volume unique blend mode
				// set subroutine values
				
				glUniformSubroutinesuiv( GL_COMPUTE_SHADER, GLsizei(25), &mSubroutineIndex[0] );
			}
		}
		else if (stage == eAfterProgramDispatch)
		{
			mGPUShadowBuffer.UnBind();
		}
	}

protected:

	struct ShadowData
	{
		vec4		color;

		float		density;
		float		feather;
		float		fnear;
		float		ffar;
	
		vec4		zoneInfo;	// x - number of volume models connected
	};

	struct ZoneData
	{
		float		density;
		float		feather;
		float		blendMode;
		float		Id;
	
		vec4		color;
	
		vec4		volumeMin;
		vec4		volumeMax;
		mat4		lightMatrix;
		mat4		invMatrix;
	};

	// gpu memory buffer
	ShadowData						mShadowData;
	std::vector<ZoneData>			mZoneData;

	CGPUBufferSSBO		mGPUShadowBuffer;
	CGPUBufferSSBO		mGPUZoneBuffer;

	// intermid buffers for processing many volumes inside one filter
	CompositeComputeShader::CComputeTextureBuffer<2>	mVolumeTextureBuffer;

	// for blend mode subroutines
	GLuint	mSubroutineIndex[25];

	void		PrepShadowData(const CCompositionInfo *pInfo);
	void		PrepZoneData(ZoneData &data, FBModel *pZoneModel, const bool useModelProperties);
	void		PrepAllZones(FBCamera *pCamera);

protected:

	FBSystem					mSystem;
	
	CGPUFBScene					*pGPUFBScene;

	std::vector<std::pair<FBModel*, double>>		mZoneSortVector;
	
	// list of all combination of blend modes for volume models
	std::vector<CompositeComputeShader::CMixedProgram*>		mZoneProgram;

	const int ComputeNumberOfZoneModels();
	void	SortZoneModels(FBCamera *pCamera, const bool sorting);

	void SetUpAlphaUniform();
	void SetUpObjectUniforms(FBCamera *pCamera, const bool useZoneObject);
	void SetUpZoneUniforms(FBCamera *pCamera, FBModel *pModel);

	void DoCreateZone();
	void DoUpdateAllZones();
};