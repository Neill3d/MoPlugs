
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_common.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <GL\glew.h>
#include "algorithm\nv_math.h"

#include "render_layer_info.h"
#include "graphics_framebuffer.h"

/*
void					ChooseComposite(ObjectComposition *pNewComposite);
void					ChooseCompositeByName(const char *name);
ObjectComposition	*FindCompositeByName(const char *name);
ObjectCompositeBase		*FindCompositeNodeByName(const char *name);
void					MakeSomeCompositeCurrent();

// current composition output (used in viewport rendering)
ObjectComposition	*GetOutputCompositePtr();
void					ClearOutputCompositePtr();
void					SetOutputCompositePtr(FBComponent *exceptThis=nullptr);

bool					AnyCompositionWithRender();

//
ObjectComposition	*GetCurrentCompositePtr();
ObjectCompositionRender	*FindRenderNodePtr(FBComponent *root);
ObjectComposition	*GetParentCompositePtr(FBComponent *startNode);
EffectSSAO				*FindEffectSSAOPtr(FBComponent *root);
ObjectCompositeBase		*GetSelectedCompositeObject(FBComponent *root);

ObjectComposition	*AddComposite(const char *name="composite");
// insert a new knot from the specified knot
void					RemoveComposite(ObjectComposition *composite);
ObjectComposition	*DublicateComposite(ObjectComposition *composite);
void					RenameComposite(ObjectComposition *composite, const char *newname);
*/

