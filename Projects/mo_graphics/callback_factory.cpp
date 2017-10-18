
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: callback_factory.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "callback_factory.h"
#include "callbacks.h"
#include "callbacks_binded.h"
#include "callbacks_bindless.h"
#include "callbacks_particles.h"

#include "shared_content.h"

class CShadersFactoryImpl;

static CShadersFactoryImpl	*gFactoryImpl = nullptr;
static int	gFactoryRef = 0;

///////////////////////////////////////////////////////////////////////
//
class CShadersFactoryImpl 
{
public:

	//! a constructor
	CShadersFactoryImpl()
	{

		mShaderTypes.resize(6);
		//mShaderTypes[0] = ( new CShaderSelectionCallback(0) );
		mShaderTypes[0] = ( new CShaderGPUCacheCallback(SHADER_CALLBACK_GPUCACHE) );
		mShaderTypes[1] = ( new CGPUParticlesCallback(SHADER_GPU_PARTICLES) );
		mShaderTypes[2] = ( new CProjTexBindedCallback(SHADER_CALLBACK_PROJTEX) );
		mShaderTypes[3] = ( new CEyeBindedCallback(SHADER_CALLBACK_EYE) );
		mShaderTypes[4] = ( new CIBLBindedCallback(SHADER_CALLBACK_IBL) );
		mShaderTypes[5] = ( new CUberShaderCallback(SHADER_CALLBACK_UBERSHADER) );
	}

	//! a destructor
	~CShadersFactoryImpl()
	{
		FreeCallbacks();
	}
	/*
	CBaseShaderCallback *FindTypeByShaderImpl( FBShader *pShader )
	{
		int intpass = (int) eShaderPassOpaque; // pass;


		int level = -1;
		CBaseShaderCallback *pCandidate = nullptr;

		for (auto iter=begin(mShaderTypes); iter!=end(mShaderTypes); ++iter)
		{
			CBaseShaderCallback *pCallback = *iter;
			
			if ( (intpass & pCallback->CallbackPasses()) == 0 )
				continue;

			if (pCallback->IsForShader( pShader ) )
			{

				if (level == -1 || level < pCallback->PriorityLevel() )
				{
					pCandidate = pCallback;
					level = pCallback->PriorityLevel();
				}
			}
		}

		return pCandidate;
	}
	*/
	CBaseShaderCallback *FindTypeByShaderAndPassImpl( FBShader *pShader, const ERenderGoal goal )
	{
		int intgoal = (int) goal;
		int level = -1;
		CBaseShaderCallback *pCandidate = nullptr;

		for (auto iter=begin(mShaderTypes); iter!=end(mShaderTypes); ++iter)
		{
			CBaseShaderCallback *pCallback = *iter;
			
			if ( (intgoal & pCallback->ShaderGroupGoal()) == 0 )
				continue;

			if (pCallback->IsForShaderAndPass( pShader, eShaderPassOpaque ) )
			{

				if (level == -1 || level < pCallback->PriorityLevel() )
				{
					pCandidate = pCallback;
					level = pCallback->PriorityLevel();
				}
			}
		}

		return pCandidate;
	}

	void RegisterNewShaderCallbackImpl( CBaseShaderCallback *pShaderCallback )
	{
		mShaderTypes.push_back(pShaderCallback);
	}


	inline const int GetCallbackCountImpl() const {
		return (int) mShaderTypes.size();
	}

	const std::vector<CBaseShaderCallback*> &GetShaderTypesVectorImpl() const
	{
		return mShaderTypes;
	}

protected:

	std::vector<CBaseShaderCallback*>		mShaderTypes;

	void FreeCallbacks()
	{
		for (auto iter=begin(mShaderTypes); iter!=end(mShaderTypes); ++iter)
		{
			CBaseShaderCallback *pCallback = *iter;
			delete pCallback;
		}

		mShaderTypes.clear();
	}

};


////////////////////////////////////////////////////////////////////
//

CShadersFactory::CShadersFactory()
	//: mFactoryImpl( new CShadersFactoryImpl() )
{
	if ( gFactoryImpl == nullptr )
		gFactoryImpl = new CShadersFactoryImpl();

	gFactoryRef += 1;
}

CShadersFactory::~CShadersFactory()
{
	gFactoryRef -= 1;

	if (gFactoryRef <= 0)
	{
		if (gFactoryImpl)
		{
			delete gFactoryImpl;
			gFactoryImpl = nullptr;
		}
	}

	/*
	if (mFactoryImpl)
	{
		delete mFactoryImpl;
		mFactoryImpl = nullptr;
	}
	*/
}
/*
CBaseShaderCallback *CShadersFactory::FindTypeByShader( FBShader *pShader )
{
	return gFactoryImpl->FindTypeByShaderImpl(pShader);
}
*/
CBaseShaderCallback *CShadersFactory::FindTypeByShaderAndGoal( FBShader *pShader, const ERenderGoal goal )
{
	return gFactoryImpl->FindTypeByShaderAndPassImpl(pShader, goal);
}

void CShadersFactory::RegisterNewShaderCallback( CBaseShaderCallback *pShaderCallback )
{
	gFactoryImpl->RegisterNewShaderCallbackImpl(pShaderCallback);
}

const int CShadersFactory::GetCallbackCount() const {
	return gFactoryImpl->GetCallbackCountImpl();
}

const std::vector<CBaseShaderCallback*> &CShadersFactory::GetShaderTypesVector() const
{
	return gFactoryImpl->GetShaderTypesVectorImpl();
}