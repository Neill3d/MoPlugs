
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectFogFilter.h
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

#define FILTER3DFOG__CLASSNAME				ObjectFilter3dFog
#define FILTER3DFOG__CLASSSTR				"ObjectFilter3dFog"
#define FILTER3DFOG__ASSETNAME				"3d Fog Filter"

#define FILTER3DFOG__LABEL					"3d Fog"
#define FILTER3DFOG__SHADER					"\\GLSL_CS\\FilterFog.cs"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 3D FOG

enum EFilterFogTargetType
{
	eFilterFogTargetManual,
	eFilterFogTargetCenter,
	eFilterFogTargetNearFar,
	eFilterFogTargetVolume
};

//////////////////////////////////////////////////////////////////////
//
class ObjectFilter3dFog : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilter3dFog, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilter3dFog);

public:
	ObjectFilter3dFog(const char *pName = NULL, HIObject pObject=NULL);

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
	
	FBPropertyAnimatableColor		Color;		//!< main fog color
	FBPropertyAnimatableDouble		Density;	//!< fog density

	FBPropertyBaseEnum<ECompositeBlendType>	Mode;		//!< type of blending two images

	FBPropertyListObject			AlphaTexture;		//!< alpha channel video mask

	FBPropertyAnimatableDouble		FogNear;
	FBPropertyAnimatableDouble		FogFar;
	FBPropertyAnimatableDouble		FogFeather;

	//FBPropertyBool					UseTarget;
	FBPropertyBaseEnum<EFilterFogTargetType>	TargetType;
	FBPropertyListObject			TargetCenter;		//!< fog distance to a target object

	FBPropertyListObject			TargetNear;
	FBPropertyListObject			TargetFar;

	FBPropertyAction				CreateTarget;
	FBPropertyAction				CreateNearFarTargets;
	FBPropertyAction				CreateVolume;

	FBPropertyBool					UseModelProperties;	// color, composite mode ?!
	FBPropertyListObject			VolumeObject;		// could be multiple objects with multipass effect applying
	
	static void SetCreateTarget(HIObject object, bool value);
	static void SetCreateNearFarTargets(HIObject object, bool value);
	static void SetCreateVolume(HIObject object, bool value);

protected:

	virtual const char *MixedProgramLabel() override {
		return FILTER3DFOG__LABEL;
	}
	virtual const char *MixedProgramPath() override {
		return FILTER3DFOG__SHADER;
	}
	virtual void OnSetProgramDefines( const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, FBString &defineLine ) override
	{
		const ECompositeBlendType currMode = Mode;

		FBString strBlendMode = CompositeBlendTypeToString(currMode);
		defineLine += strBlendMode;

		if (AlphaTexture.GetCount() > 0)
			defineLine += "#define USE_ALPHA\n";

		const int volumeCount = ComputeNumberOfVolumeModels();
		if (volumeCount > 0)
		{
			defineLine += "#define USE_VOLUME\n";
		}
	}

	virtual void OnApplyFilter(EApplyFilterStage stage, const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, CompositeFinalStats &stats, const GLuint sourceTexId, const GLuint dstTexId) override 
	{
		if (stage == eBeforeProgramDispatch)
		{
			PrepFogData(pInfo);
			mGPUFogBuffer.Bind(0);

			if (AlphaTexture.GetCount() > 0)
			{
				FBTexture *pTexture = (FBTexture*) AlphaTexture.GetAt(0);
				glActiveTexture(GL_TEXTURE0);
				pTexture->OGLInit();
				GLuint texId = pTexture->TextureOGLId;

				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, texId);
			}

			const int volumeCount = ComputeNumberOfVolumeModels();
			if (volumeCount > 0)
			{
				PrepAllVolumes( ((CCompositionInfo*)pInfo)->GetRenderCamera() );
				mGPUVolumeBuffer.Bind(1);

				// prep subroutines for support each volume unique blend mode
				// set subroutine values
				
				glUniformSubroutinesuiv( GL_COMPUTE_SHADER, GLsizei(25), &mSubroutineIndex[0] );
			}
		}
		else if (stage == eAfterProgramDispatch)
		{
			mGPUFogBuffer.UnBind();
		}
	}

protected:

	struct FogData
	{
		vec4		color;

		float		density;
		float		feather;
		float		fnear;
		float		ffar;
	
		vec4		volumeInfo;	// x - number of volume models connected

		vec4		viewDir[4];
		vec4		viewOrig[4];
	};

	struct VolumeData
	{
		float		density;
		float		feather;
		float		blendMode;
		float		temp2;
	
		vec4		color;
	
		vec4		volumeMin;
		vec4		volumeMax;
		mat4		invVolumeMatrix;
	};

	// gpu memory buffer
	FogData							mFogData;
	std::vector<VolumeData>			mVolumeData;

	CGPUBufferSSBO		mGPUFogBuffer;
	CGPUBufferSSBO		mGPUVolumeBuffer;

	// intermid buffers for processing many volumes inside one filter
	CompositeComputeShader::CComputeTextureBuffer<2>	mVolumeTextureBuffer;

	// for blend mode subroutines
	GLuint	mSubroutineIndex[25];

	void		PrepFogData(const CCompositionInfo *pInfo);
	void		PrepVolumeData(VolumeData &data, FBModel *pVolumeModel, const bool useVolumeProperties);
	void		PrepAllVolumes(FBCamera *pCamera);
protected:

	FBSystem					mSystem;
	
	std::vector<std::pair<FBModel*, double>>		mVolumesSortVector;
	
	// list of all combination of blend modes for volume models
	std::vector<CompositeComputeShader::CMixedProgram*>		mVolumeProgram;

	const int ComputeNumberOfVolumeModels();
	void	SortVolumeModels(FBCamera *pCamera);

	void SetUpAlphaUniform();
	void SetUpObjectUniforms(FBCamera *pCamera, const bool useVolumeObject);
	void SetUpVolumeUniforms(FBCamera *pCamera, FBModel *pModel);

	void DoCreateTarget();
	void DoCreateNearFarTargets();
	void DoCreateVolume();

};