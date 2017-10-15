
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: orcommand_pack_command.cxx
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//--- Class declaration
#include "orcommand_pack_command.h"
#include "Common.h"

//--- Registration defines
#define ORCOMMANDBASE__CLASS		ORCOMMANDBASE__CLASSNAME

#define ORCOMMANDGROUP__CLASS		ORCOMMANDGROUP__CLASSNAME
#define ORCOMMANDGROUP__NAME		ORCOMMANDGROUP__CLASSSTR
#define	ORCOMMANDGROUP__LABEL		"Show \\ Hide Groups"
#define ORCOMMANDGROUP__DESC		"Show \\ Hide Groups Command"

#define ORCOMMANDREWIND__CLASS		ORCOMMANDREWIND__CLASSNAME
#define ORCOMMANDREWIND__NAME		ORCOMMANDREWIND__CLASSSTR
#define	ORCOMMANDREWIND__LABEL		"Player Rewind"
#define ORCOMMANDREWIND__DESC		"Player Control Rewind Command"

#define ORCOMMANDCOLOR__CLASS		ORCOMMANDCOLOR__CLASSNAME
#define ORCOMMANDCOLOR__NAME		ORCOMMANDCOLOR__CLASSSTR
#define	ORCOMMANDCOLOR__LABEL		"Color"
#define ORCOMMANDCOLOR__DESC		"Color Command"

#define ORCOMMANDSHAKE__CLASS		ORCOMMANDSHAKE__CLASSNAME
#define ORCOMMANDSHAKE__NAME		ORCOMMANDSHAKE__CLASSSTR
#define	ORCOMMANDSHAKE__LABEL		"Camera Shake"
#define ORCOMMANDSHAKE__DESC		"Camera Shake Command"

#define ORCOMMANDZOOM__CLASS		ORCOMMANDZOOM__CLASSNAME
#define ORCOMMANDZOOM__NAME			ORCOMMANDZOOM__CLASSSTR
#define	ORCOMMANDZOOM__LABEL		"Camera Zoom"
#define ORCOMMANDZOOM__DESC			"Camera Zoom Command"


////////////////////////////////////////////

