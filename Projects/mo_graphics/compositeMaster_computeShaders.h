
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_computeShaders.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <GL\glew.h>

#include <vector>
#include <map>

#include "types.h"
#include "graphics\GLSLShader.h"

namespace CompositeComputeShader
{

//
// FORWARD DECLARATION

struct CShaderQuery;

// blend shaders

enum EBlendShaders
{
	eShaderBlendNormal,	// TODO: add 25 blend modes here
	eShaderBlendLighten,
	eShaderBlendDarken,
	eShaderBlendMultiply,
	eShaderBlendAverage,
	eShaderBlendAdd,
	eShaderBlendSubstract,
	eShaderBlendDifference,
	eShaderBlendNegation,
	eShaderBlendExclusion,
	eShaderBlendScreen,
	eShaderBlendOverlay,
	eShaderBlendSoftLight,
	eShaderBlendHardLight,
	eShaderBlendColorDodge,
	eShaderBlendColorBurn,
	eShaderBlendLinearDodge,
	eShaderBlendLinearBurn,
	eShaderBlendLinearLight,
	eShaderBlendVividLight,
	eShaderBlendPinLight,
	eShaderBlendHardMix,
	eShaderBlendReflect,
	eShaderBlendGlow,
	eShaderBlendPhoenix,

	eShaderBlendCount
};

// masking

enum EMaskShaders
{
	eShaderMaskEmpty,
	eShaderMasked,

	eShaderMaskCount
};

//////////////////////////////////////////////////////////////////////////////////////
// shader file could contain several parts, we should specify the part name we want to use here
class CShaderCodeHolder
{

public:

	//! a constructor
	CShaderCodeHolder();

	//! a destructor
	~CShaderCodeHolder();

	bool LoadFromFilename(const char *filename);
	bool LoadFromBuffer(const char *filename, bool virtualFilename, const char *buffer, const size_t bufferLen);
	bool Reload();

	void Clear();

	const int GetNumberOfParts() const;
	const char *GetBufferPtr(size_t &bufferSize, const char *partName=nullptr) const;
	//const int GetBufferSize(const char *partName=nullptr) const;

	bool	IsLoadedOk() const
	{
		return mStatus;
	}

	const int GetNumberOfIncludeFiles() const;
	const char *GetIncludeFilename(const int index) const;

protected:

	bool				mStatus;

	std::string			mFileName;
	
	// hash map part name to part offset
	std::map<size_t, char*>	mPartsMap;

	// code of a sub-shader
	std::vector<char>	mCodeBuffer;

	//size_t		mCodeBufferSize;
	//char		*mCodeBuffer;

	std::vector<std::string>	mIncludeVector;

	void	PreparePartOffsets();
	void	PrepareIncludeList();
};

//////////////////////////////////////////////////////////////////////////////////////
//

/*
	CShaderQuery	myQuery[4] = {
	 "\\GLSL_CS\\blit.cs", "Header",
	 "\\GLSL_CS\\mask.cs", "NoMask",
	 "\\GLSL_CS\\blit.cs", "Main"
	 };

	 CMixProgram *myProgram = QueryProgramMix( "myProgram", myQuery );
	 ReloadFiles(myQuery);
	 ReloadShades(myQuery);

	 myProgram.Bind()

	 myProgram.Dispatch()
	 
	 myProgram.UnBind()


	 myQuery = { "", "", "#define BLEND_FUNC BLEND_ADD\n" };

*/

struct CShaderQuery
{
	std::string		strDefine;		// empty line for skipping defines section
	std::string		filename;		// could be empty line for skipping file import
	std::string		partname;

	int				fileCodeIndex;

	CShaderQuery()
	{}

	CShaderQuery(const char *define, const char *_filename, const char *_partname)
		: strDefine(define)
		, filename(_filename)
		, partname(_partname)
	{}
};

//////////////////////////////////////////////////////////////////////////////////////
// store main shader pipeline functionality
class CMixedProgram
{
public:

	//! a constructor
	CMixedProgram(const size_t key, const GLuint shaderid, const GLuint programid);

	//! a destructor
	~CMixedProgram();

	void Clear();

	void CreateGLObjects();
	void ReCreateShaderObject();

	void Bind();
	void UnBind();

	void DispatchPipeline(const int groups_x, const int groups_y, const int groups_z);

	const GLuint		GetProgramId() const
	{
		return mProgram;
	}
	const GLuint GetShaderId() const
	{
		return mShader;
	}

	bool IsOk() const {
		return mStatus;
	}

public:

	// header, blend, mask, main codes
	size_t			mCombinationKey;

	GLuint			mShader;
	GLuint			mProgram;

	bool			mStatus;	// compile and link shader status

	// store information for possible shader reload
	/*
	int						mVersion;
	std::vector<int>		mShaderCodes;
	std::string				mShaderDefines;
	*/

	std::string					mDisplayLabel;
	std::string					mHeader;
	std::vector<CShaderQuery>	mQuery;

	bool HasShaderCode(const int codeIndex)
	{
		for (auto iter=begin(mQuery); iter!=end(mQuery); ++iter)
		{
			if (iter->fileCodeIndex == codeIndex)
				return true;
		}

		return false;
	}

};



//////////////////////////////////////////////////////////////
// one set of texture buffers for all compositions !
template <int N>
class CComputeTextureBuffer
{

public:

