
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: compositeMaster_computeShaders.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "compositeMaster_computeShaders.h"

#include <iostream>
#include <functional>
#include <algorithm>

#include "IO\FileUtils.h"
#include "graphics\CheckGLError_MOBU.h"

using namespace CompositeComputeShader;

#define	STR_PART_IDENT			"//-- PART:"

#define DRAW_LOG_SHADER_VERTEX		"\\GLSL\\drawLog.vsh"
#define DRAW_LOG_SHADER_FRAGMENT	"\\GLSL\\drawLog.fsh"

#define DRAW_CUBEMAP_SHADER_VERTEX		"\\GLSL\\drawCubeMap.vsh"
#define DRAW_CUBEMAP_SHADER_FRAGMENT	"\\GLSL\\drawCubeMap.fsh"

/////////////////////////////////////////////////////////////////////////////////////////////////
//

CShaderCodeHolder::CShaderCodeHolder()
{
	/*
	mCodeBuffer = nullptr;
	mCodeBufferSize = 0;
	*/
	mStatus = false;
}

void CShaderCodeHolder::Clear()
{
	/*
	if (mCodeBuffer)
	{
		delete [] mCodeBuffer;
		mCodeBuffer = nullptr;
	}

	mCodeBufferSize = 0;
	*/
	mCodeBuffer.clear();
	mPartsMap.clear();
	mStatus = false;

	mIncludeVector.clear();
}

CShaderCodeHolder::~CShaderCodeHolder()
{
	Clear();
}


bool CShaderCodeHolder::LoadFromFilename(const char *filename)
{
	Clear();
	mStatus = false;

	FBString effectPath, effectFullName;
	if (false == FindEffectLocation( filename, effectPath, effectFullName ) )
		return false;

	FILE *fp = nullptr;
	fopen_s(&fp, effectFullName, "r");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		
		mCodeBuffer.resize(fileLen + 1, 0);

		// read shader from file
		char *buffer = mCodeBuffer.data();
		size_t readlen = fread_s(buffer, sizeof(char)*(fileLen+1), sizeof(char), fileLen, fp);
		
		// trick to zero all outside memory
		
		if (readlen == 0) //(readlen != len)
		{
			Clear();

			//ERR("glsl shader file size" );
			fclose(fp);
			return false;
		}
		else
		{
			for (int i=readlen; i<fileLen; ++i)
				buffer[i] = 0;
		}

		mFileName = filename;

		fclose(fp);

		//
		//
		PreparePartOffsets();
		PrepareIncludeList();

		mStatus = true;
		return true;
	}

	return false;
}

bool CShaderCodeHolder::LoadFromBuffer(const char *filename, bool virtualFilename, const char *buffer, const size_t bufferLen)
{
	Clear();
	mStatus = false;

	mCodeBuffer.resize(bufferLen + 1, 0);

	// read shader from file
	
	memcpy(mCodeBuffer.data(), buffer, bufferLen);

	mFileName = filename;

	//
	//
	PreparePartOffsets();
	PrepareIncludeList();

	mStatus = true;
	return true;
}

void CShaderCodeHolder::PreparePartOffsets()
{
	mPartsMap.clear();
	if (mCodeBuffer.size() < 1)
		return;

	// search for //-- PART: <name> \n

	char *buffer = mCodeBuffer.data();
	char *bufferEnd = mCodeBuffer.data() + (mCodeBuffer.size()-1);

	char *searchPos = strstr(buffer, STR_PART_IDENT);

	while (searchPos != nullptr)
	{

		// todo search for the end of a line (contains part name and end line symbol \n )
		int linelen = 0;
		char *pos = searchPos;
		while (pos != bufferEnd && pos[0] != '\n')
		{
			linelen += 1;
			pos++;
		}

		std::string strPart(&searchPos[10], linelen-1);
		std::remove_if(strPart.begin(), strPart.end(), ::isspace);

		std::hash<std::string> hash_fn;
		size_t str_hash = hash_fn(strPart);
		//
		mPartsMap[str_hash] = searchPos;

		// search for next part
		buffer = &searchPos[1];
		searchPos = strstr(buffer, STR_PART_IDENT);
	}
}

