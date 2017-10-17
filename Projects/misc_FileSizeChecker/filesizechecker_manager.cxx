
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: filesizechecker_manager.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "filesizechecker_manager.h"

//--- Registration defines
#define ORCUSTOMMANAGER_TEMPLATE__CLASS ORCUSTOMMANAGER_TEMPLATE__CLASSNAME
#define ORCUSTOMMANAGER_TEMPLATE__NAME  ORCUSTOMMANAGER_TEMPLATE__CLASSSTR

//--- FiLMBOX implementation and registration
FBCustomManagerImplementation( ORCUSTOMMANAGER_TEMPLATE__CLASS  );  // Manager class name.
FBRegisterCustomManager( ORCUSTOMMANAGER_TEMPLATE__CLASS );         // Manager class name.

/************************************************
 *  FiLMBOX Creation
 ************************************************/
bool FileSizeChecker_Manager::FBCreate()
{
	FBApplication::TheOne().OnFileSaveCompleted.Add( this, (FBCallback) &FileSizeChecker_Manager::EventFileSaveCompleted );
	return true;
}


/************************************************
 *  FiLMBOX Destruction.
 ************************************************/
void FileSizeChecker_Manager::FBDestroy()
{
    // Free any user memory here.
	FBApplication::TheOne().OnFileSaveCompleted.Remove( this, (FBCallback) &FileSizeChecker_Manager::EventFileSaveCompleted );
}

void FileSizeChecker_Manager::EventFileSaveCompleted( HISender pSender, HKEvent pEvent )
{
	const long limit = 2147483647;
	long len = 0;

	FBString fileName = FBApplication::TheOne().FBXFileName;

	FILE *f = fopen(fileName, "rb");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		len = ftell(f);
		fclose(f);
	}

	if (len >= limit || len < 0)
	{
		FBMessageBox( "SAVING OPERATION WARNING", "Saved file size is under the 2Gb limit!", "Ok" );
	}
}

/************************************************
 *  Execution callback.
 ************************************************/
bool FileSizeChecker_Manager::Init()
{
    return true;
}

bool FileSizeChecker_Manager::Open()
{
    return true;
}

bool FileSizeChecker_Manager::Clear()
{
    return true;
}

bool FileSizeChecker_Manager::Close()
{
    return true;
}
