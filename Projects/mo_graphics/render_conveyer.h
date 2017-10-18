
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: render_conveyer.h
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//-- 
#include <GL\glew.h>

//--- SDK include
#include <fbsdk/fbsdk.h>

#include "GPUCaching_model_display.h"

#include "algorithm\nv_math.h"
#include "shared_models.h"

#include <vector>
#include <map>

/*
	render conveyer

	store list of models

	ONLY in shaders display mode
		collect information from each frame - visibility, transformation, sprite sheet info

	draw early-z with simple shader
	draw projtex shader (only in shaders display mode active)

	using bindless textures

	draw using linear vector, passing multielements with one shader bind
	draw geometry cache using linear vector
*/

struct CRenderVertex
{
	vec4		position;
	vec4		normal;
	vec2		uv;
};


////////////////////////////////////////////////////////////////////////////////////
//

/*
	we store in the list the order of queue the mesh info, model info, material info
	 to not use Map.find each frame... do list update on changes

	 for example, commands converyer to render list of models for shadow maps
	  for reflections, for viewport color, etc.
	 commands - precache drawElements execution from batch of models

	 update commands converyer on:
	  - add, remove, merge, delete model
	  - change list of models for a specified conveyer
	  - change connection to models - attached materials, shaders, direct textures

	commands converyer make one setup for all scene models,
	 but there will be different elements buffers for multidrawelementsIndirect
	so that we could easialy filter what models we render for which goal
	 each goal should has own indirect elements buffer

	 even frustum culling it's just a change in the indirect elements buffer

	 custom commands converyer should make a copy from global scene render conveyer
	   and just specify own rule for draw or not to draw specified model or mesh !
*/

//////////////////////////////////////////////////////////////////////////////////////////////////
//
class CRenderCommandsFilter
{
public:
	CRenderCommandsFilter(void *pUserData=nullptr)
		: mUserData(pUserData)
	{}

	virtual bool FilterModel(FBModel *pModel)
	{
		return true;
	}

protected:

	void		*mUserData;

	void *GetUserDataPtr() {
		return mUserData;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CRenderCommandsConveyer
{
public:

	//! a constructor
	CRenderCommandsConveyer()
	{
		mBufferIndirect = 0;
	}

	void HasChanged()
	{
	}

	void Clear()
	{
		mCommands.clear();
	}

	// draw non of commands
	void UseNone()
	{
		for (auto iter=begin(mCommands); iter!=end(mCommands); ++iter)
		{
			iter->primCount = 0;
		}
	}

	// draw all commands
	void UseAll()
	{
		for (auto iter=begin(mCommands); iter!=end(mCommands); ++iter)
		{
			iter->primCount = 1;
		}
	}

	// add more commands
	void UseModelCommands(FBModel *pModel)
	{
		// TODO: extract commands from modelVertexData

		// TODO: find all model commands in the conveyer

		DrawElementsIndirectCommand		command;
		memset( &command, 0, sizeof(DrawElementsIndirectCommand) );
		command.primCount = 1;

		//command.count = size;
		//command.firstIndex = offset + mAccumNumberOfIndices; // DONE: shift this value by prev meshes numberOfIndices
		//command.baseInstance = (GLuint) mModelRender->mMeshInfos.size();
		
		bool filterModel = mCommandsFilter->FilterModel(pModel);
		command.primCount = (filterModel) ? 0 : 1;

		mCommands.push_back(command);
	}

	// multi draw commands
	void Execute()
	{
		if (mBufferIndirect > 0)
		{
			glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mBufferIndirect );
			glMultiDrawElementsIndirect( GL_TRIANGLES, GL_UNSIGNED_INT, (const GLvoid*) 0, (GLsizei) mCommands.size(), 0 );
		}

	}

	void PrepareBufferIndirect()
	{
		if (mCommands.size() > 0)
		{
			if (mBufferIndirect == 0)
			{
				glGenBuffers(1, &mBufferIndirect);
		
				glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mBufferIndirect );
				glBufferData( GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand) * mCommands.size(), mCommands.data(), GL_STREAM_DRAW );
				glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );
			}
		}
			
	}

protected:


	// Don't store information on CPU, we don't need any additional copies of data

	// draw all by one call
	// devide into two command list, one for fully opaque, others - the rest
	//std::vector<DrawElementsIndirectCommand>				mCommands;
	//std::vector<DrawElementsIndirectCommand>				mCommandsTransparency;	

	// this one is assigned as a attribute (location=4)
	//std::vector<MeshGLSL>						mMeshInfos;	// allocate and collect all information about meshes for render
	//std::vector<ModelGLSL>						mModelInfos;
	//GLuint										mBufferInfos;	// SSBO with submeshes data

	//CGPUBufferNV				mBufferPerMesh;
	//CGPUBufferNV				mBufferPerModel;

	std::vector<DrawElementsIndirectCommand>				mCommands;
	GLuint													mBufferIndirect;

	CRenderCommandsFilter		*mCommandsFilter;
};

////////////////////////////////////////////////////////////////////////////////////
//
class CRenderVertexConveyer
{
protected:

	struct ModelFilter {

		bool	skipGeometryCache;
		bool	skipDeformable;
		bool	skipWithSkeletalAnimation;
		bool	skipHidden;

		ModelFilter()
		{
			skipGeometryCache = true;
			skipDeformable = false;
			skipWithSkeletalAnimation = false;
			skipHidden = false;
		}

	};

	ModelFilter				mModelFilter;

public:
	//! a constructor
	CRenderVertexConveyer();
	//! a destructor
	~CRenderVertexConveyer();

	void Clear();

	bool CheckModelFilter(FBModel *pModel);

	bool CheckModelVertexData( FBModelVertexData *pVertexData );

	const int GetModelVertexIndexCount( FBModelVertexData *pVertexData );

	void AllocateFromFBModels(std::vector<FBModel*>	&models, std::vector<int> &flags);
	void CopyBufferSubData(int bufferType, FBModelVertexData *pVertexData, const unsigned int writeOffset);

	bool UpdateAllModelsSubData(FBEvaluateInfo *pEvaluateInfo);

	void Bind();
	void UnBind();

	// this is executed from model lists
	// void DrawModelElements();

protected:

	int						mLastVertexCount;
	int						mLastIndexCount;


	enum {
		BUFFER_POSITION,
		BUFFER_NORMAL,
		BUFFER_UV,
		BUFFER_INDEX,
		BUFFER_COUNT
	};

	// gpu buffers
	GLuint					mBufferIds[BUFFER_COUNT];
	unsigned int			mBufferElementSize[BUFFER_COUNT];
	unsigned int			mBufferElementCount[BUFFER_COUNT];
	
	struct VertexOffsets
	{
		bool	updated;

		unsigned int vertexOffset;
		unsigned int indexOffset;
		
		VertexOffsets()
		{
			updated = false;
		}

		VertexOffsets(unsigned int _vertexOffset, unsigned int _indexOffset)
			: vertexOffset(_vertexOffset)
			, indexOffset(_indexOffset)
		{
			updated = false;
		}
	};

	std::map<FBModel*, VertexOffsets>		mModelVertexCopyOffsets;
};


/*

	we should have global vertex array
	 for model and meshes - buffers with data (transformation matrix, assigned shader, material, etc.)


*/