void CShaderCodeHolder::PrepareIncludeList()
{
	char *buffer = mCodeBuffer.data();
	char *bufferEnd = mCodeBuffer.data() + (mCodeBuffer.size()-1);

	size_t beginAddress = (size_t) buffer;

	char *commentBlock = strstr(buffer, "/*");
	char *comment = strstr(buffer, "//");
	char *include = strstr(buffer, "#include");

	while (include != nullptr)
	{

		if (include)
		{
			bool skipThisIteration = false;

			size_t includeAddr = (size_t) include - beginAddress;

			if (commentBlock)
			{
				size_t commentBlockAddr = (size_t) commentBlock - beginAddress;

				if (commentBlockAddr < includeAddr)
				{
					// skip comment block
					buffer = commentBlock;
					while (buffer != bufferEnd && buffer[0] != '*' && buffer[1] != '/')
					{
						buffer++;
					}
					skipThisIteration = true;
				}
			}
			
			if (comment)
			{
				size_t commentAddr = (size_t) comment - beginAddress;

				if (commentAddr < includeAddr)
				{
					// skip comment line
					buffer = comment;
					while (buffer != bufferEnd && buffer[0] != '\n')
					{
						buffer++;
					}
					skipThisIteration = true;
				}
			}
			

			if (false == skipThisIteration)
			{
				//
				// parse include up to the end of a line
				buffer = include;

				int linelen = 0;
				char *pos = strstr(include, "\"");

				if (pos != nullptr && pos != bufferEnd)
				{
					buffer = pos + 1;
					pos = pos + 1;	// skip quote
				}
			
				while (pos != nullptr && pos != bufferEnd && pos[0] != '\n' && pos[0] != '\"')
				{
					linelen += 1;
					pos++;
				}

				if (linelen > 0)
				{
					std::string strLine(buffer, linelen);
					std::remove_if(strLine.begin(), strLine.end(), ::isspace);
			
					// parse line to extract include name inside the brackets
					mIncludeVector.push_back(strLine);
				}
			}
		}


		// try next iteration
		commentBlock = strstr(buffer, "/*");
		comment = strstr(buffer, "//");
		include = strstr(buffer, "#include");
	}
}

bool CShaderCodeHolder::Reload()
{
	if (mFileName.size() > 0)
		return LoadFromFilename(mFileName.c_str() );

	return false;
}

const char *CShaderCodeHolder::GetBufferPtr(size_t &bufferSize, const char *partName) const
{
	bufferSize = 0;

	if (partName == nullptr || strlen(partName) == 0)
	{
		bufferSize = mCodeBuffer.size();
		return mCodeBuffer.data();
	}
	else
	{
		std::hash<std::string> hash_fn;
		size_t str_hash = hash_fn(partName);

		auto partiter = mPartsMap.find(str_hash);
		if (partiter != end(mPartsMap) )
		{
			char *bufferBegin = partiter->second;
			char *bufferEnd = (char*) mCodeBuffer.data() + (mCodeBuffer.size()-1);

			char *nextPartPos = strstr( &bufferBegin[1], STR_PART_IDENT );
			if (nextPartPos != nullptr)
				bufferEnd = nextPartPos;

			size_t size = 0;

			while (bufferEnd != bufferBegin)
			{
				size += 1;
				bufferEnd--;
			}

			bufferSize = size;
			return bufferBegin;
		}
	}

	return nullptr;
}

const int CShaderCodeHolder::GetNumberOfParts() const
{
	return (int) mPartsMap.size();
}

/*
const int CShaderCodeHolder::GetBufferSize(const char *partName) const
{
	return mCodeBufferSize;
}
*/

const int CShaderCodeHolder::GetNumberOfIncludeFiles() const
{
	return (int) mIncludeVector.size();
}