#define FBRegisterCommand2( UniqueNameStr, ClassName, Label, Description, IconFilename ) \
	HIObject RegisterCommand##ClassName( HIObject /*pOwner*/,const char* pName,void * /*pData*/) \
	{ \
		ClassName *Class = new ClassName( Label ); \
		if (Class->FBCreate()) { \
			return Class->GetHIObject(); \
		} else { \
			delete Class; \
			return NULL; \
		} \
	} \
	FBLibraryModule( ClassName )	\
	{	\
		FBRegisterObject( ClassName##R1,"Commands",Label,Description,RegisterCommand##ClassName, true, IconFilename );						\
		FBRegisterObject( ClassName##R2,"FbxStorable/Command",UniqueNameStr,Description,RegisterCommand##ClassName, true, IconFilename );	\
		FBRegisterObject( ClassName##R3,"Browsing/Templates/Commands/Interactive",Label,Description,RegisterCommand##ClassName, true, "object_command.tif" );	\
	}	\

//--- FiLMBOX implementation and registration
FBCommandImplementation	(	ORCOMMANDBASE__CLASS	);		// Command class name.

FBCommandImplementation	(	ORCOMMANDGROUP__CLASS	);		// Command class name.
FBRegisterCommand2		(	ORCOMMANDGROUP__NAME,			// Command unique identifier.
							ORCOMMANDGROUP__CLASS,			// Command class name.
							ORCOMMANDGROUP__LABEL,			// Command label (what is displayed).
							ORCOMMANDGROUP__DESC,			// Command description.
							FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)

FBCommandImplementation	(	ORCOMMANDREWIND__CLASS	);		// Command class name.
FBRegisterCommand2		(	ORCOMMANDREWIND__NAME,			// Command unique identifier.
							ORCOMMANDREWIND__CLASS,			// Command class name.
							ORCOMMANDREWIND__LABEL,			// Command label (what is displayed).
							ORCOMMANDREWIND__DESC,			// Command description.
							FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)

FBCommandImplementation	(	ORCOMMANDCOLOR__CLASS	);		// Command class name.
FBRegisterCommand2		(	ORCOMMANDCOLOR__NAME,			// Command unique identifier.
							ORCOMMANDCOLOR__CLASS,			// Command class name.
							ORCOMMANDCOLOR__LABEL,			// Command label (what is displayed).
							ORCOMMANDCOLOR__DESC,			// Command description.
							FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)

FBCommandImplementation	(	ORCOMMANDSHAKE__CLASS	);		// Command class name.
FBRegisterCommand2		(	ORCOMMANDSHAKE__NAME,			// Command unique identifier.
							ORCOMMANDSHAKE__CLASS,			// Command class name.
							ORCOMMANDSHAKE__LABEL,			// Command label (what is displayed).
							ORCOMMANDSHAKE__DESC,			// Command description.
							FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)

FBCommandImplementation	(	ORCOMMANDZOOM__CLASS	);		// Command class name.
FBRegisterCommand2		(	ORCOMMANDZOOM__NAME,			// Command unique identifier.
							ORCOMMANDZOOM__CLASS,			// Command class name.
							ORCOMMANDZOOM__LABEL,			// Command label (what is displayed).
							ORCOMMANDZOOM__DESC,			// Command description.
							FB_DEFAULT_SDK_ICON			);		// Icon filename (default=Open Reality icon)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

FBStoryClip *FindCommandClipByTrack(FBCommand *pCommand, FBStoryTrack *pTrack)
{
	FBStoryClip *pclip = nullptr;
	FBProperty *prop = nullptr;

	for (int i=0; i<pTrack->SubTracks.GetCount(); ++i)
	{
		pclip = FindCommandClipByTrack(pCommand, pTrack->SubTracks[i]);
		if (pclip != nullptr)
			return pclip;
	}

	if (pTrack->Type != kFBStoryTrackCommand)
		return nullptr;

	//
	for (int i=0; i<pTrack->Clips.GetCount(); ++i)
	{
		pclip = pTrack->Clips[i];
	
		// check if that clip is already used in some other existing command
		if (IsClipInUse(pclip) != nullptr)
			continue;

		//
		prop = pclip->PropertyList.Find("ClipColor");
		if (prop)
		{
			FBTime startTime(FBTime::MinusInfinity), stopTime(FBTime::MinusInfinity);

			prop = pclip->PropertyList.Find("Start");
			if (prop)
				prop->GetData(&startTime, sizeof(FBTime) );
			
			prop = pclip->PropertyList.Find("Stop");
			if (prop)
				prop->GetData(&stopTime, sizeof(FBTime) );

			if (startTime == pCommand->StartTime && stopTime == pCommand->StopTime)
				return pclip;
		}
	}

	return nullptr;
}

FBStoryClip *FindCommandClipByFolder(FBCommand *pCommand, FBStoryFolder *pFolder)
{
	FBStoryClip *pclip = nullptr;

	for (int i=0; i<pFolder->Childs.GetCount(); ++i)
	{
		pclip = FindCommandClipByFolder(pCommand, pFolder->Childs[i]);
		if (pclip!=nullptr)
			return pclip;
	}

	for (int i=0; i<pFolder->Tracks.GetCount(); ++i)
	{
		pclip = FindCommandClipByTrack(pCommand, pFolder->Tracks[i]);
		if (pclip != nullptr)
			return pclip;	
	}

	return nullptr;
}

FBStoryClip *FindCommandClip(FBCommand *pCommand)
{
	FBStory &story = FBStory::TheOne();
	return FindCommandClipByFolder(pCommand, story.RootFolder);
}

/************************************************
 *	FiLMBOX Creation
 ************************************************/
static const char *ORCommand_Base_GetName(HIObject object)
{
	ORCommand_Base *pBase = FBCast<ORCommand_Base>(object);
	if (pBase) 
	{
		if (pBase->GetClipPtr() )
			return pBase->GetClipPtr()->Name;
	}
	return 0;
}

static void ORCommand_Base_SetName(HIObject object, const char *value)
{
	ORCommand_Base *pBase = FBCast<ORCommand_Base>(object);
	if (pBase && value) 
	{
		if (pBase->GetClipPtr() )
			pBase->GetClipPtr()->Name = value;
	}
}

bool ORCommand_Base::FBCreate()
{
	Description = "Interactive pack command";

	FBPropertyPublish( this, ClipName, "Clip Name", ORCommand_Base_GetName, ORCommand_Base_SetName );

	mSystem.OnUIIdle.Add( this, (FBCallback) &ORCommand_Color::OnSystemIdle );

	mReadName = "";

	// Allocate any user memory here.
	return true;
}

void ORCommand_Base::OnInit()
{
}

void ORCommand_Base::OnSystemIdle(HISender pSender, HKEvent pEvent)
{
	mClip = FindCommandClip(this);

	// add rename property
	FBProperty *prop = mClip->PropertyList.Find( "ClipName" );
	if (prop == nullptr)
	{
		prop = mClip->PropertyCreate( "ClipName", kFBPT_Reference, "reference", false, false, &ClipName );
	}

	if (prop != nullptr && mReadName != "")
	{
		ClipName = mReadName;
		mReadName = "";
	}

	//
	OnInit();

	mSystem.OnUIIdle.Remove( this, (FBCallback) &ORCommand_Color::OnSystemIdle );

	RegisterCommand(this);
}

void ORCommand_Base::DoRename()
{
	if (mClip.Ok() )
	{
		char buffer[128];
		memset(buffer, 0, sizeof(char)*128);

		FBString newName( mClip->Name );
		sprintf_s( buffer, sizeof(char)*128, "%s", newName );

		if (1 == FBMessageBoxGetUserValue( "Command", "Please enter a new name", buffer, kFBPopupString, "Ok", "Cancel" ) )
		{
			newName = buffer;
			if (newName != "")
				mClip->Name = newName;
		}
	}
}

bool ORCommand_Base::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (mClip.Ok() && pStoreWhat == kAll)
	{
		pFbxObject->FieldWriteBegin( "CommandBaseName" );
		pFbxObject->FieldWriteC( ClipName );
		pFbxObject->FieldWriteEnd();
	}

	return true;
}

bool ORCommand_Base::FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	if (pStoreWhat == kAll)
	{
		mReadName = "";

		if (pFbxObject->FieldReadBegin( "CommandBaseName" ) == true )
		{
			mReadName = pFbxObject->FieldReadC();
			pFbxObject->FieldReadEnd();
		}
	}

	return true;
}


/************************************************
 *	FiLMBOX Destruction.
 ************************************************/
void ORCommand_Base::FBDestroy()
{
	// Free any user memory here.
	UnRegisterCommand(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************
 *	FiLMBOX Creation
 ************************************************/
bool ORCommand_Group::FBCreate()
{
	ParentClass::FBCreate();

	Description = "Show\\Hide specified groups";

	FBPropertyPublish( this, CommandMode, "Command Mode", nullptr, nullptr );
	FBPropertyPublish( this, Groups, "Groups", nullptr, nullptr );
	//Groups.SetFilter( FBGroup::GetInternalClassId() );
	//Groups.SetSingleConnect( false );

	// Allocate any user memory here.
	return true;
}

void ORCommand_Group::OnInit()
{
	UpdateReferences();
}

void ORCommand_Group::UpdateReferences()
{
	if (mClip.Ok() )
	{
		// DONE: add properties
		FBProperty *prop = mClip->PropertyList.Find( "Command Mode" );
		if (prop == nullptr)
		{
			prop = mClip->PropertyCreate( "Command Mode", kFBPT_Reference, "reference", false, false, &CommandMode );
		}

		prop = mClip->PropertyList.Find( "Groups" );
		if (prop == nullptr)
		{
			prop = mClip->PropertyCreate( "Groups", kFBPT_Reference, "reference", false, false, &Groups );
		}
	}
}

/************************************************
 *	FiLMBOX Destruction.
 ************************************************/
void ORCommand_Group::FBDestroy()
{
	// Free any user memory here.
	ParentClass::FBDestroy();
}


/************************************************
 *	Execution callback.
 ************************************************/
void ORCommand_Group::ExecuteNotify()
{
	FBTime startTime(StartTime);
	FBTime stopTime(StopTime);
	FBTime localTime(mSystem.LocalTime);
	if (localTime > startTime && localTime > stopTime)
		return;

	int mode = CommandMode.AsInt();

	FBScene *pScene = mSystem.Scene;

	switch(mode)
	{
	case 0:
		for (int i=0; i<Groups.GetCount(); ++i)
		{
			FBGroup *pgroup = (FBGroup*) Groups[i];
			pgroup->Show = false;
		}
		break;
	case 1:
		for (int i=0; i<Groups.GetCount(); ++i)
		{
			FBGroup *pgroup = (FBGroup*) Groups[i];
			pgroup->Show = true;
		}
		break;
	case 2:
		for (int i=0; i<pScene->Groups.GetCount(); ++i)
			pScene->Groups[i]->Show = false;
		break;
	case 3:
		for (int i=0; i<pScene->Groups.GetCount(); ++i)
			pScene->Groups[i]->Show = true;
		break;
	case 4:
		for (int i=0; i<pScene->Groups.GetCount(); ++i)
		{
			bool vis = pScene->Groups[i]->Show;
			pScene->Groups[i]->Show = !vis;
		}
		break;
	}
}


bool ORCommand_Group::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxStore(pFbxObject, pStoreWhat);

	if (mClip.Ok() && pStoreWhat == kAll)
	{
		pFbxObject->FieldWriteBegin( "CommandGroup" );

		int mode = CommandMode.AsInt();
		pFbxObject->FieldWriteI( mode );

		int count = Groups.GetCount();
		pFbxObject->FieldWriteI( count );
		for (int i=0; i<count; ++i)
			pFbxObject->FieldWriteObjectReference( Groups[i] );

		pFbxObject->FieldWriteEnd();
	}

	return true;
}
bool ORCommand_Group::FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAll)
	{
		mRetrieveComponents.clear();
		Groups.Clear();

		if (pFbxObject->FieldReadBegin( "CommandGroup" ) )
		{
			int mode = pFbxObject->FieldReadI();
			CommandMode.SetInt(mode);

			int count = pFbxObject->FieldReadI();
			mRetrieveComponents.resize(count);

			for (int i=0; i<count; ++i)
			{
				mRetrieveComponents[i] = pFbxObject->FieldReadObjectReference();
			}

			pFbxObject->FieldReadEnd();
		}

		mSystem.OnUIIdle.Add( this, (FBCallback) &ORCommand_Group::OnRetrieveFinished );
	}

	return true;
}

