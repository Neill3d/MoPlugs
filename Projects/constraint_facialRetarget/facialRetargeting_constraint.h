#ifndef __FACIAL_RETARGET_CONSTRAINT_H__
#define __FACIAL_RETARGET_CONSTRAINT_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: facialRetargeting_constraint.h
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

#define ORCONSTRAINTFACIALRETARGET__CLASSNAME		ConstraintFacialRetargeting
#define ORCONSTRAINTFACIALRETARGET__CLASSSTR		"ConstraintFacialRetargeting"

#define MAX_NUMBER_OF_NODES					128

// #define MORE_PARAMETRIC_VALUES			1

enum RigGroups
{
	eGroupMouth,
	eGroupNose,
	eGroupJaw,
	eGroupLeftEye,
	eGroupRightEye
};

enum Mapping 
{
	eMouthLipOuterVeeRight,
	eMouthLipOuterVeeRight_MouthLipOuterCornerRight_033,
	eMouthLipOuterVeeRight_MouthLipOuterCornerRight_066,
	eMouthLipOuterCornerRight,
	eMouthLipOuterCornerRight_MouthLipOuterBottomMiddle_033,
	eMouthLipOuterCornerRight_MouthLipOuterBottomMiddle_066,
	eMouthLipOuterBottomMiddle,
	eMouthLipOuterBottomMiddle_MouthLipOuterCornerLeft_033,
	eMouthLipOuterBottomMiddle_MouthLipOuterCornerLeft_066,
	eMouthLipOuterCornerLeft,
	eMouthLipOuterCornerLeft_MouthLipOuterVeeLeft_033,
	eMouthLipOuterCornerLeft_MouthLipOuterVeeLeft_066,
	eMouthLipOuterVeeLeft,
	eMouthLipOuterTopMiddle,

	eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_066,
	eMouthLipInnerCornerRight,
	eMouthLipInnerCornerRight_MouthLipInnerBottomMiddle_033,
	eMouthLipInnerCornerRight_MouthLipInnerBottomMiddle_066,
	eMouthLipInnerBottomMiddle,
	eMouthLipInnerBottomMiddle_MouthLipInnerCornerLeft_033,
	eMouthLipInnerBottomMiddle_MouthLipInnerCornerLeft_066,
	eMouthLipInnerCornerLeft,
	eMouthLipInnerCornerLeft_MouthLipInnerTopMiddle_033,
	eMouthLipInnerCornerLeft_MouthLipInnerTopMiddle_066,
	eMouthLipInnerTopMiddle,
	eMouthLipInnerTopMiddle_MouthLipInnerCornerRight_033,

	// nose area

	eNostrilOuterRight,
	eNoseLowerMiddle,
	eNostrilOuterLeft,

	// right eye

	eEyeTopRight,
	eEyeCornerOuterRight,
	eEyeBottomRight,
	eEyeCornerInnerRight,

	ePupilOuterRight,
	ePupilInnerRight,

	eBrowInnerRight,
	eBrowInnerRight_BrowMiddleRight_05,
	eBrowMiddleRight,
	eBrowMiddleRight_BrowOuterRight_05,
	eBrowOuterRight,

	// left eye

	eEyeTopLeft,
	eEyeCornerInnerLeft,
	eEyeBottomLeft,
	eEyeCornerOuterLeft,

	ePupilOuterLeft,
	ePupilInnerLeft,
	
	eBrowOuterLeft,
	eBrowMiddleLeft_BrowOuterLeft_05,
	eBrowMiddleLeft,
	eBrowInnerLeft_BrowMiddleLeft_05,
	eBrowInnerLeft,

	eBaseCount,

	eJaw1, // joint for retarget using position
	eJaw2,
	eJaw3,
	eJawRotation, // retarget using rotation

	eStickyLipUpLeft,
	eStickyLipDownLeft,
	eStickyLipUpRight,
	eStickyLipDownRight,

	eEyesLevel,

	/*
	eInnerCheekLeft1,	// drag by nostril outer left
	eInnerCheekLeft2,	// drag by nostril outer left and mouth corner
	eInnerCheekLeft3,	// drag by mouth corner

	eInnerCheekRight1,	// drag by nostril outer left
	eInnerCheekRight2,	// drag by nostril outer left and mouth corner
	eInnerCheekRight3,	// drag by mouth corner

	eEyeSqueezeLeft1,	// drag by nostril outer left
	eEyeSqueezeLeft2,	// drag by nostril outer left and mouth corner
	eEyeSqueezeLeft3,	// drag by mouth corner
	*/
	eMappingCount
};

