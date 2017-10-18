
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_objectLUTFilter.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "OpenColorIO/OpenColorIO.h"
namespace OCIO = OCIO_NAMESPACE;

#include "compositeMaster_objectLUTFilter.h"
#include "shared_misc.h"
#include "graphics\CheckGLError_MOBU.h"
#include "StringUtils.h"

#include <sstream>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Effect 3D LUT

FBClassImplementation2(ObjectFilterLUT)
FBUserObjectImplement(ObjectFilterLUT, "Composition 3d LUT", EFFECT_ICON);			//Register UserObject class
FBElementClassImplementationFilter(ObjectFilterLUT, FILTERLUT__ASSETNAME, EFFECT_ICON);		//Register to the asset system

void AddPropertyViewForFilterLUT(const char* pPropertyName, const char* pHierarchy, bool pIsFolder=false)
{
	FBPropertyViewManager::TheOne().AddPropertyView(FILTERLUT__CLASSSTR, pPropertyName, pHierarchy);
}
void ObjectFilterLUT::AddPropertiesToPropertyViewManager()
{
	AddPropertyViewForFilterLUT( "Active", "" );
	AddPropertyViewForFilterLUT( "Reload Shader", "" );
	AddPropertyViewForFilterLUT( "Opacity", "" );

	AddPropertyViewForFilterLUT( "", "Masking", true );
	AddPropertyViewForFilterLUT( "Use Composite Mask", "Masking" );
	AddPropertyViewForFilterLUT( "Select Composite Mask", "Masking" );
	AddPropertyViewForFilterLUT( "Invert Composite Mask", "Masking" );
	AddPropertyViewForFilterLUT( "Custom Mask", "Masking" );

	AddPropertyViewForFilterLUT( "", "Info", true );
	AddPropertyViewForFilterLUT( "Out Width", "Info" );
	AddPropertyViewForFilterLUT( "Out Height", "Info" );
}

/************************************************
 *	Constructor.
 ************************************************/
void ObjectFilterLUT::SetLoad(HIObject object, bool value)
{
	ObjectFilterLUT *pBase = FBCast<ObjectFilterLUT>(object);
	if (pBase && value) 
	{
		pBase->DoLoad();
	}
}

void ObjectFilterLUT::SetEdgeSize(HIObject object, int value)
{
	ObjectFilterLUT *pBase = FBCast<ObjectFilterLUT>(object);
	if (pBase) 
	{
		if (value < 1) value = 1;
		if (value > 128) value = 128;

		pBase->mLutEdgeSize = value;
		pBase->LutSize.SetPropertyValue(value);

		pBase->AllocateLut3D();
		pBase->mLutFileLoaded = "";
	}
}

