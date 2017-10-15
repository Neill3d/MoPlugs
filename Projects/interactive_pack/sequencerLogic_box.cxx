
/**	\file	orboxtransport_box.cxx
*/

//--- Class declaration
#include "box_sequencerLogic_box.h"

//--- Registration defines
#define ORBOXLOGIC__CLASS		ORBOXLOGIC__CLASSNAME
#define ORBOXLOGIC__NAME		ORBOXLOGIC__CLASSSTR
#define	ORBOXLOGIC__LOCATION	"Neill3d"
#define ORBOXLOGIC__LABEL		"Image Seq Logic"
#define	ORBOXLOGIC__DESC		"Image Sequencer Logic Box"

//--- TRANSPORT implementation and registration
FBBoxImplementation	(	ORBOXLOGIC__CLASS	);	// Box class name
FBRegisterBox		(	ORBOXLOGIC__NAME,		// Unique name to register box.
						ORBOXLOGIC__CLASS,		// Box class name
						ORBOXLOGIC__LOCATION,	// Box location ('plugins')
						ORBOXLOGIC__LABEL,		// Box label (name of box to display)
						ORBOXLOGIC__DESC,		// Box long description.
						FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)


static void Logic_Update(HIObject object, bool value)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		if (value)
		{
			pItem->DoUpdate();
		}
	}
}

static void Logic_RemoveAll(HIObject object, bool value)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		if (value)
		{
			pItem->DoRemoveAll();
		}
	}
}

static void Logic_Add(HIObject object, bool value)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		if (value)
		{
			pItem->DoAdd();
		}
	}
}

static int Logic_GetCount(HIObject object)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		return pItem->GetCount();
	}
	return 0;
}

static int Logic_GetCurrent(HIObject object)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		return pItem->GetCurrent();
	}
	return -1;
}

static void Logic_SetCurrent(HIObject object, int value)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		pItem->SetCurrent(value);	
	}
}

static bool Logic_GetLoop(HIObject object)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		return pItem->GetLoop();
	}
	return false;
}

static void Logic_SetLoop(HIObject object, bool value)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		pItem->SetLoop(value);	
	}
}

static int Logic_GetLoopStart(HIObject object)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		return pItem->GetLoopStart();
	}
	return 0;
}

static void Logic_SetLoopStart(HIObject object, int value)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		pItem->SetLoopStart(value);	
	}
}

static int Logic_GetLoopStop(HIObject object)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		return pItem->GetLoopStop();
	}
	return 0;
}

static void Logic_SetLoopStop(HIObject object, int value)
{
	ORBoxImageSequencerLogic *pItem = FBCast<ORBoxImageSequencerLogic>(object);
	if (pItem) 
	{
		pItem->SetLoopStop(value);	
	}
}

/************************************************
 *	TRANSPORT Creation
 ************************************************/
bool ORBoxImageSequencerLogic::FBCreate()
{
	/*
	*	Create the nodes for the box.
	*/
	mInput = AnimationNodeInCreate	( 0, "Input",		ANIMATIONNODE_TYPE_NUMBER );
	mTrigEnd = AnimationNodeInCreate	( 1, "Trig End",		ANIMATIONNODE_TYPE_BOOL );
	
	mOutput = AnimationNodeOutCreate ( 2, "Output",			ANIMATIONNODE_TYPE_NUMBER );
	mLoop = AnimationNodeOutCreate	( 3, "Loop",		ANIMATIONNODE_TYPE_BOOL );
	mLoopStart = AnimationNodeOutCreate	( 4, "Loop Start",		ANIMATIONNODE_TYPE_INTEGER );
	mLoopStop = AnimationNodeOutCreate	( 5, "Loop Stop",		ANIMATIONNODE_TYPE_INTEGER );
	
	FBPropertyPublish( this, Update, "Update", nullptr, Logic_Update );

	FBPropertyPublish( this, RemoveAll, "Remove All", nullptr, Logic_RemoveAll );
	FBPropertyPublish( this, Add, "Add", nullptr, Logic_Add );

	FBPropertyPublish( this, ExCount, "ExCount", Logic_GetCount, nullptr );
	FBPropertyPublish( this, ExItem, "ExItem", Logic_GetCurrent, Logic_SetCurrent );
	FBPropertyPublish( this, ExLoop, "ExLoop", Logic_GetLoop, Logic_SetLoop );
	FBPropertyPublish( this, ExLoopStart, "ExLoopStart", Logic_GetLoopStart, Logic_SetLoopStart );
	FBPropertyPublish( this, ExLoopStop, "ExLoopStop", Logic_GetLoopStop, Logic_SetLoopStop );

	currSeq = -1;
	DoUpdate();

	mInputActive = false;

	return true;
}