const char *CShaderCodeHolder::GetIncludeFilename(const int index) const
{
	return mIncludeVector[index].c_str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

CMixedProgram::CMixedProgram(const size_t key, const GLuint shaderid, const GLuint programid)
	: mCombinationKey(key)
	, mShader(shaderid)
	, mProgram(programid)
{
	mStatus = false;
}

CMixedProgram::~CMixedProgram()
{
	Clear();
}

void CMixedProgram::Clear()
{
	if (mShader > 0)
	{
		if (mProgram > 0)
			glDetachShader(mProgram, mShader);
		glDeleteShader(mShader);
		mShader = 0;
	}
	if (mProgram > 0)
	{
		glDeleteProgram(mProgram);
		mProgram = 0;
	}
}

void CMixedProgram::CreateGLObjects()
{
	Clear();
	// TODO:
}

void CMixedProgram::ReCreateShaderObject()
{
	if (mProgram > 0 && mShader > 0)
	{
		glDetachShader(mProgram, mShader);
		glDeleteShader(mShader);

		mShader = glCreateShader(GL_COMPUTE_SHADER);
		glAttachShader(mProgram, mShader);
	}
}

void CMixedProgram::Bind()
{
	if (mProgram > 0)
		glUseProgram(mProgram);
}

void CMixedProgram::UnBind()
{
	glUseProgram(0);
}

void CMixedProgram::DispatchPipeline(const int groups_x, const int groups_y, const int groups_z)
{
	glDispatchCompute(groups_x, groups_y, groups_z);
}


/////////////////////////////////////////////////////////////////////////////////////////////////

CMixedProgramManager::CMixedProgramManager()
{
	mIncludeRootPath = "\\GLSL_CS\\";
}

CMixedProgramManager::~CMixedProgramManager()
{
	//ChangeContext();
	FreeCodeHolders();
}

bool CMixedProgramManager::checkCompileStatus(GLuint shader, const char *shadername)
{
    GLint  compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        std::cerr << shadername << " failed to compile:" << std::endl;
        GLint  logSize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
        char* logMsg = new char[logSize];
        glGetShaderInfoLog(shader, logSize, NULL, logMsg);
        std::cerr << logMsg << std::endl;
        delete[] logMsg;

		return false;
    }
	return true;
}

bool CMixedProgramManager::checkLinkStatus(GLuint program, const char * programName)
{
    GLint  linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        std::cerr << "Shader program " << programName << " failed to link" << std::endl;
        GLint  logSize;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
        char* logMsg = new char[logSize];
        glGetProgramInfoLog(program, logSize, NULL, logMsg);
        std::cerr << logMsg << std::endl;
        delete[] logMsg;
        system("Pause");
        // exit(0);
		return false;
    }
	return true;
}

bool CMixedProgramManager::loadComputeShaderFromBuffer(const char* buffer, const char *shaderName, const GLuint shaderid, const GLuint programid)
{
	const GLcharARB*  bufferARB = buffer;

	//GLuint shaderCompute = glCreateShader(GL_COMPUTE_SHADER);
	GLuint shaderCompute = shaderid;
	glShaderSource(shaderCompute, 1, &bufferARB, NULL);
	
	if (GLEW_ARB_shading_language_include)
	{
		std::string rootPath = "/";
		const char *SourceString = rootPath.c_str();
		glCompileShaderIncludeARB(shaderCompute, 1, &SourceString, NULL);
	}
	else
	{
		glCompileShader(shaderCompute);
	}

	if (false == checkCompileStatus(shaderCompute, shaderName) )
		return false;
		
	//GLuint programCompute = glCreateProgram();
	//glAttachShader(programCompute, shaderCompute);
	GLuint programCompute = programid;
	glLinkProgram(programCompute);
	if (false == checkLinkStatus(programCompute, shaderName) )
		return false;
	/*
	shaderid = shaderCompute;
	programid = programCompute;
	*/
	return true;
}

bool CMixedProgramManager::loadComputeShader(const char* computeShaderName, const GLuint shaderid, const GLuint programid)
{
	//shaderid = 0;
	//programid = 0;
	bool lSuccess = false;

	FILE *fp = nullptr;
	fopen_s(&fp, computeShaderName, "r");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t fileLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char  *buffer = new char[ fileLen + 1 ];
		
		GLint   len = (GLint) fileLen;

		// read shader from file
		memset( &buffer[0], 0, sizeof(char)*(len + 1) );
		
		size_t readlen = fread(buffer, sizeof(char), fileLen, fp);
  
		// trick to zero all outside memory
		memset( &buffer[readlen], 0, sizeof(char)*(len + 1 - readlen) );

		if (readlen == 0) //(readlen != len)
		{
			//ERR("glsl shader file size" );
			fclose(fp);
			return 0;
		}

		if (true == loadComputeShaderFromBuffer(buffer, computeShaderName, shaderid, programid) )
		{
			lSuccess = true;
		}

		if (buffer) {
			delete[] buffer;
			buffer = NULL;
		}

		fclose(fp);
	}

    return lSuccess;
}