///////////////////////////////////////////////////////////////////////////////////////
//! A simple constraint class.
class ConstraintFacialRetargeting : public FBConstraint
{
	//--- declaration
	FBConstraintDeclare(ConstraintFacialRetargeting, FBConstraint);

public:

	virtual void ActiveChanged() override;				//!< Notification for Activation Status Change.

	//--- Creation & Destruction
	virtual bool			FBCreate() override;							//!< Constructor.
	virtual void			FBDestroy() override;						//!< Destructor.

	//--- Animation node management
	virtual void			RemoveAllAnimationNodes() override;			//!< Remove animation nodes.
	virtual void			SetupAllAnimationNodes() override;			//!< Setup animation nodes.

	virtual void			SnapSuggested() override;				//!< Suggest 'snap'.
	virtual void			FreezeSuggested		() override;				//!< Suggest 'freeze'.

	//! Real-time evaluation engine function.
	virtual bool			AnimationNodeNotify		( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo ) override;

	virtual bool PlugNotify(FBConnectionAction pAction,FBPlug* pThis,int pIndex,FBPlug* pPlug,FBConnectionType pConnectionType,FBPlug* pNewPlug ) override;

	//--- FBX Interface
	virtual bool			FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Storage.
	virtual bool			FbxRetrieve	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat );	//!< FBX Retrieval.

	//--- Local parameters
	FBAnimationNode*	mSrcTranslation[MAX_NUMBER_OF_NODES];			//!< AnimationNode: INPUT  -> 
	//FBAnimationNode*	mBoneInTranslation[MAX_NUMBER_OF_NODES];
	FBAnimationNode*	mBoneOutTranslation[MAX_NUMBER_OF_NODES];			//!< AnimationNode: OUTPUT -> Constraint (world space result).

	FBAnimationNode		*mLeftEyeOut;
	FBAnimationNode		*mRightEyeOut;

	struct CollisionAnimNodes
	{
		FBAnimationNode		*tr;
		FBAnimationNode		*rot;
		FBAnimationNode		*scl;

		CollisionAnimNodes()
		{
			tr = nullptr;
			rot = nullptr;
			scl = nullptr;
		}
	};

	CollisionAnimNodes		mHeadBackingNodes;
	CollisionAnimNodes		mEyesLevelNodes;

	int					mGroupSource;		//!< Source groupe index - uv coords in camera view
	int					mGroupConstrain;	//!< Constraint group index - face joints

	// mapping

	struct MappingItem
	{
		FBPropertyListObject			SrcObject;
		FBPropertyListObject			DstObject;

		FBPropertyVector3d				PivotPoint;
		
		FBPropertyAnimatableVector3d	Offset;
		FBPropertyAnimatableVector3d	Scale;

		// associate with animation nodes
		//FBAnimationNode		*SrcNode;
		//FBAnimationNode		*DstNode;

		bool				srcInUse;
		bool				dstInUse;

		int					srcRefIndex;
		int					dstRefIndex;

		bool				needComputeOffset;
		FBVector3d			bakedSource;
		FBVector3d			bakedPosition;
		FBTVector			globalPosForOffset;

		bool				status;		// evaluation status
	};
	
	MappingItem						Items[eMappingCount];

	//
	FBPropertyAction				Characterize;
	FBPropertyAction				GrabPositions;

	// save/load xml with source and destination mapping information
	FBPropertyAction				MappingClear;
	FBPropertyAction				MappingLoad;
	FBPropertyAction				MappingSave;

	FBPropertyAction				ZeroEyesOffsets;
	FBPropertyAction				ZeroEyesLookAtOffsets;
	FBPropertyAction				ZeroMouthOffsets;

	// replace symbols (- .) to _ 
	FBPropertyBool					MappingFixNames;

	//

	FBPropertyListObject			CameraMouth;
	FBPropertyListObject			CameraNose;
	FBPropertyListObject			CameraEyeLeft;
	FBPropertyListObject			CameraEyeRight;

	FBPropertyDouble				CameraFOV;
	FBPropertyInt					CameraWidth;
	FBPropertyInt					CameraHeight;
	FBPropertyAction				CameraApply;	// apply camera settings to all assigned cameras
	FBPropertyBool					UseValuesFromOpticalRoot;

	FBPropertyListObject			HeadBacking;
	FBPropertyBool					UseNoseBacking;
	FBPropertyListObject			NoseBacking;
	FBPropertyBool					UseEyesBacking;
	FBPropertyListObject			EyesBacking;
	FBPropertyBool					UseEyesLevelBacking;
	FBPropertyListObject			EyesLevelBacking;
	FBPropertyBool					UseJawBacking;
	FBPropertyListObject			JawBacking;

	// eye lids joints collision with eye balls
	FBPropertyBool					UseLeftEyeCollision;
	FBPropertyListObject			LeftEyeCollision;
	FBPropertyBool					UseRightEyeCollision;
	FBPropertyListObject			RightEyeCollision;

	FBPropertyDouble				CollisionThickness;

	FBPropertyDouble				UnProjectDepth;

	//
	// ADDITIONAL FEATURES

	// keep eye corners static
	FBPropertyBool					StaticEyeCorner;

	// JAW ROTATION
	FBPropertyVector3d				JawUpVector;
	FBPropertyListObject			JawRotationUp;


	// Sticky lips features, needs assigned destination bones

	FBPropertyAnimatableDouble		StickyLips;
	FBPropertyAnimatableDouble		StickyLeftLip;
	FBPropertyAnimatableDouble		StickyRightLip;
	
	// camera stabilization

	FBPropertyBool					Stabilization;

	FBPropertyVector2d				StabilizationGoal;
	FBPropertyVector2d				StabilizationMult;
	FBPropertyVector3d				StabilizationVector;
	FBPropertyDouble				StabilizationError;

	// control local camera x and y movement (in camera plane)
	//  optional - control camera pitch and yaw

	// stabilization based on nose middle marker

	FBPropertyInt					StabilizationIterations;

	// stabilization result - new modelview camera matrix
	
	
	FBMatrix						mStabilizeMatrix;

	// parametric values
	
	struct ParametricProperty3d
	{
		FBPropertyVector3d				Base;
		FBPropertyDouble				Distance;
		FBPropertyVector3d				Dir;	// direction vector on which we calculate distance
		FBPropertyDouble				Low;
		FBPropertyDouble				High;
		FBPropertyAnimatableDouble		Position;	// OUT

		FBAnimationNode					*NodeOut;	// animated node for writing value into
	};
	
	FBPropertyBool						ProcessParametric;
	FBPropertyBool						CalibrateParametric;

	ParametricProperty3d				LeftEyeParam;
	ParametricProperty3d				RightEyeParam;
