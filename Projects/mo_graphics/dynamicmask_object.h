
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: dynamicmask_object.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <GL\glew.h>
#include "graphics\glslShader.h"
#include <vector>

//--- Registration define
#define OBJECTSETTINGS__CLASSNAME	ObjectMask
#define OBJECTSETTINGS__CLASSSTR	"ObjectMask"

#define OBJECTCHANNEL__CLASSNAME	ObjectChannel
#define OBJECTCHANNEL__CLASSSTR		"ObjectChannel"

#define OBJECTSHAPE__CLASSNAME		ObjectShape
#define OBJECTSHAPE__CLASSSTR		"ObjectShape"

#define OBJECTKNOT__CLASSNAME		ObjectKnot
#define OBJECTKNOT__CLASSSTR		"ObjectKnot"

/// Forward declaration

class ObjectKnot;
class ObjectShape;
class ObjectChannel;
class ObjectMask;


/////////////////////////

enum EObjectMaskSize
{
	eMaskSize32,
	eMaskSize64,
	eMaskSize128,
	eMaskSize256,
	eMaskSize512,
	eMaskSize1024,
	eMaskSize2048,
	eMaskSize4096,
};

const char * FBPropertyBaseEnum<EObjectMaskSize>::mStrings[] = {
	"32",
	"64",
	"128",
	"256",
	"512",
	"1024",
	"2048",
	"4096",
	0};

enum BezierSegmentType
{
	eBezierSegmentTypeLine,
	eBezierSegmentTypeCurve
};


///////////////////////////////////////////////////////////
//

/**	store global masking settings (rendering dynamic mask)
*/
class ObjectMask : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(ObjectMask, FBUserObject)
	FBDeclareUserObject(ObjectMask);

public:
	ObjectMask(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);

	FBPropertyBool							Enable;
	/*
	FBPropertyBaseEnum<EObjectMaskSize>		CanvasWidth;
	FBPropertyBaseEnum<EObjectMaskSize>		CanvasHeight;
	*/

	FBPropertyInt						CanvasWidth;
	FBPropertyInt						CanvasHeight;

	FBPropertyBool					RecalculatePoints;

	FBPropertyListObject			Video;

	FBPropertyBool					ManualUpdate;
	FBPropertyAction				Update;

	FBPropertyColor					ShapeColor;
	FBPropertyBool					Stroke;
	FBPropertyColor					StrokeColor;
	FBPropertyDouble				ShapeTransparency;
	FBPropertyBool					ShowGrid;

	FBPropertyDouble				BlurFactor;

	// background texture or video
	FBPropertyBool					ShowBackgroundTexture;
	FBPropertyListObject			BackgroundTexture;

	// models to show UV layout
	FBPropertyListObject			UVLayout;
	FBPropertyBool					ShowTexture;
	FBPropertyBool					ShowWire;

	static void SetCanvasWidth(HIObject object, int value);
	static void SetCanvasHeight(HIObject object, int value);
	static int GetCanvasWidth(HIObject object);
	static int GetCanvasHeight(HIObject object);

public:
	//GLuint				mMaskTexId;		// final mask texture (composite from 4 channels)
	//FBVideoMemory		mVideoMemory;
	//FBTexture			mTexture;

	void Render();

	GLuint				getColorTextureId() { 
		return (BlurFactor > 0.0) ? getBlurVTextureId() : color_buffer_id; 
	}

	void	NeedResize()
	{
		mNeedResize = true;
	}
	void	DoReSize();
	void	DoUpdate();

	void ChangeContext();
	
	//const int	GetCanvasWidth() const;
	//const int	GetCanvasHeight() const;

protected:

	bool				mLoaded;
	bool				mNeedResize;
	bool				mNeedUpdate;

	int					mWidth;
	int					mHeight;

	int					mLastWidth;
	int					mLastHeight;

	std::auto_ptr<GLSLShader>	mShaderComposite;	// composite 4 channels into the one image
	struct CShaderCompositeLoc {
		GLint				rSampler;
		GLint				gSampler;
		GLint				bSampler;
		GLint				aSampler;

		CShaderCompositeLoc()
		{
			rSampler = -1;
			gSampler = -1;
			bSampler = -1;
			aSampler = -1;
		}
	} mShaderCompositeLoc;

	GLuint				color_buffer_id;
	GLuint				depth_buffer_id;
	GLuint				stencil_buffer_id;
	GLuint			depth_id;
	GLuint				fbo_id;

	
	// blur shaders and buffer
	std::auto_ptr<GLSLShader>	mShaderBlur;	// horizontal blur
	struct CShaderBlurLoc {
		GLint				sampler;
		GLint				scale;

		CShaderBlurLoc()
		{
			sampler = -1;
			scale = -1;
		}
	} mShaderBlurLoc;

	GLuint				blur_h_texture_id;
	GLuint				blur_v_texture_id;

	bool PrepCompositeShader();
	bool PrepBlurShader();

	void RenderBlur(const GLuint color_buffer_id, const GLuint color_output_id, const float hFactor, const float vFactor);

	const GLuint	getBlurHTextureId();
	const GLuint	getBlurVTextureId();

	void GenerateResources();

	void GenerateFBO();
	void CleanupFBO();
};

/**	channel (r,g,b,a) stores shapes inside and channel options
*/
class ObjectChannel : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(ObjectChannel, FBUserObject)
	FBDeclareUserObject(ObjectChannel);