void CMixedProgramManager::LoadDefaultShaders()
{

}

int CMixedProgramManager::MakeAKey(unsigned char header, unsigned char blendmode, unsigned char mask, unsigned char effect)
{
	int key = header;

	key = key | (blendmode << 8);
	key = key | (mask << 16);
	key = key | (effect << 24);

	return key;
}

void CMixedProgramManager::ChangeContext()
{
	// we have to recompile shaders from the beginning

	for (auto iter=begin(mMixedMap); iter!=end(mMixedMap); ++iter)
	{
		CMixedProgram *pProgram = iter->second;

		if (pProgram)
		{
			pProgram->Clear();
			delete pProgram;
		}
	}

	mMixedMap.clear();
	mMixedToShaderCodeMap.clear();

	if ( mDrawShader.get() != nullptr )
	{
		mDrawShader->Free();
		mDrawShader.reset(nullptr);
	}
}

const int CMixedProgramManager::PreLoadNewShaderCode(const char *filename, int &numberOfPreloadedParts)
{
	size_t hashKey = MakeAHashKey(filename);

	int index = -1;
	numberOfPreloadedParts = 0;

	auto searchIter = mShaderCodeMap.find(hashKey);
	if (searchIter == end(mShaderCodeMap) )
	{
		CShaderCodeHolder *newHolder = new CShaderCodeHolder();
		
		if (false == newHolder->LoadFromFilename(filename) )
		{
			delete newHolder;
			return -1;
		}

		mShaderCodeVector.push_back(newHolder);
		index = (int) mShaderCodeVector.size()-1;
		mShaderCodeMap[hashKey] = index;

		for (int i=0, count=newHolder->GetNumberOfIncludeFiles(); i<count; ++i)
		{
			std::string filepath(mIncludeRootPath);
			filepath += newHolder->GetIncludeFilename(i);

			PreLoadNewShaderCode( filepath.c_str(), numberOfPreloadedParts );
		}
	}
	else
	{
		index = searchIter->second;

		// TODO: update code here
	}

	numberOfPreloadedParts = mShaderCodeVector[index]->GetNumberOfParts();

	return index;
}

const int CMixedProgramManager::NewShaderCodeFromBuffer(const char *virtualFilename, const char *buffer, const size_t bufferLen, int &numberOfPreloadedParts)
{
	size_t hashKey = MakeAHashKey(virtualFilename);

	int index = -1;
	numberOfPreloadedParts = 0;

	auto searchIter = mShaderCodeMap.find(hashKey);
	if (searchIter == end(mShaderCodeMap) )
	{
		CShaderCodeHolder *newHolder = new CShaderCodeHolder();
		
		if (false == newHolder->LoadFromBuffer(virtualFilename, true, buffer, bufferLen) )
		{
			delete newHolder;
			return -1;
		}

		mShaderCodeVector.push_back(newHolder);
		index = (int) mShaderCodeVector.size()-1;
		mShaderCodeMap[hashKey] = index;

		// from buffer that is not possible I guess
		/*
		for (int i=0, count=newHolder->GetNumberOfIncludeFiles(); i<count; ++i)
		{
			std::string filepath(mIncludeRootPath);
			filepath += newHolder->GetIncludeFilename(i);

			PreLoadNewShaderCode( filepath.c_str(), numberOfPreloadedParts );
		}
		*/
	}
	else
	{
		index = searchIter->second;

		// DONE: update code here
		CShaderCodeHolder *newHolder = mShaderCodeVector[index];
		newHolder->LoadFromBuffer(virtualFilename, true, buffer, bufferLen);
	}

	numberOfPreloadedParts = mShaderCodeVector[index]->GetNumberOfParts();

	return index;
}

CMixedProgram *CMixedProgramManager::QueryAProgramMix(const char *displayLabel, const char *header, const char *define, const char *filename, const char *partname)
{
	const size_t key = MakeAHashKey(define, filename, partname);

	auto searchIter = mMixedMap.find(key);
	if (searchIter == end(mMixedMap) )
	{
		std::vector<CShaderQuery>	query;
		query.push_back( CShaderQuery(define, filename, partname) );

		return MakeANewProgramMix(displayLabel, header, query);
	}

	return searchIter->second;
}