#ifdef MORE_PARAMETRIC_VALUES
	ParametricProperty3d				LeftNostrilParam;
	ParametricProperty3d				RightNostrilParam;
	ParametricProperty3d				LeftMouthCornerParam;
	ParametricProperty3d				RightMouthCornerParam;
#endif

	FBPropertyAction					ActionAbout;

	void PublishParametricProperty3d( const char *name, ParametricProperty3d &prop, const FBVector3d &dir );
	void SetupParametricAnimationNode( const int index, ParametricProperty3d &prop );
	void ProcessParametricProperty3dOneWay( const Mapping itemId, ParametricProperty3d &prop, FBEvaluateInfo *pinfo );
	void ProcessParametricProperty3dTwoWays( const Mapping itemId, const Mapping itemId2, ParametricProperty3d &prop, FBEvaluateInfo *pinfo, bool calibrate );

	// try to characterize all markers under specified node
	void							AssignSource(FBModel *pModel);
	// characterize all bones under the specified node
	void							AssignDestination(FBModel *pModel);

	void							AssignHeadBacking(FBModel *pModel);
	void							AssignNoseBacking(FBModel *pModel);
	void							AssignEyesBacking(FBModel *pModel);
	void							AssignEyesLevelBacking(FBModel *pModel);
	void							AssignJawBacking(FBModel *pModel);

	void							AssignLeftEyeCollision(FBModel *pModel);
	void							AssignRightEyeCollision(FBModel *pModel);

	bool							AreCamerasMissing();
	void							CheckForOtherCameras(bool userPrompt);

	void							AssignCameraMouth(FBCamera *pCamera);
	void							AssignCameraNose(FBCamera *pCamera);
	void							AssignCameraEyeLeft(FBCamera *pCamera);
	void							AssignCameraEyeRight(FBCamera *pCamera);

	void							DoApplyCameraSettings();

	bool							DoCharacterize(const bool state);

	void							DoGrabPositions();

	void							DoMappingClear();
	void							DoMappingSave();
	void							DoMappingLoad();

	void							DoZeroMouthOffsets();
	void							DoZeroEyesOffsets();
	void							DoZeroEyesLookAtOffsets();

	void							DoSelectCameraMouth();
	void							DoSelectCameraNose();
	void							DoSelectCameraEyeLeft();
	void							DoSelectCameraEyeRight();

	void							DoCalibrateParametric();

	void							ShowBakingGeometry(const bool show=true);
	void							ToggleBakingGeometry();

	// put properties into the folders
	static void AddPropertiesToPropertyViewManager();

	static void GrabPositionsProc(HIObject pObject, bool value);
	static void CameraApplyProc(HIObject pObject, bool value);
	static void MappingClearProc(HIObject pObject, bool value);
	static void MappingSaveProc(HIObject pObject, bool value);
	static void MappingLoadProc(HIObject pObject, bool value);
	static void ZeroEyesOffsetsProc(HIObject pObject, bool value);
	static void ZeroEyesLookAtOffsetsProc(HIObject pObject, bool value);
	static void ZeroMouthOffsetsProc(HIObject pObject, bool value);
	static void CalibrateParametricProc(HIObject pObject, bool value);
	static void CharacterizeProc(HIObject pObject, bool value);
	static void OnActionAboutClick(HIObject pObject, bool value);

