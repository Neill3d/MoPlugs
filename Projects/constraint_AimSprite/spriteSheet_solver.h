
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: spriteSheet_solver.h
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
#include <fbsdk/fbapplymanagerrule.h>
#include <vector>
#include <map>
#include <random>

#include "spriteSheet_properties.h"

//--- Registration define
#define SPRITESOLVERASSOCIATION__CLASSNAME	    KSpriteSolverAssociation 
#define SPRITESOLVERASSOCIATION__CLASSSTR		"KSpriteSolverAssociation"

#define SPRITESHEETSOLVER__CLASSNAME			SolverSpriteSheet
#define SPRITESHEETSOLVER__CLASSSTR				"SolverSpriteSheet"





/*
enum ESpriteOrder
{
	eSpriteVector,
	eSpriteMatrix,
	eSpriteBlendedMatrix
};

const char *FBPropertyBaseEnum<ESpriteOrder>::mStrings[] = {
	"Vector",
	"Matrix",
	"Blended Matrix",
	nullptr
};
*/


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
/**	KAMRManipulatorAssociation class.
*	Provides custom context menu rules for ORManip_Association and ORModelItem.
*/
class KSpriteSolverAssociation : public FBApplyManagerRule
{
	FBApplyManagerRuleDeclare(KSpriteSolverAssociation, FBApplyManagerRule);

public:


	/** Return true if pSrc is the type of custom object which this rule to be defined for. 
        *
        *    @warning you should only return true if pSrc is the type of custom object you defined.
        */
    virtual bool IsValidSrc( FBComponent* /*pSrc*/) override;

    /** Return true if pDst is the type of custom or MB object which your custom object 
    *    pSrc will be applied on. it's possible to return proper destination and return it by pDst. 
    *    For example, pSrc is RagDoll Property, pDst is a one of IK/FK bone, then you can find
    *    associated Character and replace.
    *
    *    \param    pSrc    the custom type of object you drag it to attach another object.
    *    \retval    pDst    one of the selected destinations, it could be modified and return proper destination.
    *    \retval    pAllowMultiple    return true if allow multiple objects of same type as pSrc to be connected
    *                   to pDst.
    *    \return            return true if connection between pSrc and pDst is allowed.
    */
    virtual bool IsValidConnection( FBComponent* pSrc, FBComponent*& pDst, bool& pAllowMultiple) override;

	/** Build context menu for KAMRManipulatorAssociation.
	*
	*	\param	pAMMenu	the menu to add menu option on.
	*	\param	pFocusedObject	the ORModelItem type of object you right-click on.
	*	\return	return true if menu item has been added.
	*/
	virtual bool MenuBuild( FBAMMenu* pAMMenu, FBComponent* pFocusedObject) override;

	/** Perform action if the added menu item is clicked.
	*
	*	\param	pMenuId	the menu ID right-click on.
	*	\param	pFocusedObject	the ORModelItem type of object you right-click on.
	*	\return	return true if action has been performed.
	*/
	virtual bool MenuAction( int pMenuId, FBComponent* pFocusedObject) override;

protected:
	int mAddTextureProps;
	int mAddModelProps;
	int mAddModelEvaluation;
	int mRemoveModelEvaluation;
	
	FBSystem mSystem;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//! A sprite sheet solver class.
class SolverSpriteSheet : public FBConstraintSolver
{
	//--- declaration
	FBClassDeclare( SolverSpriteSheet, FBConstraintSolver );
public: 
	SolverSpriteSheet(const char* pName)
		: FBConstraintSolver(pName)
		, e2(rd())
		, dist(0, 1.0)
	{ 
		FBClassInit; 
	}
  
public:

	FBPropertyBool			LocalPlay;
	// ?! SpriteProperties, SpriteTextureProperties
	FBPropertyInt			SpriteNodes;		// output number of computed nodes in that solver
	FBPropertyTime			ResetTime;
	FBPropertyAction		ManualReset;
	FBPropertyBool			ZeroNonPlaying;	// zero scaling for non playing sprite textures
	FBPropertyBool			UseLocalCache;
	FBPropertyDouble		TriggerOnScalingThreshold;

	void	OnManualResetClick();

	static void ActionManualReset(HIObject pObject, bool value);

public:
	//--- Creation & Destruction
	virtual bool			FBCreate() override;							//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.
	/*
	//--- Animation node management
	virtual void			RemoveAllAnimationNodes();			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes();			//!< Setup animation nodes.

	virtual void			SnapSuggested() override;
	virtual void			FreezeSuggested		() override;				//!< Suggest 'freeze'.
	*/

	virtual bool FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;
	virtual bool FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;

	// hack to run a solver after scene load
	void	OnSystemIdle(HISender pSender, HKEvent pEvent);
	void	OnSystemIdle2(HISender pSender, HKEvent pEvent);

	virtual void ActiveChanged() override;				//!< Notification for Activation Status Change.

	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo );

public:

	static void AddTextureProperties(FBTexture *pTexture);
	static void AddModelProperties(FBModel *pModel);

protected:

	struct SpriteSheetNode
	{
		// texture input nodes
		FBProperty			*inType;	// 0-basic (use rows, cols for compute number of items, each item position)
										// 1 - xml based (use data inside a file to define number of items and positions)
		FBProperty			*inRows; // number of rows
		FBProperty			*inCols;
		FBProperty			*inXmlData;	// path to xml data file

