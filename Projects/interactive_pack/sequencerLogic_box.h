#ifndef __ORBOX_SEQ_LOGIC_BOX_H__
#define __ORBOX_SEQ_LOGIC_BOX_H__

/**	\file	orboxtransport_box.h
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

#include <vector>

//--- Registration defines
#define	ORBOXLOGIC__CLASSNAME		ORBoxImageSequencerLogic
#define ORBOXLOGIC__CLASSSTR		"ORBoxImageSequencerLogic"

/**	Template for FBBox class.
*/
class ORBoxImageSequencerLogic : public FBBox
{
	//--- box declaration.
	FBBoxDeclare( ORBoxImageSequencerLogic, FBBox );

public:
	//! creation function.
	virtual bool FBCreate();

	//! destruction function.
	virtual void FBDestroy();

	//! Overloaded FBBox real-time evaluation function.
	virtual bool AnimationNodeNotify(FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo);

	//! FBX Storage function
	virtual bool FbxStore( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	//! FBX Retrieval functionc
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );

	FBPropertyAction			Update;
	
	FBPropertyAction			RemoveAll;	// remove all sequence ranges
	FBPropertyAction			Add;		// add a new sequence range (exloop, exloopstart, exloopstop)

	FBPropertyInt				ExCount;	// return number of parameter in stored list
	FBPropertyInt				ExItem;		// choose item to return next parameters
	FBPropertyBool				ExLoop;
	FBPropertyInt				ExLoopStart;
	FBPropertyInt				ExLoopStop;

	void		DoUpdate();
	void		DoRemoveAll();
	void		DoAdd();

	const int GetCount() const
	{
		return (int) sequences.size();
	}

	const int GetCurrent() const
	{
		return currSeq;
	}

	void SetCurrent(const int value)
	{
		currSeq = value;
	}

	const bool GetLoop() const
	{
		if (currSeq >= 0 && currSeq < sequences.size() )
			return sequences[currSeq].loop;
		return tempseq.loop;
	}

	void SetLoop(const bool value)
	{
		tempseq.loop = value;
	}

	const int GetLoopStart() const
	{
		if (currSeq >= 0 && currSeq < sequences.size() )
			return sequences[currSeq].start;
		return tempseq.start;
	}

	void SetLoopStart(const int value)
	{
		tempseq.start = value;
	}

	const int GetLoopStop() const
	{
		if (currSeq >= 0 && currSeq < sequences.size() )
			return sequences[currSeq].stop;
		return tempseq.stop;
	}

	void SetLoopStop(const int value)
	{
		tempseq.stop = value;
	}

private:

	FBPlayerControl		mPlayerControl;

	FBAnimationNode		*mOutput;
	FBAnimationNode		*mLoopStart;		//!< out
	FBAnimationNode		*mLoopStop;			//!< out
	FBAnimationNode		*mLoop;				//!< out

	FBAnimationNode		*mInput;
	FBAnimationNode		*mTrigEnd;			//!< in

	struct	ImageSeq
	{
		bool		loop;
		int			start;
		int			stop;

		//
		ImageSeq()
		{
			loop = false;
			start = 0;
			stop = 1;
		}
		//
		ImageSeq(const bool _loop, const int _start, const int _stop)
			: loop(_loop)
			, start(_start)
			, stop(_stop)
		{}
	};

	bool						mInputActive;

	int							currSeq;
	std::vector<ImageSeq>		sequences;

	ImageSeq					tempseq;

	void				GoToNextSeq();
};

#endif /* __ORBOXTRANSPORT_BOX_H__ */