ObjectFilterLUT::ObjectFilterLUT( const char* pName, HIObject pObject ) 
	: ObjectCompositeFilter( pName, pObject, new CProgramUniformsBase() )
{
	FBClassInit;

	mLutEdgeSize = 64;

	mLut3dTexID = 0;
	mLutEdgeSizeCache = 0;

	mExposureFStop = 0.0f;
	mDisplayGamma = 1.0f;

	mLutFileLoaded = "";
	mNeedUpdate = true;
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool ObjectFilterLUT::FBCreate()
{
	InitParams();
	
	FBPropertyPublish(this, LutSize, "LUT Size", nullptr, ObjectFilterLUT::SetEdgeSize);
	FBPropertyPublish(this, FileName, "FileName", nullptr, nullptr);
	FBPropertyPublish(this, Load, "Load", nullptr, ObjectFilterLUT::SetLoad);

	LutSize = 64;
	FileName = "";

	return ParentClass::FBCreate();
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/

void ObjectFilterLUT::FBDestroy()
{
	Cleanup();
	ParentClass::FBDestroy();	
}

void ObjectFilterLUT::InitParams()
{
	mLut3dCacheid = "";
	mShaderCacheid = "";
	mLutFileLoaded = "";
}

void ObjectFilterLUT::AllocateLut3D()
{
	if (mLut3dTexID > 0 && mLutEdgeSize == mLutEdgeSizeCache)
		return;

	if (mLut3dTexID > 0)
	{
		glDeleteTextures( 1, &mLut3dTexID );
		mLut3dTexID = 0;
	}

	glGenTextures( 1, &mLut3dTexID );

	int num3Dentries = 3 * mLutEdgeSize*mLutEdgeSize*mLutEdgeSize;

	mLut3d.resize(num3Dentries);
	memset( mLut3d.data(), 0, sizeof(float)*num3Dentries );

	glBindTexture(GL_TEXTURE_3D, mLut3dTexID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, mLutEdgeSize, mLutEdgeSize, mLutEdgeSize,
		0, GL_RGB, GL_FLOAT, mLut3d.data() );

	glBindTexture(GL_TEXTURE_3D, 0);

	mLutEdgeSizeCache = mLutEdgeSize;
	mNeedUpdate = true;
}

void replaceAll( std::string &s, const std::string &search, const std::string &replace ) {
    for( size_t pos = 0; ; pos += replace.length() ) {
        // Locate the substring to replace
        pos = s.find( search, pos );
        if( pos == std::string::npos ) break;
        // Replace by erasing and inserting
        s.erase( pos, search.length() );
        s.insert( pos, replace );
    }
}

bool ObjectFilterLUT::UpdateOCIOGLState()
{
	const char *filename = FileName;
	OCIO::ConstProcessorRcPtr processor;
	
	//mGeneratedShaderCode.clear();

	try
	{
		OCIO::ConstConfigRcPtr	config = OCIO::GetCurrentConfig();

		OCIO::FileTransformRcPtr transform = OCIO::FileTransform::Create();
		transform->setSrc(filename);

		transform->setDirection(OCIO::TRANSFORM_DIR_FORWARD);
		transform->setInterpolation(OCIO::INTERP_LINEAR);
	
		processor = config->getProcessor(transform, OCIO::TRANSFORM_DIR_FORWARD);
	}
	catch(OCIO::Exception &e)
	{
		printf(e.what() );
		return false;
	}
	catch(...)
	{
		return false;
	}

	// Step 1: Create a GPU Shader Description
	OCIO::GpuShaderDesc shaderDesc;
	shaderDesc.setLanguage(OCIO::GPU_LANGUAGE_GLSL_1_3);
	shaderDesc.setFunctionName("OCIODisplay");
	shaderDesc.setLut3DEdgeLen(mLutEdgeSize);

	// Step 2: compute the 3D LUT
	std::string lut3dCacheID = processor->getGpuLut3DCacheID(shaderDesc);
	if (lut3dCacheID != mLut3dCacheid)
	{
		mLut3dCacheid = lut3dCacheID;
		processor->getGpuLut3D(mLut3d.data(), shaderDesc);

		glBindTexture(GL_TEXTURE_3D, mLut3dTexID);
		glTexSubImage3D(GL_TEXTURE_3D, 0,
			0, 0, 0,
			mLutEdgeSize, mLutEdgeSize, mLutEdgeSize,
			GL_RGB, GL_FLOAT, mLut3d.data() );
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	CHECK_GL_ERROR_MOBU();

	// Step 3: compute the shader
	std::string shaderCacheID = processor->getGpuShaderTextCacheID(shaderDesc);
	if (nullptr == mProgram || shaderCacheID != mShaderCacheid)
	{
		mShaderCacheid = shaderCacheID;

		std::ostringstream os;
		os << processor->getGpuShaderText(shaderDesc) << "\n";
		
		mGeneratedShaderCode = os.str();
		replaceAll(mGeneratedShaderCode, std::string("texture3D"), std::string("texture") );
	}

	return true;
}

bool ObjectFilterLUT::ReadyToApply(const CCompositionInfo *pInfo, const CProcessingInfo &prInfo)
{
	if (false == Active)
		return false;

	if (0.0 == Opacity)
		return false;

	const bool useMask = UseCompositeMask;

	if ( Active && mNeedUpdate )
	{
		AllocateLut3D();
		CHECK_GL_ERROR_MOBU();

		if (strcmp(mLutFileLoaded.c_str(), FileName) != 0)
		{
			Active = UpdateOCIOGLState();
			mLutFileLoaded = FileName;
			mProgram = nullptr;
		}
		mNeedUpdate = false;
	}

	if (mLutFileLoaded.length() == 0)
		return false;	

	if (nullptr == mProgram)
	{
		FBString strHeader( "#version 430\n"
			"#extension GL_ARB_shading_language_include : require\n"
			"layout (local_size_x =32, local_size_y = 32) in;\n" );

		FBString strDefine( "" );

		// TODO: check when to apply same size difinition
		strDefine = strDefine + "#define SAME_SIZE\n";

		if (useMask)
			strDefine = strDefine + "#define USE_MASK\n";

		// use that for exclusive shader cache
		FBString fileNameOnly = ExtractFileName( FBString(mLutFileLoaded.c_str()) );
		FBString strInclude( "#include \"", fileNameOnly );
		strInclude = strInclude + "\"\n";
		
		strDefine = strDefine + strInclude;

		// TODO: upload generated shader code and include it
		int numberOfPreloadedParts = 0;
		CompositeComputeShader::CMixedProgramManager::instance().NewShaderCodeFromBuffer(fileNameOnly, mGeneratedShaderCode.c_str(), mGeneratedShaderCode.length(), numberOfPreloadedParts);
		CompositeComputeShader::CMixedProgramManager::instance().UploadProgramIncludeCodes( fileNameOnly );

		const char *programLabel = MixedProgramLabel();
		const char *programPath = MixedProgramPath();

		mProgram = CompositeComputeShader::CMixedProgramManager::instance().QueryAProgramMix(programLabel, strHeader, strDefine, programPath, "");
		
		bool shaderOk = mProgram->IsOk();
		if (false == shaderOk)
		{
			Active = false;
			return false;
		}

		// DONE: update uniform locations
		mProgramUniforms->Prep(mProgram->GetProgramId() );
	}

	if (nullptr == mProgram)
		return false;

	return true;
}

void ObjectFilterLUT::DoLoad()
{
	FBFilePopup		dialog;

	dialog.Style = kFBFilePopupOpen;
	dialog.Caption = "Please choose a LUT file";
	dialog.FullFilename = FileName;

	if (dialog.Execute() )
	{
		FileName = dialog.FullFilename;
		Active = true;
		mNeedUpdate = true;
	}
}

void ObjectFilterLUT::LoadFromFile(const char *filename)
{
	/*
	try
	{
		OCIO::ConstConfigRcPtr	config = OCIO::GetCurrentConfig();

		OCIO::FileTransformRcPtr transform = OCIO::FileTransform::Create();
		transform->setSrc(filename);

		transform->setCCCId(m_cccid.c_str());

		transform->setDirection(OCIO::TRANSFORM_DIR_FORWARD);
		transform->setInterpolation(OCIO::INTERP_LINEAR);

		m_processor = config->getProcessor(transform, OCIO::TRANSFORM_DIR_FORWARD);
	}
	catch(OCIO::Exception &e)
	{
		printf(e.what() );
		return;
	}
	*/
}

void ObjectFilterLUT::Cleanup()
{
	
	if (mLut3dTexID > 0)
	{
		glDeleteTextures(1, &mLut3dTexID);
		mLut3dTexID = 0;
	}

	InitParams();

	mNeedUpdate = true;
}