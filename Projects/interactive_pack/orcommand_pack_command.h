#ifndef __ORCOMMAND_PACK_COMMAND_H__
#define __ORCOMMAND_PACK_COMMAND_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orcommand_pack_command.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>

//--- Registration defines
#define ORCOMMANDBASE__CLASSNAME	ORCommand_Base
#define ORCOMMANDBASE__CLASSSTR		"ORCommand_Base"

#define ORCOMMANDGROUP__CLASSNAME	ORCommand_Group
#define ORCOMMANDGROUP__CLASSSTR	"ORCommand_Group"

#define ORCOMMANDREWIND__CLASSNAME	ORCommand_Rewind
#define ORCOMMANDREWIND__CLASSSTR	"ORCommand_Rewind"

#define ORCOMMANDCOLOR__CLASSNAME	ORCommand_Color
#define ORCOMMANDCOLOR__CLASSSTR	"ORCommand_Color"

#define ORCOMMANDSHAKE__CLASSNAME	ORCommand_CameraShake
#define ORCOMMANDSHAKE__CLASSSTR	"ORCommand_CameraShake"

#define ORCOMMANDZOOM__CLASSNAME	ORCommand_CameraZoom
#define ORCOMMANDZOOM__CLASSSTR		"ORCommand_CameraZoom"

///////////////////////////////////////////////////////////////

/**	Command Template.
		Special class to help make a connection between command and storyclip
*/
class ORCommand_Base : public FBCommand
{
	//--- FiLMBOX box declaration.
	FBCommandDeclare( ORCommand_Base, FBCommand );

public:

	FBPropertyString		ClipName;

public:
	virtual bool FBCreate();		//!< FiLMBOX creation function.
	virtual void FBDestroy();		//!< FiLMBOX destruction function.

	virtual bool	FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool	FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	void	OnSystemIdle(HISender pSender, HKEvent pEvent);
	
	FBStoryClip *GetClipPtr() {
		if (mClip.Ok() )
			return (FBStoryClip*) mClip.GetPlug();

		return nullptr;
	}

	virtual void UpdateReferences()
	{
	}

	virtual void DoRename();

protected:
	FBSystem							mSystem;
	HdlFBPlugTemplate<FBStoryClip>		mClip;

	FBString							mReadName;

	virtual void		OnInit();
};

//////////////////////////////////////////////////////////

enum ECommandGroupModes
{
	eCommandGroup_Hide,
	eCommandGroup_Show,
	eCommandGroup_HideAll,
	eCommandGroup_ShowAll,
	eCommandGroup_InverseAll
};

const char *FBPropertyBaseEnum<ECommandGroupModes>::mStrings[] = 
{
	"Hide",
	"Show",
	"Hide All",
	"Show All",
	"Inverse All",
	0
};

/**	Command Template.
*/
class ORCommand_Group : public ORCommand_Base
{
	//--- FiLMBOX box declaration.
	FBCommandDeclare( ORCommand_Group, ORCommand_Base );

public:
	virtual bool FBCreate();		//!< FiLMBOX creation function.
	virtual void FBDestroy();		//!< FiLMBOX destruction function.

	//!	Execute the command.
	virtual	void	ExecuteNotify();

	//! IO
	virtual bool	FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool	FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	void	OnRetrieveFinished(HISender pSender, HKEvent pEvent);

	FBPropertyBaseEnum<ECommandGroupModes>	CommandMode;
	FBPropertyListObject					Groups;

	virtual void UpdateReferences();

private:

	std::vector<HIObject>		mRetrieveComponents;

	virtual void		OnInit();
};


///////////////////////////////////////////////////////////////////////////////////////////////////////
/**	Command Color.
		Use this class to put colored markers on a timeline (jump to them by commands transport box)
*/

enum ECommandColorModes
{
	eCommandColor_White,
	eCommandColor_Black,
	eCommandColor_Red,
	eCommandColor_Green,
	eCommandColor_Blue,
	eCommandColor_Yellow,
	eCommandColor_Cyan,
	eCommandColor_Magenta
};

const char *FBPropertyBaseEnum<ECommandColorModes>::mStrings[] = 
{
	"White",
	"Black",
	"Red",
	"Green",
	"Blue",
	"Yellow",
	"Cyan",
	"Magenta",
	0
};

class ORCommand_Color : public ORCommand_Base
{
	//--- FiLMBOX box declaration.
	FBCommandDeclare( ORCommand_Color, ORCommand_Base );

public:
	virtual bool FBCreate();		//!< FiLMBOX creation function.
	virtual void FBDestroy();		//!< FiLMBOX destruction function.

	virtual bool	FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool	FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	FBPropertyBaseEnum<ECommandColorModes>		ColorMode;

	void SetClipColorMode( const ECommandColorModes mode );

	virtual void UpdateReferences();

protected:

	bool				mRetreived;

	virtual void		OnInit();
};

///////////////////////////////////////////////////////////////////////////////////
//

enum ECommandRewindModes
{
	eCommandRewind_Start,
	eCommandRewind_FirstColor,
	eCommandRewind_PrevColor,
	eCommandRewind_AnyPrevColor
};

const char *FBPropertyBaseEnum<ECommandRewindModes>::mStrings[] = 
{
	"Start",
	"First Color",
	"Prev Color",
	"Any Prev Color",
	0
};

/**	Command Template.
*/
class ORCommand_Rewind : public ORCommand_Base
{
	//--- FiLMBOX box declaration.
	FBCommandDeclare( ORCommand_Rewind, ORCommand_Base );

public:
	virtual bool FBCreate();		//!< FiLMBOX creation function.
	virtual void FBDestroy();		//!< FiLMBOX destruction function.

	//!	Execute the command.
	virtual	void	ExecuteNotify();

	virtual bool	FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool	FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	FBPropertyBaseEnum<ECommandRewindModes>		RewindMode;
	FBPropertyBaseEnum<ECommandColorModes>		ColorMode;

	virtual void UpdateReferences();

private:

	virtual void		OnInit();
};

/////////////////////////////////////////////////////////////////////////////////////////////
/**	Command Camera Shake.
		Use this class to run a current camera shake effect.
*/

class ORCommand_CameraShake : public ORCommand_Base
{
	//--- FiLMBOX box declaration.
	FBCommandDeclare( ORCommand_CameraShake, ORCommand_Base );

public:
	virtual bool FBCreate();		//!< FiLMBOX creation function.
	virtual void FBDestroy();		//!< FiLMBOX destruction function.

	//!	Execute the command.
	virtual	void	ExecuteNotify();

	virtual bool	FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool	FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );


	FBPropertyDouble	Amplitude;
	FBPropertyDouble	Frequency;
	FBPropertyDouble	Duration;

	virtual void UpdateReferences();

protected:

	virtual void		OnInit();
};

/**	Command Camera Zoom.
		Use this class to run a current camera zoom effect.
*/

class ORCommand_CameraZoom : public ORCommand_Base
{
	//--- FiLMBOX box declaration.
	FBCommandDeclare( ORCommand_CameraZoom, ORCommand_Base );

public:
	virtual bool FBCreate();		//!< FiLMBOX creation function.
	virtual void FBDestroy();		//!< FiLMBOX destruction function.

	//!	Execute the command.
	virtual	void	ExecuteNotify();

	virtual bool	FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );
	virtual bool	FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );


	FBPropertyDouble	Amplitude;
	FBPropertyDouble	Duration;

	virtual void UpdateReferences();

protected:

	virtual void		OnInit();
};

#endif /* __ORCOMMAND_TEMPLATE_COMMAND_H__ */