CMixedProgram *CMixedProgramManager::QueryAProgramMix(const char *displayLabel, const char *header, std::vector<CShaderQuery> &query)
{

	const size_t key = MakeAHashKey(query);

	auto searchIter = mMixedMap.find(key);
	if (searchIter == end(mMixedMap) )
	{
		return MakeANewProgramMix(displayLabel, header, query);
	}

	return searchIter->second;
}

const size_t CMixedProgramManager::MakeAHashKey(const char *filename) const
{
	std::hash<std::string> hash_fn;
	size_t str_hash = hash_fn(filename);

	return str_hash;
}

const size_t CMixedProgramManager::MakeAHashKey(std::vector<CShaderQuery> &query) const
{
	std::string str_all("");

	for (auto iter=begin(query); iter!=end(query); ++iter)
	{
		str_all += iter->strDefine;
		str_all += iter->filename;
		str_all += iter->partname;
	}

	std::hash<std::string> hash_fn;
	size_t str_hash = hash_fn(str_all);

	return str_hash;
}

const size_t CMixedProgramManager::MakeAHashKey(const char *define, const char *filename, const char *partname) const
{
	std::string str_all("");

	if (define != nullptr)
		str_all += define;
	if (filename != nullptr)
		str_all += filename;
	if (partname != nullptr)
		str_all += partname;

	std::hash<std::string> hash_fn;
	size_t str_hash = hash_fn(str_all);

	return str_hash;
}

bool CMixedProgramManager::CompileProgramMix(CMixedProgram *mixedProgram, const char *displayLabel, const char *header, std::vector<CShaderQuery> &query)
{
	bool isSuccess=true;
	mixedProgram->mStatus = false;
	// buffer size

	size_t bufferSize = 0;
	if (header != nullptr && strlen(header) > 0)
		bufferSize += strlen(header);

	// check all the query files
	std::vector<std::string>	filesToLoad;
	std::vector<std::string>	includeVector;

	for (auto iter=begin(query); iter!=end(query); ++iter)
	{
		size_t hash_key = MakeAHashKey(iter->filename.c_str() );

		auto codeIter = mShaderCodeMap.find(hash_key);
		int codeIndex = -1;

		if (codeIter == end(mShaderCodeMap) )
		{
			// we should try to load a file
			int numberOfPreloadedParts = 0;
			codeIndex = PreLoadNewShaderCode(iter->filename.c_str(), numberOfPreloadedParts);
		}
		else
			codeIndex = codeIter->second;

		iter->fileCodeIndex = codeIndex;

		if (codeIndex < 0)
		{
			isSuccess = false;
			break;
		}

		size_t partBufferSize = 0;
		mShaderCodeVector[codeIndex]->GetBufferPtr(partBufferSize, iter->partname.c_str() );

		bufferSize = bufferSize + iter->strDefine.size(); // when symbol = char, then size = count
		bufferSize = bufferSize + partBufferSize;
	}

	// if failed to load any of shader code
	if (false == isSuccess)
	{
		return nullptr;
	}

	// allocate memory

	char *mixbuffer = new char[bufferSize+1];
	memset(mixbuffer, 0, sizeof(char) * (bufferSize+1) );

	// fill mixbuffer with shader codes and defines
	char *mixbufferPos = mixbuffer;

	if (header != nullptr && strlen(header) > 0)
	{
		memcpy(mixbufferPos, header, sizeof(char) * strlen(header) );
		mixbufferPos += strlen(header);
	}

	for (auto iter=begin(query); iter!=end(query); ++iter)
	{
		size_t hash_key = MakeAHashKey(iter->filename.c_str() );

		auto codeIter = mShaderCodeMap.find(hash_key);
		int codeIndex = -1;

		if (codeIter != end(mShaderCodeMap) )
		{
			codeIndex = codeIter->second;
		}

		if(codeIndex == -1)
			continue;

		CShaderCodeHolder *pHolder = mShaderCodeVector[codeIndex];

		memcpy(mixbufferPos, iter->strDefine.c_str(), iter->strDefine.size() );
		mixbufferPos = mixbufferPos + iter->strDefine.size();

		
		size_t localBufferSize=0;
		const char *localBuffer = pHolder->GetBufferPtr(localBufferSize);

		memcpy(mixbufferPos, localBuffer, localBufferSize);
		mixbufferPos = mixbufferPos + localBufferSize;

		//
		//
		if (pHolder->GetNumberOfIncludeFiles() > 0)
		{
			for (int i=0, count=pHolder->GetNumberOfIncludeFiles(); i<count; ++i)
			{
				includeVector.push_back( pHolder->GetIncludeFilename(i) );
			}
		}
	}

	//
	// check for include and pre-cache include shader codes
	if (false == UploadProgramIncludeCodes( includeVector ) )
	{
		isSuccess = false;
		mixedProgram->mStatus = false;
	}

	//
	//
	if (isSuccess)
	{
		if (false == loadComputeShaderFromBuffer(mixbuffer, mixedProgram->mDisplayLabel.c_str(), mixedProgram->GetShaderId(), mixedProgram->GetProgramId()) )
		{
			isSuccess = false;
			mixedProgram->mStatus = false;
		}
		else
		{
			mixedProgram->mStatus = true;
		}
	}

	// free memory
	if (mixbuffer)
	{
		delete [] mixbuffer;
		mixbuffer = nullptr;
	}

	return isSuccess;
}