void ORCommand_Group::OnRetrieveFinished(HISender pSender, HKEvent pEvent)
{
	for (int i=0; i<mRetrieveComponents.size(); ++i)
	{
		HIObject pObj = mRetrieveComponents[i];
		if (pObj != nullptr)
		{
			FBComponent *pComp = FBGetFBComponent(pObj);
			if (pComp != nullptr)
				Groups.Add( pComp );
		}
	}

	mSystem.OnUIIdle.Remove( this, (FBCallback) &ORCommand_Group::OnRetrieveFinished );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//


/************************************************
 *	FiLMBOX Creation
 ************************************************/
bool ORCommand_Rewind::FBCreate()
{
	ParentClass::FBCreate();

	Description = "Player Control Rewind";

	FBPropertyPublish( this, RewindMode, "Rewind Mode", nullptr, nullptr );
	FBPropertyPublish( this, ColorMode, "Color Mode", nullptr, nullptr );

	RewindMode = eCommandRewind_Start;
	ColorMode = eCommandColor_White;

	// Allocate any user memory here.
	return true;
}

void ORCommand_Rewind::UpdateReferences()
{
	if (mClip.Ok() )
	{
		// DONE: add properties for shake customization
		FBProperty *prop = mClip->PropertyList.Find( "Rewind Mode" );
		if (prop == nullptr)
		{
			prop = mClip->PropertyCreate( "Rewind Mode", kFBPT_Reference, "reference", false, false, &RewindMode );
		}

		//
		prop = mClip->PropertyList.Find( "Color Mode" );
		if (prop == nullptr)
		{
			prop = mClip->PropertyCreate( "Color Mode", kFBPT_Reference, "reference", false, false, &ColorMode );
		}
	}
}

void ORCommand_Rewind::OnInit()
{
	UpdateReferences();
}

/************************************************
 *	FiLMBOX Destruction.
 ************************************************/
void ORCommand_Rewind::FBDestroy()
{
	// Free any user memory here.
	ParentClass::FBDestroy();
}


/************************************************
 *	Execution callback.
 ************************************************/
void ORCommand_Rewind::ExecuteNotify()
{
	FBTime startTime(StartTime);
	FBTime stopTime(StopTime);
	FBTime localTime(mSystem.LocalTime);
	if (localTime > startTime && localTime > stopTime)
		return;


	FBPlayerControl &pcontrol = FBPlayerControl::TheOne();

	bool isPlaying = pcontrol.IsPlaying;

	switch(RewindMode)
	{
	case eCommandRewind_Start:
		pcontrol.GotoStart();
		break;

	case eCommandRewind_FirstColor:
		{
			FBTime time = FindFirstCommandTime( GetCommandColor(ColorMode) );
			if (time != FBTime::Infinity)
				pcontrol.Goto(time);
		}
		break;
	case eCommandRewind_PrevColor:
		{
			FBTime time = FindPrevCommandTime( localTime, GetCommandColor(ColorMode) );
			if (time != FBTime::Infinity)
				pcontrol.Goto(time);
		}
		break;
	case eCommandRewind_AnyPrevColor:
		{
			FBTime time = FindAnyPrevCommandTime( localTime );
			if (time != FBTime::Infinity)
				pcontrol.Goto(time);
		}
		break;
	}

	
	if (isPlaying)
		pcontrol.Play(true);
}


bool ORCommand_Rewind::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxStore(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAll)
	{
		pFbxObject->FieldWriteBegin( "CommandRewind" );

		pFbxObject->FieldWriteI( RewindMode.AsInt() );
		pFbxObject->FieldWriteI( ColorMode.AsInt() );

		pFbxObject->FieldWriteEnd();
	}
	return true;
}