protected:
	static FBModel *FindParent(FBModel *pModel);
	static void CollectModels(FBModelList *pList, FBModel *pModel);
	static void	FixMappingName(FBString &name);
	static FBModel *FindModelInList(FBModelList *pList, const char *name, const bool fixMapping);
	static bool UnProjectPoint(const char *debugName, const FBVector3d &inputCoords, const double lDepth, const FBMatrix &invCamMVP, 
		const FBVector3d &cameraPos, FBModel *collisionModel, const FBMatrix &collisionMatrix, FBTVector &result);

	static bool PointCollideWith(FBEvaluateInfo *pInfo, FBModel *pColModel, const FBMatrix &colMatrix, const double thickness, FBVector4d &point);

protected:

	FBSystem		mSystem;

	FBTime			mLastUpdateTime;

	FBMatrix		mHeadBackingMatrix;
	FBMatrix		mEyesLevelMatrix;

	FBStringList	mSrcMapping;
	FBStringList	mDstMapping;

	bool			mProcessJawRotation;
	
	bool			mNeedGrabEyesDistance;

	//

	void ReadModelMatrix(FBEvaluateInfo *pEvaluateInfo, CollisionAnimNodes &nodes, FBMatrix &lResult);

	bool	ProcessAll(FBEvaluateInfo *pEvaluateInfo);

	bool	ProcessEyesLevel(	FBCamera *pCameraForLeftEye,
								FBCamera *pCameraForRightEye,
								FBModel *backingModel,
								const FBMatrix &backingMatrix,
								FBEvaluateInfo *pEvaluateInfo );

	bool	ProcessPositions(	FBCamera *pCamera, 
								FBModel *backingModel,
								const FBMatrix &backingMatrix,
								FBModel *colModel,
								const double colThickness,
								const int startIndex, 
								const int endIndex,
								FBEvaluateInfo* pEvaluateInfo);

	bool	ProcessJawAim(	FBCamera *pCamera, 
							FBModel *backingModel,
							const FBMatrix &backingMatrix,
							FBEvaluateInfo* pEvaluateInfo);

	bool	ProcessStickyLips(FBEvaluateInfo* pEvaluateInfo);

	bool	StabilizePositions( FBCamera *pCamera, FBModel *pBakingMesh, const FBMatrix &backingMatrix,
						const int stabIndex,
						const double stabSourceU, const double stabSourceV,
						FBEvaluateInfo *pEvaluateInfo );

	void	ShowAbout();
};

#endif	/* __MOUTH_RETARGET_CONSTRAINT_H__ */
