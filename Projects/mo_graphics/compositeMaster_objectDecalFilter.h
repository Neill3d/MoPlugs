
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectDecalFilter.h
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

#define FILTER3DDECAL__CLASSNAME				ObjectFilter3dDecal
#define FILTER3DDECAL__CLASSSTR					"ObjectFilter3dDecal"
#define FILTER3DDECAL__ASSETNAME				"3d Decal Filter"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// 3D DECAL

#define MAX_DECALS_COUNT		8

//////////////////////////////////////////////////////////////////////
//
class ObjectFilter3dDecal : public ObjectCompositeFilter
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(ObjectFilter3dDecal, ObjectCompositeFilter)
	FBDeclareUserObject(ObjectFilter3dDecal);

public:
	ObjectFilter3dDecal(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

	/** Override to reuse regular marker object type's property viewSet.
    */
    virtual bool UseCustomPropertyViewSet() const override { return true; }
	static void AddPropertyView(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false);
	static void AddPropertiesToPropertyViewManager();

	virtual bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;
	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

public:
	
	FBPropertyBaseEnum<ECompositeBlendType>		BlendMode;		//!< type of blending two images

	FBPropertyBool					UseModelProperties;	// color, composite mode ?!
	FBPropertyListObject			DecalObjects;		// could be multiple objects with multipass effect applying
	
	// for decal creation, let's specify final render resolution
	//  when we draw decal, we should downscale from original texture
	FBPropertyBool					UseGlobalResolution;
	FBPropertyInt					CustomWidth;
	FBPropertyInt					CustomHeight;

	FBPropertyBool					RunPhotoshopOnCreation;
	FBPropertyString				PhotoshopPath;

	FBPropertyAction				CreateDecal;	//!< create a decal from current camera view and input image
	FBPropertyAction				RefreshTexture;

	FBPropertyAction				GrabImage;	//!< save to file an incoming composition texture data

	static void SetCreateDecal(HIObject object, bool value);
	static void SetRefreshTexture(HIObject object, bool value);
	static void SetGrabImage(HIObject object, bool value);
	//static void SetResolution(HIObject object, EDecalResolution value);

protected:

	virtual const char *MixedProgramLabel() override {
		return "3d Decal";
	}
	virtual const char *MixedProgramPath() override {
		return "\\GLSL_CS\\FilterDecal.cs";
	}
	virtual void OnSetProgramExtension( FBString &strHeader ) override
	{
		strHeader += "#extension GL_ARB_bindless_texture : enable\n";
	}
	virtual void OnSetProgramDefines( const CCompositionInfo *pInfo, const CProcessingInfo &prInfo, FBString &defineLine ) override
	{
		const ECompositeBlendType currMode = BlendMode;

		FBString strBlendMode = CompositeBlendTypeToString(currMode);
		defineLine += strBlendMode;
	}

	virtual void OnApplyFilter(	EApplyFilterStage stage, 
								const CCompositionInfo *pInfo, 
								const CProcessingInfo &prInfo, 
								CompositeFinalStats &stats, 
								const GLuint sourceTexId, 
								const GLuint dstTexId) override;

protected:

	struct DecalData
	{
		mat4		matrix;

		// terrain texture pointer (bindless texture)
		GLuint64		textureAddress;
		
		float			opacity;
		float			blendMode;

		float			contrast;
		float			saturation;
		float			brightness;

		float			gamma;

		float			inverse;

		float			hue;
		float			hueSat;
		float			lightness;
	};

	struct DecalGlobalData
	{
		vec4	info;

		vec4	viewDir[4];
		vec4	viewOrig[4];

		//std::vector<DecalData>			decals;
		DecalData				decals[MAX_DECALS_COUNT];
	};

	// gpu memory buffer
	DecalGlobalData							mData;
	
	CGPUBufferSSBO		mGPUBuffer;

	// for blend mode subroutines
	GLuint	mSubroutineIndex[25];

	const int ComputeRealDecalCount();
	FBComponent *GetRealDecal(const int index);

	void	PrepGPUBuffer(const CCompositionInfo *pInfo);

protected:

	FBSystem					mSystem;
	
	bool						mCreateNewDecal;
	bool						mGrabImage;

	void ConnectOrUpdateDecal(const char *imageName);

	void DoCreateDecal();
	void DoRefreshTexture();
	void DoGrabImage();

	void CropTexture(const GLuint texId, 
		const int texWidth,
		const int texHeight,
		const int offsetX,
		const int offsetY,
		const GLuint dstId,
		const int dstWidth,
		const int dstHeight);
	void DoSaveTextureToFile(const GLuint texId, const char *filename, bool needResize, int newWidth, int newHeight);
	void DoRunPhotoshop(const char *psPath, const char *filename);
};