bool ORCommand_Rewind::FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAll && pFbxObject->FieldReadBegin( "CommandRewind" ) == true )
	{
		RewindMode.SetInt( pFbxObject->FieldReadI() );
		ColorMode.SetInt( pFbxObject->FieldReadI() );

		pFbxObject->FieldReadEnd();
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ORCommand_Color

/************************************************
 *	FiLMBOX Creation
 ************************************************/
static void ORCommand_Color_SetMode(HIObject object, ECommandColorModes value)
{
	ORCommand_Color *pBase = FBCast<ORCommand_Color>(object);
	if (pBase) 
	{
		pBase->SetClipColorMode(value);
		pBase->ColorMode.SetPropertyValue(value);
	}
}

bool ORCommand_Color::FBCreate()
{
	ParentClass::FBCreate();

	Description = "Colored assistent marker";

	FBPropertyPublish( this, ColorMode, "Color Mode", nullptr, ORCommand_Color_SetMode );
	ColorMode = eCommandColor_White;
	mRetreived = false;

	// Allocate any user memory here.
	return true;
}

void ORCommand_Color::SetClipColorMode( const ECommandColorModes mode )
{
	if (mClip.Ok() )
	{
		FBProperty *prop = mClip->PropertyList.Find("ClipColor");
		if (prop)
		{
			FBColor &color = GetCommandColor( (int)mode );
			prop->SetData(color);
		}
	}
}

void ORCommand_Color::UpdateReferences()
{
	if (mClip.Ok() )
	{
		FBProperty *prop = mClip->PropertyList.Find( "Color Mode" );
		if (prop == nullptr)
		{
			prop = mClip->PropertyCreate( "Color Mode", kFBPT_Reference, "reference", false, false, &ColorMode );
			SetClipColorMode( ColorMode );
		}
	}
}

void ORCommand_Color::OnInit()
{
	if (mClip.Ok() )
	{
		if (mRetreived == false)
		{
			SetClipColorMode( ColorMode );
		
			FBTime time(StartTime);
			time = time + FBTime(0,0,0,30);
			StopTime = time;

			FBProperty *prop = mClip->PropertyList.Find("Stop");
			if (prop)
			{
				prop->SetData( &time );
			}
		}

		// Special custom properties

		UpdateReferences();
	}
}


bool ORCommand_Color::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxStore(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAll)
	{
		pFbxObject->FieldWriteBegin( "CommandColor" );

		int mode = ColorMode.AsInt();
		pFbxObject->FieldWriteI( mode );

		pFbxObject->FieldWriteEnd();
	}

	return true;
}
bool ORCommand_Color::FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAll)
	{
		mRetreived = true;

		if (pFbxObject->FieldReadBegin( "CommandColor" ) )
		{
			int mode = pFbxObject->FieldReadI();
			ColorMode.SetInt(mode);

			pFbxObject->FieldReadEnd();
		}
	}

	return true;
}