CMixedProgram *CMixedProgramManager::MakeANewProgramMix(const char *displayLabel, const char *header, std::vector<CShaderQuery> &query)
{
	// compile a shader and make a new mixed program

	GLuint shaderid = glCreateShader(GL_COMPUTE_SHADER);
		
	GLuint programid = glCreateProgram();
	glAttachShader(programid, shaderid);
	
	CMixedProgram *newMix = nullptr;
	if (programid > 0)
	{
		size_t key = MakeAHashKey(query);
		newMix = new CMixedProgram(key, shaderid, programid);
		newMix->mDisplayLabel = displayLabel;
		newMix->mHeader = header;

		mMixedMap[key] = newMix;

		// compile and link mixed shader
		CompileProgramMix(newMix, displayLabel, header, query);

		newMix->mQuery = query;
	}

	return newMix;
}

void CMixedProgramManager::FreeCodeHolders()
{
	for (auto iter=begin(mShaderCodeVector); iter!=end(mShaderCodeVector); ++iter)
	{
		CShaderCodeHolder *pHolder = *iter;
		
		if (pHolder)
			delete pHolder;
	}

	mShaderCodeVector.clear();
	mShaderCodeMap.clear();
}

bool CMixedProgramManager::ReloadProgram(CMixedProgram *mixedProgram)
{
	std::vector<CMixedProgram*>	recompileVector;

	for (auto queryIter=begin(mixedProgram->mQuery); queryIter!=end(mixedProgram->mQuery); ++queryIter)
	{
		CShaderCodeHolder *pHolder = mShaderCodeVector[queryIter->fileCodeIndex];

		if (pHolder)
		{
			if (false == pHolder->Reload() )
			{
				return false;
			}

			// TODO: reload all include files
			for (int i=0, count=pHolder->GetNumberOfIncludeFiles(); i<count; ++i)
			{
				std::string strname(mIncludeRootPath);
				strname += pHolder->GetIncludeFilename(i);

				size_t hash_key = MakeAHashKey( strname.c_str() );

				auto codeIter = mShaderCodeMap.find(hash_key);
				int codeIndex = -1;

				if (codeIter != end(mShaderCodeMap) )
				{
					codeIndex = codeIter->second;
				}

				if (codeIndex == -1)
					return false;

				CShaderCodeHolder *pIncludeHolder = mShaderCodeVector[codeIndex];
				if (false == pIncludeHolder->Reload() )
				{
					return false;
				}
			}
		}

		for (auto iterPr=begin(mMixedMap); iterPr!=end(mMixedMap); ++iterPr)
		{
			CMixedProgram *iterProgram = iterPr->second;
			auto recompileIter = std::find(begin(recompileVector), end(recompileVector), iterProgram);

			if ( recompileIter==end(recompileVector) && iterProgram->HasShaderCode(queryIter->fileCodeIndex) )
			{
				recompileVector.push_back(iterPr->second);
			}
		}
	}

	//
	for (auto iterPr=begin(recompileVector); iterPr!=end(recompileVector); ++iterPr)
	{
		CMixedProgram *pMixedProgram = *iterPr;
		// recompile shader using the same define and shader codes
		pMixedProgram->ReCreateShaderObject();
		CompileProgramMix(pMixedProgram, "recompile mixed shaders", pMixedProgram->mHeader.c_str(), pMixedProgram->mQuery);
	}
	
	return true;
}