public:
	ObjectChannel(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	
	FBPropertyBool			OverrideDisplay;
	FBPropertyColor			ShapeColor;
	FBPropertyBool			Stroke;
	FBPropertyColor			StrokeColor;
	FBPropertyDouble		ShapeTransparency;
	FBPropertyBool			ShowGrid;

	// background texture or video
	FBPropertyBool					ShowBackgroundTexture;
	FBPropertyListObject			BackgroundTexture;

	FBPropertyDouble		BlurFactor;

	// models to show UV layout
	FBPropertyListObject	UVLayout;
	FBPropertyBool			ShowTexture;
	FBPropertyBool			ShowWire;

	
	void		Render();

	const GLuint	getColorTextureId();
	
	void		ChangeContext();

	void		ReSize(const int w, const int h);

	void		RecalculatePoints(const double x, const double y);

	void		SetShapeDisplay(const bool showGrid, const FBColor &color, const double transparency)
	{
		mShowGrid = showGrid;
		mColor = color;
		mTransparency = transparency;
	}


private:

	int					mWidth;
	int					mHeight;

	bool				mShowGrid;
	FBColor				mColor;
	double				mTransparency;

	// main shapes stencil buffer
	GLuint				color_texture_id;

	// compile path into ogl display list
	GLuint				diplayListId;

	void GenerateResources();
	void PreRender();
	void Cleanup();
};

/**	store each figure
*/
class ObjectShape : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(ObjectShape, FBUserObject)
	FBDeclareUserObject(ObjectShape);

public:
	ObjectShape(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);

	FBPropertyBool					ClosePath;
	FBPropertyBool					Fill;

	FBPropertyBool					OverrideDisplay;
	FBPropertyAnimatableColor		Color;		// shape color
	FBPropertyBool					Stroke;
	FBPropertyAnimatableColor		StrokeColor;	// border color
	// shape tm
	FBPropertyAnimatableVector2d	Position;
	FBPropertyAnimatableDouble		Rotation;
	FBPropertyAnimatableVector2d	Scale;

	FBMatrix	GetMatrix()
	{
		FBTVector pos;
		FBSVector scale;
		FBRVector rot;
		FBMatrix m;
		
		Position.GetData(pos, sizeof(double)*2);
		Scale.GetData(scale, sizeof(double)*2);
		Rotation.GetData(&rot[2], sizeof(double));

		FBTRSToMatrix(m, pos, rot, scale);
		return m;
	}

	void		UpdateTangents();
	void		CalculateSegments();
	void		PrepPathCommandsNV();

	const int GetNumberOfCommands() const {
		return (int) mCommands.size();
	}
	GLubyte *GetCommandsArray() {
		return mCommands.data();
	}

	const int GetNumberOfCoords() const {
		return (int) mCoords.size();
	}

	GLshort *GetCoordsArray() {
		return &mCoords[0].x;
	}

	const GLuint	GetPathObjectNV() {
		if (mPathId == 0)
			mPathId = glGenPathsNV(1);
		return mPathId;
	}

	void ClearPathObjectNV() {
		if (mPathId > 0)
			glDeletePathsNV( mPathId, 1 );
	}

protected:

	struct BezierSegment
	{
		BezierSegmentType		type;	// (line, curve)

		FBVector2d				ctrlpoints[4];

		void Set(const FBVector2d &knot1, const FBVector2d &inVec, const FBVector2d &outVec, const FBVector2d &knot2)
		{
			ctrlpoints[0] = knot1;
			ctrlpoints[1] = inVec;
			ctrlpoints[2] = outVec;
			ctrlpoints[3] = knot2;
		}
	};

	struct ShapeCoord
	{
		GLshort x,y;
	};

	std::vector<BezierSegment>			mSegments;

	// pre cached data for sending NV path commands and coords
	std::vector<GLubyte>				mCommands;
	std::vector<ShapeCoord>				mCoords;

	GLuint									mPathId;

	// TODO: should be calculate to SEGMENT, not KNOT !!!
	void CalculateSmoothTangents(const int index, const FBVector2d &p1, const FBVector2d &p2);
};


enum ObjectKnotType
{
	eObjectKnotType_CORNER,
	eObjectKnotType_BEZIER,
	eObjectKnotType_SMOOTH
};

const char * FBPropertyBaseEnum<ObjectKnotType>::mStrings[] = {"Corner", "Bezier", "Smooth", 0};

/**	store each control point in specified figure
*/
class ObjectKnot : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(ObjectKnot, FBUserObject)
	FBDeclareUserObject(ObjectKnot);

public:
	ObjectKnot(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);


	FBPropertyBaseEnum<ObjectKnotType> Type;		//!< line or curve

	FBPropertyAnimatableVector2d	Position;
	FBPropertyAnimatableVector2d	LeftTangent;
	FBPropertyAnimatableVector2d	RightTangent;

	FBPropertyBool					LockTangents;	// to avoid cusp
	
	bool		Drag;	// is point is dragging by the mouse
	double		DragWeight;		// magnet operation with radius
	double		DragWeightLeft;
	double		DragWeightRight;

	bool		Selected; // is point selected ?

	bool		SelectedLeftTangent;
	bool		SelectedRightTangent;

	// cached global values
	FBVector2d	incoming;
	FBVector2d	outgoing;

public:
	void SetType( const ObjectKnotType type )
	{
		Type = type;
	}

	ObjectKnotType GetType()
	{
		return Type;
	}
/*
private:
	ObjectKnotType			mType;
	*/
};