/************************************************
 *	FiLMBOX Destruction.
 ************************************************/
void ORCommand_Color::FBDestroy()
{
	if (mClip.Ok() )
	{
		

		FBProperty *prop = mClip->PropertyList.Find( "Color Mode" );
		if (prop)
			mClip->PropertyRemove(prop);
	}

	// Free any user memory here.
	ParentClass::FBDestroy();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ORCommand_CameraShake

/************************************************
 *	FiLMBOX Creation
 ************************************************/
bool ORCommand_CameraShake::FBCreate()
{
	ParentClass::FBCreate();

	Description = "Camera Shake Command";

	FBPropertyPublish( this, Amplitude, "Amplitude", nullptr, nullptr );
	FBPropertyPublish( this, Frequency, "Frequency", nullptr, nullptr );
	FBPropertyPublish( this, Duration, "Duration", nullptr, nullptr );

	Amplitude = 100.0;
	Frequency = 10.0;
	Duration = 10.0;

	// Allocate any user memory here.
	return true;
}

void ORCommand_CameraShake::UpdateReferences()
{
	if (mClip.Ok() )
	{
		// DONE: add properties for shake customization

		FBProperty *prop = mClip->PropertyList.Find( "Amplitude" );
		if (prop == nullptr)
		{
			prop = mClip->PropertyCreate( "Amplitude", kFBPT_Reference, "reference", false, false, &Amplitude );
		}

		//
		prop = mClip->PropertyList.Find( "Frequency" );
		if (prop == nullptr)
		{
			prop = mClip->PropertyCreate( "Frequency", kFBPT_Reference, "reference", false, false, &Frequency );
		}

		//
		prop = mClip->PropertyList.Find( "Duration" );
		if (prop == nullptr)
		{
			prop = mClip->PropertyCreate( "Duration", kFBPT_Reference, "reference", false, false, &Duration );
		}
	}
}

void ORCommand_CameraShake::OnInit()
{
	UpdateReferences();
}

void ORCommand_CameraShake::ExecuteNotify()
{
	FBTime startTime(StartTime);
	FBTime stopTime(StopTime);
	FBTime localTime(mSystem.LocalTime);
	if (localTime > startTime && localTime > stopTime)
		return;

	double amplitude = Amplitude;
	double frequency = Frequency;
	double duration = Duration;

	// DONE: use properties
	RunCameraShake(0.1*amplitude, 0.1*frequency, 0.1*duration);
}

/************************************************
 *	FiLMBOX Destruction.
 ************************************************/
void ORCommand_CameraShake::FBDestroy()
{
	// Free any user memory here.
	ParentClass::FBDestroy();
}

bool ORCommand_CameraShake::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxStore(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAll)
	{
		pFbxObject->FieldWriteBegin( "CommandCameraShake" );

		pFbxObject->FieldWriteD( Amplitude );
		pFbxObject->FieldWriteD( Frequency );
		pFbxObject->FieldWriteD( Duration );

		pFbxObject->FieldWriteEnd();
	}
	return true;
}