bool CMixedProgramManager::UploadProgramIncludeCodes(const char *ShaderCodeName)
{
	size_t hash_key = MakeAHashKey( ShaderCodeName );

	auto codeIter = mShaderCodeMap.find(hash_key);
	int codeIndex = -1;

	if (codeIter != end(mShaderCodeMap) )
	{
		codeIndex = codeIter->second;
	}

	if (codeIndex == -1)
		return false;

	CShaderCodeHolder *pIncludeHolder = mShaderCodeVector[codeIndex];
	
	size_t bufferSize = 0;
	const char *buffer = pIncludeHolder->GetBufferPtr(bufferSize);

	if (bufferSize > 0)
	{
		std::string strname("/");
		strname += ShaderCodeName;

		glNamedStringARB(GL_SHADER_INCLUDE_ARB, strname.length(), strname.c_str(), (GLint) bufferSize, buffer);
		CHECK_GL_ERROR_MOBU();
	}

	return true;
}

bool CMixedProgramManager::UploadProgramIncludeCodes(const std::vector<std::string> &includeVector)
{

	for (auto iter=begin(includeVector); iter!=end(includeVector); ++iter)
	{
		std::string strname(mIncludeRootPath);
		strname += *iter;

		size_t hash_key = MakeAHashKey( strname.c_str() );

		auto codeIter = mShaderCodeMap.find(hash_key);
		int codeIndex = -1;

		if (codeIter != end(mShaderCodeMap) )
		{
			codeIndex = codeIter->second;
		}

		if (codeIndex == -1)
			return false;

		CShaderCodeHolder *pIncludeHolder = mShaderCodeVector[codeIndex];
	
		size_t bufferSize = 0;
		const char *buffer = pIncludeHolder->GetBufferPtr(bufferSize);

		if (bufferSize > 0)
		{
			std::string strname("/");
			strname += *iter;

			glNamedStringARB(GL_SHADER_INCLUDE_ARB, strname.length(), strname.c_str(), (GLint) bufferSize, buffer);
			CHECK_GL_ERROR_MOBU();
		}
	}

	return true;
}

GLSLShader *CMixedProgramManager::QueryLogDrawShader()
{

	if ( mDrawShader.get() == nullptr )
	{
		// try to load a new shader

		FBString effectPath, effectFullName;
		if (true == FindEffectLocation( DRAW_LOG_SHADER_VERTEX, effectPath, effectFullName ) )
		{
			GLSLShader *pNewShader = new GLSLShader();

			FBString vertexPath( effectPath, DRAW_LOG_SHADER_VERTEX );
			FBString fragmentPath( effectPath, DRAW_LOG_SHADER_FRAGMENT );

			if (false == pNewShader->LoadShaders( vertexPath, fragmentPath ) )
			{
				delete pNewShader;
				pNewShader = nullptr;
			}

			mDrawShader.reset(pNewShader);
		}
	}

	return mDrawShader.get();
}

GLSLShader *CMixedProgramManager::QueryCubeMapShader()
{

	if ( mCubeMapShader.get() == nullptr )
	{
		// try to load a new shader

		FBString effectPath, effectFullName;
		if (true == FindEffectLocation( DRAW_CUBEMAP_SHADER_VERTEX, effectPath, effectFullName ) )
		{
			GLSLShader *pNewShader = new GLSLShader();

			FBString vertexPath( effectPath, DRAW_CUBEMAP_SHADER_VERTEX );
			FBString fragmentPath( effectPath, DRAW_CUBEMAP_SHADER_FRAGMENT );

			if (false == pNewShader->LoadShaders( vertexPath, fragmentPath ) )
			{
				delete pNewShader;
				pNewShader = nullptr;
			}

			mCubeMapShader.reset(pNewShader);
		}
	}

	return mCubeMapShader.get();
}