//
// NEED THIS TO MAKE CUSTOM FBDELETE PROC
//
#define FBClassImplementation2( Name )						\
    Name::~Name()				{ }                         \
    const char* Name::ClassName(){ return #Name; }			\
    int Name::GetTypeId()		{ return TypeInfo; }        \
    bool Name::Is( int pTypeId ){ return (pTypeId==TypeInfo) ? true : ParentClass::Is( pTypeId ); }  \
    int Name::TypeInfo=FBPlug::mGlobalTypeInfo++;

/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define FBElementClassImplementation2(ClassName, AssetName, IconFileName)\
	HIObject RegisterElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
		ClassName* Class = new ClassName(pName);\
		Class->mAllocated = true;\
		if( Class->FBCreate() ){\
            __FBRemoveModelFromScene( Class->GetHIObject() ); /* Hack in MoBu2013, we shouldn't add object to the scene/entity automatically*/\
			return Class->GetHIObject();\
		} else {\
			delete Class;\
			return NULL;}}\
	FBLibraryModule(ClassName##Element){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements/Composition", AssetName, "", RegisterElement##ClassName##Create, true, IconFileName);}

/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define FBElementClassImplementationLayer(ClassName, AssetName, IconFileName)\
	HIObject RegisterElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
		ClassName* Class = new ClassName(pName);\
		Class->mAllocated = true;\
		if( Class->FBCreate() ){\
            __FBRemoveModelFromScene( Class->GetHIObject() ); /* Hack in MoBu2013, we shouldn't add object to the scene/entity automatically*/\
			return Class->GetHIObject();\
		} else {\
			delete Class;\
			return NULL;}}\
	FBLibraryModule(ClassName##Element){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements/Composition/Layers", AssetName, "", RegisterElement##ClassName##Create, true, IconFileName);}


/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define FBElementClassImplementationFilter(ClassName, AssetName, IconFileName)\
	HIObject RegisterElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
		ClassName* Class = new ClassName(pName);\
		Class->mAllocated = true;\
		if( Class->FBCreate() ){\
            __FBRemoveModelFromScene( Class->GetHIObject() ); /* Hack in MoBu2013, we shouldn't add object to the scene/entity automatically*/\
			return Class->GetHIObject();\
		} else {\
			delete Class;\
			return NULL;}}\
	FBLibraryModule(ClassName##Element){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements/Composition/Filters", AssetName, "", RegisterElement##ClassName##Create, true, IconFileName);}


/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define FBElementClassImplementationCompositionModel(ClassName,AssetName,IconFileName)\
	HIObject RegisterElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
		ClassName* Class = new ClassName(pName);\
		Class->mAllocated = true;\
		if( Class->FBCreate() ){\
            __FBRemoveModelFromScene( Class->GetHIObject() ); /* Hack in MoBu2013, we shouldn't add object to the scene/entity automatically*/\
			return Class->GetHIObject();\
		} else {\
			delete Class;\
			return NULL;}}\
	FBLibraryModule(ClassName##Element){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements/Composition/Models", AssetName, "", RegisterElement##ClassName##Create, true, IconFileName);}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//

enum ECompositionInput
{
	eCompositionInputColor,
	eCompositionInputTexture,
	eCompositionInputRender
};

enum ECompositionQuality
{
	eCompositionQualityFull,
	eCompositionQualityPreview2,
	eCompositionQualityPreview4
};

struct CompositeFinalRenderDataExchange
{
	bool	showAsPreview;	// global settings that ask to render to preview

	int		w;
	int		h;
	
	// viewport border to keep internal aspect ratio of an image
	int		borderX;
	int		borderY;

	bool	ms;
	int		samples;

	bool	logDepth;
	bool	nvDepth;
	/*
	GLuint	colorId;
	GLuint	depthId;
	GLuint	normalId;
	GLuint	maskId;
	GLuint	positionId;

	GLuint	colorMSId;
	GLuint	depthMSId;
	GLuint	normalMSId;
	GLuint	maskMSId;
	GLuint	positionMSId;
	*/

	MainFrameBuffer		*frameBuffers[eRenderLayerCount];

	double	nearPlane;
	double	farPlane;

	FBCamera *pCamera;
	FBModel *pCameraInterest;

	bool							useDOF;
	FBCameraFocusDistanceSource		focusDistanceSource;
	double							focusSpecificDistance;
	double							focusAngle;

	//! a constructor
	CompositeFinalRenderDataExchange()
	{
		for (int i=0; i<eRenderLayerCount; ++i)
			frameBuffers[i] = nullptr;

		pCamera = nullptr;
		pCameraInterest = nullptr;
	}
};

enum ECompositionStats
{
	eCompositionStatsW,
	eCompositionStatsH,
	eCompositionStatsComputeShaders,
	eCompositionStatsDispatchGroups,
	eCompositionStatsNumberOfTextures,
	eCompositionStatsTexturesMemory,
	eCompositionStatsCount
};

struct CompositeFinalStats
{
	union
	{
		int values[eCompositionStatsCount];
		struct {
			int		w;
			int		h;		// height of a composition image

			int		computeShaders;			// evaluated compute shaders during render
			int		numberOfDispatchGroups;	// dispatch groups to gpu for computing

			int		numberOfTextures;	// number of textures used by the composition
			int		texturesMemory;		// (in bytes) allocated memory for the textures
		};
	};

	double texturesMemoryMb;

	//! a constructor
	CompositeFinalStats()
	{
		Clear();
	}

	void Clear()
	{
		texturesMemoryMb = 0;
		for (int i=0; i<eCompositionStatsCount; ++i)
			values[i] = 0;
	}

	void	CountTextures(const int count, const int memsize)
	{
		numberOfTextures += count;
		texturesMemory += memsize;
	}
	void	CountComputeShader(const int count, const int numgroups)
	{
		computeShaders += count;
		numberOfDispatchGroups += numgroups;
	}

	void UpdateTextureMemoryMb()
	{
		texturesMemoryMb = 1.0 * (double)texturesMemory / 1048576.0;
	}

	int GetStatsValue(const int index)
	{
		return values[index];
	}
};

// default framebuffer contains a result of a screen rendering (blitting from the MS framebuffer)
/*
FrameBuffer		&GetDefaultFrameBuffer();

const GLuint	gGetAttachedFBO();
const bool gIsUseAttachedIds();
const GLuint gGetColorTextureId();
const GLuint gGetNormalTextureId();
const GLuint gGetDepthTextureId();
const GLuint gGetMaskTextureId();

const GLuint gGetNormalTextureIdMS();
const GLuint gGetDepthTextureIdMS();
const GLuint gGetMaskTextureIdMS();

void	SetAttachState(const bool defaultFramebuffer, const GLuint fbo);
void	SetAttachedTextures(const GLuint textureId, const GLuint normalId, const GLuint depthId, const GLuint maskId);
void	SetAttachedTexturesMS( const GLuint normalMS, const GLuint depthMS, const GLuint maskMS );
*/

///////////////////////////////////////////////////////////

GLuint FindDepthId(FBComponent *root);
GLuint FindNormalsId(FBComponent *root);
GLuint FindPosId(FBComponent *root);
GLuint	FindMaskTextureId();

//////////////////////////////////////////////////////////

bool IsNodeFinal(FBComponent *pNode);
bool IsNodeFilter(FBComponent *pNode);
bool IsNodeSource(FBComponent *pNode);



///////////////////

void ComputeCameraOrthoPoints(const float renderWidth, const float renderHeight, FBModel *pCamera, double farPlane, double nearPlane, vec3 *points);
void ComputeCameraFrustumPoints(const float renderWidth, const float renderHeight, FBModel *pCamera, double farPlane, double nearPlane, double FieldOfView, vec3 *points);