bool ORCommand_CameraShake::FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAll && pFbxObject->FieldReadBegin( "CommandCameraShake" ) == true )
	{
		Amplitude = pFbxObject->FieldReadD();
		Frequency = pFbxObject->FieldReadD();
		Duration = pFbxObject->FieldReadD();

		pFbxObject->FieldReadEnd();
	}
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ORCommand_CameraZoom

/************************************************
 *	FiLMBOX Creation
 ************************************************/
bool ORCommand_CameraZoom::FBCreate()
{
	ParentClass::FBCreate();

	Description = "Camera Zoom Command";

	FBPropertyPublish( this, Amplitude, "Amplitude", nullptr, nullptr );
	FBPropertyPublish( this, Duration, "Duration", nullptr, nullptr );

	Amplitude = 40.0;
	Duration = 10.0;

	// Allocate any user memory here.
	return true;
}

void ORCommand_CameraZoom::UpdateReferences()
{
	if (mClip.Ok() )
	{
		// DONE: add properties for shake customization
		FBProperty *prop = mClip->PropertyList.Find( "Amplitude" );
		if (prop == nullptr)
		{
			prop = mClip->PropertyCreate( "Amplitude", kFBPT_Reference, "reference", false, false, &Amplitude );
		}

		//
		prop = mClip->PropertyList.Find( "Duration" );
		if (prop == nullptr)
		{
			prop = mClip->PropertyCreate( "Duration", kFBPT_Reference, "reference", false, false, &Duration );
		}
	}
}