		// model input nodes
		
		FBProperty			*inRangeCount;		// indicate number of regions (default - 3, or take value from xml)

		struct FrameRange
		{
			FBProperty			*inStartFrame;			// range start
			FBProperty			*inStopFrame;			// range stop

		} inFrameRange[MAX_NUMBER_OF_FRAME_RANGE];

		FBProperty			*inFPS;
		FBProperty			*inLocalPlay;
		FBProperty			*inPlayOffset;
		FBProperty			*inLoopPlay;
		FBProperty			*inSpeed;

		// triggers
		FBProperty			*inUseTrigger;
		FBAnimationNode		*inTrigger;
		FBProperty			*inTriggerOnScaling;
		FBProperty			*inTriggerDelay;

		FBProperty			*inTriggerChangeMode;		// mode when we change rangeIndex from the trigger value (1 set range to 0, 2 set range to 1, etc.)
		FBProperty			*inTriggerIndex;			// use specified range for triggering
		FBProperty			*inTriggerRandomMode;		// randomly choose a sequence range

		// write output nodes
		FBAnimationNode		*modelScaling;	// used for compute trigger on scaling

		FBProperty			*inIndex;

		FBAnimationNode		*outTranslation;
		FBAnimationNode		*outScaling;
		FBAnimationNode		*outVisibility;

		// some add-on info
		bool				needReset;
		double				lastFrame;
		double				lastTrigger;
		double				lastModelScaling;
		FBTime				lastTime;
		FBTime				triggerTime;	// if time is not MinusInfinity, then set trigger on time happen

		FBTime				offsetTime;		// in local mode, we assign offset from reset time(action) and each time trigger happent

		// TODO: pre-cache local frame value
		FBTime				cachedFrame;
		double				cachedPos[3];
		double				cachedScl[3];

		// for debug purpose
		FBTexture			*debugTexture;

		int					index;
	};

	// values are in range [0; 1], and w,h are scaling values
	struct SpriteFrame
	{
		double x;
		double y;
		double w;
		double h;
	};

	// read info from xml file or calculate in manualy by help of number of cols and rows
	struct SpriteSheetInfo
	{
		std::vector<SpriteFrame>		frames;
		std::vector<int>				ranges;	// index of a first frame for each atlas sequence range
	};

	void EmptyNode( SpriteSheetNode &node );
	void ResetNode( SpriteSheetNode &node );
	void ComputeLocalOffsetTime( SpriteSheetNode &node );

	static void prepProp (FBComponent *pComp, const char *propName, double defValue, double minValue, double maxValue, FBProperty *& outNode, bool typeBool);
	static void prepAnimProp (FBComponent *pComp, const char *propName, double defValue, double minValue, double maxValue, FBAnimationNode *& outNode) ;

	static void AddTextureProperties(FBTexture *pTexture, SpriteSheetNode &node);
	static void AddModelProperties(FBModel *pModel, const int rangeCount, SpriteSheetNode &node);

	FBAnimationNode						*mDummyNode;
	FBSystem							mSystem;
	std::vector<SpriteSheetNode>		mNodes;
	
	int									mLastFrame;

	// pre-cache frame information for every sprite texture that used for solver
	std::map<FBTexture*, SpriteSheetInfo*>	mSpriteInfoMap;

	std::map<FBAnimationNode*, SpriteSheetNode*>	mNodeToSpriteInfoMap;

	//
	std::random_device					rd;
	std::mt19937						e2;		// engine
	std::uniform_real_distribution<>	dist;	// distribution

	//
	//
	void ClearNodes();
	void ClearSpriteInfoMap();

	void ReadSpriteSheetInfo(  FBTexture *spriteTexture, SpriteSheetInfo *&newInfo );
	bool ComputeBasicTextureInformation( const int rows, const int cols, SpriteSheetInfo *outInfo );
	bool ReadXmlTextureInformation( const char *filename, SpriteSheetInfo *outInfo );

	bool IsNewRangeStarted( const std::string &prevName, const std::string &newName );

	// we start animation from the beginning in case
	//	1 - first run
	//	2 - func execution with big delay > time limit (1 second)
	//	3 - inTrigger > 0.0 and inTrigger != lastTrigger

	bool ComputeSpriteSheetNode( FBEvaluateInfo* pEvaluateInfo, SpriteSheetNode *node,
		const bool localPlay, const bool useLocalCache, const bool zeroNonPlaying, FBTime &resetTime,
		const double triggerOnScalingThreshold,
		double *outScl, double *outPos);

	bool ComputeTextureMatrix(	double *outTranslation, 
								double *outScaling, 
								FBTime &lastTime,
								FBTime &localOffsetTime,
								SpriteSheetNode &node,
								double		&lastFrame,
								double		&lastTrigger,
								double		&vis,
								const FBTime &systemTime,
								const SpriteSheetInfo *pSpriteInfo,
								const double inFPS,
								const bool	localPlay,
								const double playOffset,
								const double inStartFrame,
								const double inStopFrame,
								const double inTrigger,
								const bool inLoopPlay,
								const double inSpeed );


};