	//! a constructor
	CComputeTextureBuffer()
	{
		mWidth = 1;
		mHeight = 1;

		mCurrentTexture = 0;
		for (int i=0; i<N; ++i)
			mTextures[i] = 0;
	}

	//! a destructor
	~CComputeTextureBuffer()
	{
		FreeLayersData();
	}

	
	// return a ready to use texture buffer for doing a write operation
	const GLuint	QueryATextureBuffer()
	{
		mCurrentTexture = mCurrentTexture+1;
		if (mCurrentTexture >= N)
			mCurrentTexture = 0;

		return mTextures[mCurrentTexture];
	}

	
	bool IsTexturesReady()
	{
		return (mTextures[0] > 0);
	}

	bool IsEqualSize(const int w, const int h)
	{
		return (w == mWidth && h == mHeight);
	}

	void ReSizeLayersData(const int w, const int h)
	{
		// DONE: texture buffer size
		mWidth = w;
		mHeight = h;

		if (mTextures[0] == 0)
			glGenTextures( N, mTextures );

		for (int i=0; i<N; ++i)
		{
	
			glBindTexture(GL_TEXTURE_2D, mTextures[i]);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

	}

	void FreeLayersData()
	{
		if (mTextures[0] > 0)
		{
			glDeleteTextures(N, mTextures);

			for (int i=0; i<N; ++i)
				mTextures[i] = 0;
		}
		mCurrentTexture = 0;
	}

	const GLuint GetCurrentTextureId() const
	{
		return mTextures[mCurrentTexture];
	}

	const int GetWidth() const
	{
		return mWidth;
	}
	const int GetHeight() const
	{
		return mHeight;
	}

protected:

	int				mWidth;
	int				mHeight;

	int				mCurrentTexture;

	GLuint			mTextures[N];

};

///////////////////////////////////////////////////////////////////////
// DONE: make a singleton
class CMixedProgramManager : public Singleton<CMixedProgramManager>
{

public:

	//! a constructor
	CMixedProgramManager();

	//! a destructor
	~CMixedProgramManager();

	// pre-load shaders for blend modes and masking
	void	LoadDefaultShaders();

	// shader execution groups
	void GetGroupLocalSize(int &local_x, int &local_y)
	{
		local_x = 32;
		local_y = 32;
	}

	// return a combination of several shaders
	CMixedProgram	*QueryAProgramMix(const char *displayLabel, 
		const char *header,
		const char *define, 
		const char *filename, 
		const char *partname);
	CMixedProgram	*QueryAProgramMix(const char *displayLabel,
		const char *header,
		std::vector<CShaderQuery> &query);

	GLSLShader *QueryLogDrawShader();
	GLSLShader *QueryCubeMapShader();

	// need to free all the shaders
	void ChangeContext();

	void FreeShaders();
	void FreeCodeHolders();

	// put a shader code into a buffer with specified index
	//  pre-loaded shader could contain several parts inside, like header and main body
	const int PreLoadNewShaderCode(const char *filename, int &numberOfPreloadedParts);
	const int NewShaderCodeFromBuffer(const char *virtualFilename, const char *buffer, const size_t bufferLen, int &numberOfPreloadedParts);
	// reload several shader codes starting from index 
	//void ReloadShaderCode(const int index, const int count);

	bool ReloadProgram(CMixedProgram *pMixedProgram);
	
	bool UploadProgramIncludeCodes(const char *ShaderCodeName );
	bool UploadProgramIncludeCodes(const std::vector<std::string> &includeVector);
public:

	static bool checkCompileStatus(GLuint shader, const char *shadername);
	static bool checkLinkStatus(GLuint program, const char * programName);

	static bool loadComputeShaderFromBuffer(const char* buffer, const char *shaderName, const GLuint shaderid, const GLuint programid);
	static bool loadComputeShader(const char* filename, const GLuint shaderid, const GLuint programid);

protected:

	// shader for draw helper geometry in logarithmic depth
	std::auto_ptr<GLSLShader>			mDrawShader;
	std::auto_ptr<GLSLShader>			mCubeMapShader;

	//CComputeTextureBuffer<2>			mTextureBuffer;

	std::string							mIncludeRootPath;

	// store shader code
	std::vector<CShaderCodeHolder*>		mShaderCodeVector;
	std::map<size_t, int>				mShaderCodeMap;

	//int									mShaderBlendModesIndices[eShaderBlendCount];
	//int									mShaderMaskIndices[eShaderMaskCount];

	// store preloaded combinations
	std::map<size_t, CMixedProgram*>	mMixedMap;

	// we need connection between mixed program and shader codes (to know what to reload)
	// TODO: we need to update all program mixes when connected shader codes has updated

	std::multimap<CMixedProgram*, int>		mMixedToShaderCodeMap;

	// include shaders (key is a hash filename)
	//	value is a shader
	//std::map<size_t, GLuint>	mIncludeShadersMap;

	CMixedProgram	*MakeANewProgramMix(const char *displayLabel, const char *header, std::vector<CShaderQuery> &query);

	bool CompileProgramMix(CMixedProgram *mixedProgram, const char *displayLabel, const char *header, std::vector<CShaderQuery> &query);

	int	MakeAKey(unsigned char header, unsigned char blendmode, unsigned char mask, unsigned char effect);

	const size_t MakeAHashKey(const char *filename) const;
	const size_t MakeAHashKey(const char *define, const char *filename, const char *partname) const;
	const size_t MakeAHashKey(std::vector<CShaderQuery> &query) const;

};

};