void ORCommand_CameraZoom::OnInit()
{
	UpdateReferences();
}

void ORCommand_CameraZoom::ExecuteNotify()
{
	FBTime startTime(StartTime);
	FBTime stopTime(StopTime);
	FBTime localTime(mSystem.LocalTime);
	if (localTime > startTime && localTime > stopTime)
		return;

	double amplitude = Amplitude;
	double duration = Duration;

	// DONE: use properties
	RunCameraZoom(amplitude, 0.1*duration, false);
}

/************************************************
 *	FiLMBOX Destruction.
 ************************************************/
void ORCommand_CameraZoom::FBDestroy()
{
	// Free any user memory here.
	ParentClass::FBDestroy();
}


///////////////////

bool ORCommand_CameraZoom::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxStore(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAll)
	{
		pFbxObject->FieldWriteBegin( "CommandCameraZoom" );

		pFbxObject->FieldWriteD( Amplitude );
		pFbxObject->FieldWriteD( Duration );

		pFbxObject->FieldWriteEnd();
	}
	return true;
}

bool ORCommand_CameraZoom::FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);

	if (pStoreWhat == kAll && pFbxObject->FieldReadBegin( "CommandCameraZoom" ) == true )
	{
		Amplitude = pFbxObject->FieldReadD();
		Duration = pFbxObject->FieldReadD();

		pFbxObject->FieldReadEnd();
	}
	return true;
}
