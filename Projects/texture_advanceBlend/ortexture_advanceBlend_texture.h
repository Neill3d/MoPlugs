#ifndef __ORTEXTURE_ADVANCEBLEND_TEXTURE_H__
#define __ORTEXTURE_ADVANCEBLEND_TEXTURE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: ortexture_advanceBlend_texture.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>
#include "graphics\GLSLShader.h"

#include "compositeMaster_shaders.h"

/** Shading Element Class implementation.
*	This should be placed in the source code file for a class.
*/
#define FBShadingElementClassImplementationWithLabel(ClassName,ElemName,IconFileName)\
	HIObject RegisterShadingElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
		ClassName* Class = new ClassName(pName);\
		Class->mAllocated = true;\
		if( Class->FBCreate() ){\
			return Class->GetHIObject();\
		} else {\
			delete Class;\
			return NULL;}}\
	FBLibraryModule(ClassName##ShadingElement){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements", ElemName, "", RegisterShadingElement##ClassName##Create, true, IconFileName);}


//--- Registration define
#define ORTEXTUREADVANCEBLEND__CLASSNAME	ORTextureAdvanceBlend
#define ORTEXTUREADVANCEBLEND__CLASSSTR		"ORTextureAdvanceBlend"
#define ORTEXTUREADVANCEBLEND__DESCSTR		"Advance Layered Texture"

#define MAX_NUMBER_OF_BLEND_LAYERS			4
/*
enum ECompositeBlendType
{
	eCompositeBlendNormal,
	eCompositeBlendLighten,
	eCompositeBlendDarken,
	eCompositeBlendMultiply,
	eCompositeBlendAverage,
	eCompositeBlendAdd,
	eCompositeBlendSubstract,
	eCompositeBlendDifference,
	eCompositeBlendNegation,
	eCompositeBlendExclusion,
	eCompositeBlendScreen,
	eCompositeBlendOverlay,
	eCompositeBlendSoftLight,
	eCompositeBlendHardLight,
	eCompositeBlendColorDodge,
	eCompositeBlendColorBurn,
	eCompositeBlendLinearDodge,
	eCompositeBlendLinearBurn,
	// Linear Light is another contrast-increasing mode
	// If the blend color is darker than midgray, Linear Light darkens the image by decreasing the brightness. If the blend color is lighter than midgray, the result is a brighter image due to increased brightness.
	eCompositeBlendLinearLight,
	eCompositeBlendVividLight,
	eCompositeBlendPinLight,
	eCompositeBlendHardMix,
	eCompositeBlendReflect,
	eCompositeBlendGlow,
	eCompositeBlendPhoenix
};
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

/**	Advance Blend modes texture.
*/
FB_FORWARD(ORTextureAdvanceBlend);
class ORTextureAdvanceBlend : public FBLayeredTexture
{
	//--- FiLMBOX Tool declaration.
	FBStorableClassDeclare( ORTextureAdvanceBlend, FBLayeredTexture );

public:
	ORTextureAdvanceBlend(const char *pName = NULL, HIObject pObject=NULL);

	// for each layer we need separate blend mode

	FBPropertyBaseEnum<ECompositeBlendType>		BlendMode;	// override original mode
	
	FBPropertyBool						UseMask;
	FBPropertyListObject				Mask;

	FBPropertyAnimatableColorAndAlpha   AuxLayer;             //!< <b>Read/Write Property:</b> A Texture Connectable Color property, used to provide additional info for composition. 
    FBPropertyBool                      CustomComposition;    //!< <b>Read/Write Property:</b> Switch to default / custom composition method. 
	/*
	FBPropertyAnimatableDouble			Contrast;
	FBPropertyAnimatableDouble			Saturation;
	FBPropertyAnimatableDouble			Brightness;

	FBPropertyAnimatableDouble			Gamma;
	*/
	// FBPropertyBool						GenerateMipmaps; // we don't have a control under RTT buffer

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

    /** Virtual function to be override if custom layer composition method needed.
    *   Upon this callback occurs, MotionBuilder already setup GL Render-To-Texture frame buffer target. 
    *   plugin code is responsible to fill up RTT buffer accordingly. 
    *   \param  pTime                   time in action time referential 
    *   \param  pTimeInCurrentTimeRef   time in current time ref
    *   \param  pWidth                  width of the RTT frame buffer target
    *   \param  pHeight                 height of the RTT frame buffer target
    *   \note   If not override in subclass, the internal layer composition logic will be used. 
    */
    virtual void TextureLayerComposition(FBTime pTime,FBTime pTimeInCurrentTimeRef, int pWidth, int pHeight) override;

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

    /**	Evaluation of non TRS nodes that needs to be evaluated. Inherited from FBBox.
    *	This function is called by the real-time engine in order to process animation information.
    *	\param	pEvaluateInfo	Information concerning the evaluation of the animation (time, etc.)
    *	\return \b true if animation node notification is successful.
    */
    virtual bool EvaluateAnimationNodes( FBEvaluateInfo* pEvaluateInfo ) override;

    virtual	bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;

protected:

	bool		mSupported;
	bool		mLoaded;

	void		SetUpBlendMode( const ECompositeBlendType mode );

	const GLuint GetTextureId( FBTexture *pTexture );

	CompositeShaderManager		mShaderManager;

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define	TOTAL_NUMBER_OF_SPRITE_SHADERS		3

enum ESpriteOrder
{
	eSpriteVector,
	eSpriteMatrix,
	eSpriteBlendedMatrix
};

const char *FBPropertyBaseEnum<ESpriteOrder>::mStrings[] = {
	"Vector",
	"Matrix",
	"Blended Matrix",
	nullptr
};

/**	Parametric Blend texture.
*/
FB_FORWARD(ORTextureParamBlend);
class ORTextureParamBlend : public FBLayeredTexture
{
	//--- FiLMBOX Tool declaration.
	FBStorableClassDeclare( ORTextureParamBlend, FBLayeredTexture );


public:
	ORTextureParamBlend(const char *pName = NULL, HIObject pObject=NULL);

	// for each layer we need separate blend mode

	
	FBPropertyAnimatableColorAndAlpha   AuxLayer;             //!< <b>Read/Write Property:</b> A Texture Connectable Color property, used to provide additional info for composition. 
    FBPropertyBool                      CustomComposition;    //!< <b>Read/Write Property:</b> Switch to default / custom composition method. 

	FBPropertyBaseEnum<ESpriteOrder>	SpriteOrder;

	FBPropertyInt						CountU;
	FBPropertyInt						CountV;

	FBPropertyAnimatableDouble			ParamU;
	FBPropertyAnimatableDouble			ParamV;

	FBPropertyBool						SpriteAnimation;
	FBPropertyInt						SpriteFPS;
	FBPropertyDouble					SpriteFramesLimit;
	FBPropertyBool						SpriteLocalPlay;
	FBPropertyBool						SpriteLoopPlay;

	// FBPropertyBool						GenerateMipmaps; // we don't have a control under RTT buffer

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate() override;		//!< FiLMBOX Creation function.
	virtual void FBDestroy() override;		//!< FiLMBOX Destruction function.

    /** Virtual function to be override if custom layer composition method needed.
    *   Upon this callback occurs, MotionBuilder already setup GL Render-To-Texture frame buffer target. 
    *   plugin code is responsible to fill up RTT buffer accordingly. 
    *   \param  pTime                   time in action time referential 
    *   \param  pTimeInCurrentTimeRef   time in current time ref
    *   \param  pWidth                  width of the RTT frame buffer target
    *   \param  pHeight                 height of the RTT frame buffer target
    *   \note   If not override in subclass, the internal layer composition logic will be used. 
    */
    virtual void TextureLayerComposition(FBTime pTime,FBTime pTimeInCurrentTimeRef, int pWidth, int pHeight) override;

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat) override;

    /**	Evaluation of non TRS nodes that needs to be evaluated. Inherited from FBBox.
    *	This function is called by the real-time engine in order to process animation information.
    *	\param	pEvaluateInfo	Information concerning the evaluation of the animation (time, etc.)
    *	\return \b true if animation node notification is successful.
    */
    virtual bool EvaluateAnimationNodes( FBEvaluateInfo* pEvaluateInfo ) override;

    virtual	bool PlugDataNotify(FBConnectionAction pAction,FBPlug* pThis,void* pData=NULL,void* pDataOld=NULL,int pDataSize=0) override;

protected:

	bool				mSupported;
		
	GLSLShader			*mShaders[TOTAL_NUMBER_OF_SPRITE_SHADERS];

	struct Locations
	{
		GLint				tex;
		GLint				countU;
		GLint				countV;
		GLint				paramU;
		GLint				paramV;
	};

	Locations			mLocations[TOTAL_NUMBER_OF_SPRITE_SHADERS];

	//
	//

	FBTime				mLastTime;
	double				mSpriteParamU;

	//
	//

	bool				InitShaders();
	bool				InitLocations(const GLSLShader *shader, Locations &locations);

	void				FreeShaders();
};


#endif /* __ORTEXTURE_ADVANCEBLEND_TEXTURE_H__ */