/************************************************
 *	TRANSPORT Destruction.
 ************************************************/
void ORBoxImageSequencerLogic::FBDestroy()
{
	/*
	*	Free any user memory associated to box.
	*/
}

void ORBoxImageSequencerLogic::DoUpdate()
{
	currSeq = -1;
	//sequences.resize(2);

	//sequences[0] = ImageSeq( false, 100, 200 );
	//sequences[1] = ImageSeq( false, 250, 350 );
}

void ORBoxImageSequencerLogic::DoRemoveAll()
{
	currSeq = -1;
	sequences.clear();
}

void ORBoxImageSequencerLogic::DoAdd()
{
	sequences.push_back( tempseq );
}

void ORBoxImageSequencerLogic::GoToNextSeq()
{
	currSeq += 1;
	if (currSeq >= sequences.size() )
		currSeq = 0;
}

/************************************************
 *	Real-time engine evaluation
 ************************************************/
bool ORBoxImageSequencerLogic::AnimationNodeNotify( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo )
{
	/*
	*	1. Read the data from the in connector
	*	2. Treat the data as required
	*	3. Write the data to the out connector
	*	4. Return the status (LIVE/DEAD) of the box.
	*/
	double lValue, lTrigEnd;
	bool lStatus, lStatus2;

	bool result = false;

	
	double output = 0.0;
	double loop = 0.0;
	double start = 0.0;
	double stop = 1.0;

	//
	lStatus = mInput->ReadData( &lValue, pEvaluateInfo );
	if( lStatus )
	{
		
		if (lValue == 0.0)
		{
			mInputActive = false;
		}
		if (lValue > 0.0 && mInputActive == false)
		{
			mInputActive = true;
			output = 1.0;

			GoToNextSeq();
		}
		
		result = true;
	}

	lStatus2 = mTrigEnd->ReadData( &lTrigEnd, pEvaluateInfo );
	if ( lStatus2 ) 
	{
		if (lTrigEnd > 0.0)
			output = 1.0;
	}

	if (currSeq >= 0 && currSeq < sequences.size() )
	{
		loop = sequences[currSeq].loop;
		start = sequences[currSeq].start;
		stop = sequences[currSeq].stop;
	}

	mLoop->WriteData( &loop, pEvaluateInfo );
	mLoopStart->WriteData( &start, pEvaluateInfo );
	mLoopStop->WriteData( &stop, pEvaluateInfo );
	mOutput->WriteData( &output, pEvaluateInfo );

	return result;
}


/************************************************
 *	FBX Storage.
 ************************************************/
bool ORBoxImageSequencerLogic::FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Store box parameters.
	*/

	if (pStoreWhat == kAttributes)
	{
		pFbxObject->FieldWriteBegin( "imageseqlogic" );
		
		pFbxObject->FieldWriteI( (int) sequences.size() );

		//char sztemp[128];
		//memset(sztemp, 0, sizeof(char) * 128);

		for (int i=0; i< (int)sequences.size(); ++i)
		{
			//sprintf_s( sztemp, sizeof(char)*128, "seqloop_%d", i );
			pFbxObject->FieldWriteI( (sequences[i].loop) ? 1 : 0 );
			
			//sprintf_s( sztemp, sizeof(char)*128, "seqloopstart_%d", i );
			pFbxObject->FieldWriteI( sequences[i].start );
			
			//sprintf_s( sztemp, sizeof(char)*128, "seqloopstop_%d", i );
			pFbxObject->FieldWriteI( sequences[i].stop );
		}
		pFbxObject->FieldWriteEnd();
	}

	return true;
}


/************************************************
 *	FBX Retrieval.
 ************************************************/
bool ORBoxImageSequencerLogic::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	/*
	*	Retrieve box parameters.
	*/

	if (pStoreWhat == kAttributes)
	{

		if (pFbxObject->FieldReadBegin( "imageseqlogic" ) )
		{

			int count = pFbxObject->FieldReadI( );

			sequences.resize(count);

			//char sztemp[128];
			//memset(sztemp, 0, sizeof(char) * 128);

			int value; 

			for (int i=0; i< (int)sequences.size(); ++i)
			{
				//sprintf_s( sztemp, sizeof(char)*128, "seqloop_%d", i );
				value = pFbxObject->FieldReadI( );
				sequences[i].loop = (value > 0);

				//sprintf_s( sztemp, sizeof(char)*128, "seqloopstart_%d", i );
				value = pFbxObject->FieldReadI( );
				sequences[i].start = value;

				//sprintf_s( sztemp, sizeof(char)*128, "seqloopstop_%d", i );
				value = pFbxObject->FieldReadI( );
				sequences[i].stop = value;
			}

			pFbxObject->FieldReadEnd();
		}
	}

	return true;
}
