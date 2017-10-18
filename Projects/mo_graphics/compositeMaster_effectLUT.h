
#pragma once

#include "compositeMaster_object.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////
//
enum EEffectLUTPreset
{
	eEffectLUT_Custom,
	eEffectLUT_File1,
	eEffectLUT_File2
};

const char * FBPropertyBaseEnum<EEffectLUTPreset>::mStrings[] = {
	"Custom",
	"File1",
	"File2",
	0};

class EffectLUT : public ObjectCompositeBase
{
	//--- FiLMBOX declaration.
	FBClassDeclare2(EffectLUT, ObjectCompositeBase)
	FBDeclareUserObject(EffectLUT);

public:
	//! a constructor
	EffectLUT(const char *pName = NULL, HIObject pObject=NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	//virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	//virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	
public:
	
	// enum presets
	
	FBPropertyAnimatableDouble				Weight;

	FBPropertyInt							LutSize;	//!< 3d texture edge size

	FBPropertyString						FileName;	//!< 3d LUT file path
	FBPropertyAction						Load;		//!< choose a custom LUT file

	void CopyFrom(ObjectCompositeBase *pNode);

	// render to buffers with render tree
	virtual void Render() override;

	virtual void CleanupRender() override;

	void DoLoad();

private:

	GLuint		mFragShader;
	GLuint		mProgram;
	GLuint		mWeightLoc;

	GLuint		mLut3dTexID;
	int			mLutEdgeSize;

	bool		mNeedUpdate;

	int			mLutEdgeSizeCache;
	std::string			mLutFileLoaded;
	std::string			mLut3dCacheid;
	std::string			mShaderCacheid;
	std::vector<float>	mLut3d;

	float		mExposureFStop;
	float		mDisplayGamma;

	void	InitParams();
	void	LoadFromFile(const char *filename);

	void	AllocateLut3D();

	const GLuint	CompileShaderText(const GLenum shaderType, const char *text);
	const GLuint	LinkShaders(const GLuint fragShader);

	bool	UpdateOCIOGLState();

	static void SetLoad(HIObject object, bool value);
	static void SetEdgeSize(HIObject object, int value);

public:

	virtual void DoReloadShader();
};