
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: moPhysics_solver.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "moPhysics_solver.h"
#include "library_NewtonPhysics\newton_PUBLIC.h"
#include "queryFBGeometry.h"

#include "moPhysics_CarProperties.h"
#include "moPhysics_ChainProperties.h"

//--- FiLMBOX implementation and registration

FBConstraintSolverImplementation( MOPhysicsSolver );
FBRegisterConstraintSolver( "MoPhysicsSolver",
							MOPhysicsSolver,
							"MoPhysics Solver",
							"MoPhysics Solver",
							"character_solver.png" );


FBConstraintLayoutImplementation( MOPhysicsSolverLayout );
FBRegisterConstraintLayout( MOPhysicsSolverLayout,
							"MoPhysicsSolver",
							FB_DEFAULT_SDK_ICON );

#ifdef ORSDK2013
	MOPhysicsSolver *gGlobalSolver = nullptr;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// MO Physics

void MOPhysicsSolver::ActionRebuildCollisions( HIObject pObject, bool value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
    
	// update wheel information
	if (lDevice && value) {
		lDevice->DoRebuildCollisions();
	}
}

void MOPhysicsSolver::ActionResetToStart( HIObject pObject, bool value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
    
	// update wheel information
	if (lDevice && value) {
		lDevice->RestoreState();
	}
}

void MOPhysicsSolver::ActionSaveState( HIObject pObject, bool value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
    
	// update wheel information
	if (lDevice && value) {
		lDevice->SaveState();
	}
}

void MOPhysicsSolver::ActionSerialize( HIObject pObject, bool value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
    
	// update wheel information
	if (lDevice && value) {
		lDevice->DoSerialize();
	}
}

bool MOPhysicsSolver::GetLiveMode( HIObject pObject )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
    
	// update wheel information
	if (lDevice) {
		return lDevice->Live;
	}
	return false;
}

void MOPhysicsSolver::SetLiveMode( HIObject pObject, bool value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
    
	// update wheel information
	if (lDevice) {
		lDevice->Live = value;
	}
}

bool MOPhysicsSolver::GetRecording( HIObject pObject )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
    
	// update wheel information
	if (lDevice) {
		return lDevice->RecordMode;
	}
	return false;
}

void MOPhysicsSolver::SetRecording( HIObject pObject, bool value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
    
	// update wheel information
	if (lDevice) {
		lDevice->RecordMode = value;
	}
}

void MOPhysicsSolver::SetWorldScale( HIObject pObject, double value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
    
	// update wheel information
	if (lDevice) {
		lDevice->WorldScale.SetPropertyValue(value);
		lDevice->DoRestart();
	}
}

void MOPhysicsSolver::SetDefaultSoftness( HIObject pObject, double value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
	if (lDevice) {
		lDevice->DefaultSoftness.SetPropertyValue(value);
		lDevice->DoUpdateDefaultMaterialParams();
	}
}
void MOPhysicsSolver::SetDefaultElasticity( HIObject pObject, double value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
	if (lDevice) {
		lDevice->DefaultElasticity.SetPropertyValue(value);
		lDevice->DoUpdateDefaultMaterialParams();
	}
}
void MOPhysicsSolver::SetDefaultCollidable( HIObject pObject, double value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
	if (lDevice) {
		lDevice->DefaultCollidable.SetPropertyValue(value);
		lDevice->DoUpdateDefaultMaterialParams();
	}
}
void MOPhysicsSolver::SetDefaultStaticFriction( HIObject pObject, double value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
	if (lDevice) {
		lDevice->DefaultStaticFriction.SetPropertyValue(value);
		lDevice->DoUpdateDefaultMaterialParams();
	}
}
void MOPhysicsSolver::SetDefaultDynamicFriction( HIObject pObject, double value )
{     
    MOPhysicsSolver* lDevice = FBCast<MOPhysicsSolver>(pObject);
	if (lDevice) {
		lDevice->DefaultDynamicFriction.SetPropertyValue(value);
		lDevice->DoUpdateDefaultMaterialParams();
	}
}

//--- Creation & Destruction
bool MOPhysicsSolver::FBCreate()
{
	Active = false;
	Live = false;
	RecordMode = false;
	SyncAnimationPlay = false;
	SampleRecordingPrecision = 100;

	FBPropertyPublish(this, LiveState, "Live State", GetLiveMode, SetLiveMode);
	FBPropertyPublish(this, RecordState, "Record State", GetRecording, SetRecording);

	FBPropertyPublish(this, PhysicsThreads, "Physics Threads", nullptr, nullptr);
	FBPropertyPublish(this, PhysicsSamples, "Physics Samples", nullptr, nullptr);
	FBPropertyPublish(this, PhysicsFPS, "Physics FPS", nullptr, nullptr);

	FBPropertyPublish(this, ResetToStart, "Reset To Start", nullptr, ActionResetToStart );
	FBPropertyPublish(this, SetStartState, "Set Start State", nullptr, ActionSaveState );
	FBPropertyPublish(this, Serialize, "Serialize", nullptr, ActionSerialize );

	FBPropertyPublish(this, PhysicsEngine, "Physics Engine", nullptr, nullptr);

	FBPropertyPublish(this, ResetFrame, "Reset Frame", nullptr, nullptr);

	FBPropertyPublish(this, Gravity, "Gravity", nullptr, nullptr);
	FBPropertyPublish(this, AutoScale, "Auto Scale", nullptr, nullptr);
	FBPropertyPublish(this, WorldScale, "World Scale", nullptr, nullptr);

	FBPropertyPublish(this, StaticCollisions, "Static Collisions", nullptr, nullptr);
	FBPropertyPublish(this, DefaultSoftness, "Default Softness", nullptr, SetDefaultSoftness);
	FBPropertyPublish(this, DefaultElasticity, "Default Elasticity", nullptr, SetDefaultElasticity);
	FBPropertyPublish(this, DefaultCollidable, "Default Collidable", nullptr, SetDefaultCollidable);
	FBPropertyPublish(this, DefaultStaticFriction, "Default Static Friction", nullptr, SetDefaultStaticFriction);
	FBPropertyPublish(this, DefaultDynamicFriction, "Default Dynamic Friction", nullptr, SetDefaultDynamicFriction);

	FBPropertyPublish(this, DisplayDebug, "Display Debug", nullptr, nullptr);

	PhysicsThreads = 1;
	PhysicsSamples = 1024;
	PhysicsFPS = 120.0;

	StaticCollisions.SetFilter( FBModel::GetInternalClassId() );

	DefaultSoftness = 15.0;
	DefaultElasticity = 30.0;
	DefaultCollidable = 100.0;
	DefaultStaticFriction = 60.0;
	DefaultDynamicFriction = 50.0;

	PhysicsEngine = ePhysicsNewton;

	ResetFrame = 0;

	LiveState.SetEnable(false);
	RecordState.SetEnable(false);
	LiveState = false;
	RecordState = false;

	Gravity = FBVector3d(0.0, -9.8, 0.0);
	AutoScale = true;
	WorldScale = 10.0;

	WorldIsReady = false;
	DisplayDebug = false;
	mNeedRebuild = true;

	MultiThreaded(false);

	mLastTimeLocal = true;
	mLastIsStop = true;
	mLastIsRecording = false;

	mAnimTimeSecs = 0.0;

	mPhysTimeSecs = 0.0;
	mLastPhysTimeSecs = 0.0;

	mLastTorque = 0.0;

	return true;
}

void MOPhysicsSolver::FBDestroy()
{
}

void MOPhysicsSolver::ActiveChanged()
{
	if (Active)
	{
		WorldIsReady = false;

		// goes online - prepare physics world
		mWorldScaling = 0.01 * WorldScale;
		mHardware.reset( CreateNewNewtonWorld(mWorldScaling, PhysicsThreads, PhysicsSamples, (float) PhysicsFPS) );

		EnterOnline();

		mHardware->Open();

		// update gravity value
		double value[3];
		Gravity.GetData( value, sizeof(double)*3 );
		mHardware->SetGravity( &value[0] );
#ifdef ORSDK2013
		gGlobalSolver = this;
		FBEvaluateManager::TheOne().RegisterEvaluationGlobalFunction( MOPhysicsSolver::OnPerFrameEvaluationPipelineCallback, kFBGlobalEvalCallbackBeforeDAG );
#else
		//FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Add(this, (FBCallback)&MOPhysicsSolver::OnPerFrameEvaluationPipelineCallback);
#endif

		WorldIsReady = true;
	}
	else
	{
		WorldIsReady = false;

#ifdef ORSDK2013
		FBEvaluateManager::TheOne().UnregisterEvaluationGlobalFunction( MOPhysicsSolver::OnPerFrameEvaluationPipelineCallback, kFBGlobalEvalCallbackBeforeDAG );
		gGlobalSolver = nullptr;
#else
		//FBEvaluateManager::TheOne().OnEvaluationPipelineEvent.Remove(this, (FBCallback)&MOPhysicsSolver::OnPerFrameEvaluationPipelineCallback);
#endif
		// goes offline - free physics world
		LeaveOnline();
		
		if (mHardware.get() )
		{
			mHardware->Close();
			mHardware->Clear();
		
			//mHardware.reset(nullptr);
		}
	}
}

//--- Animation node management
void MOPhysicsSolver::RemoveAllAnimationNodes()
{
}

void MOPhysicsSolver::SetupAllAnimationNodes()
{
}

void MOPhysicsSolver::SnapSuggested()
{
}

void MOPhysicsSolver::FreezeSuggested ()
{
}

//--- Constraint Status interface
bool MOPhysicsSolver::Disable ( FBModel* pModel )
{
	return true;
}

#ifdef ORSDK2013

// TODO: connect to mobu time instead of internal newton timer
void MOPhysicsSolver::DoEvaluate()
{
	if (Active)
	{
		if (Live || (RecordMode && mPlayerControl.IsPlaying) )
		{
			if (mHardware.get() != nullptr)
				mHardware->FetchDataPacket(0.01);
		}
	}
}

void MOPhysicsSolver::OnPerFrameEvaluationPipelineCallback(FBEvaluateInfo* pEvaluteInfo)
{
	if (gGlobalSolver)
	{
		gGlobalSolver->DoEvaluate();
	}
}

#else

void MOPhysicsSolver::OnPerFrameEvaluationPipelineCallback(HISender pSender, HKEvent pEvent)
{
	FBEventEvalGlobalCallback lFBEvent(pEvent);
    switch(lFBEvent.GetTiming())
    {
    case kFBGlobalEvalCallbackBeforeDAG:
		{
        //
        // We could add custom tasks here before MoBu perform standard evaluation tasks. 
        //

		if (Active)
		{
			/*
			if (Live || (RecordMode && mPlayerControl.IsPlaying) )
			{
				if (mHardware.get() != nullptr)
					mHardware->FetchDataPacket(0.01);
			}
			*/
			if (Live)
			{
				if (mHardware.get() != nullptr)
					mHardware->FetchDataPacket(0.01);
			}
		}
		} break;
    case kFBGlobalEvalCallbackAfterDAG:
        //
        // We could add custom tasks here after MoBu finish standard animation & deformation evaluation tasks. 
        //

        break;
    case kFBGlobalEvalCallbackAfterDeform:
        //
        // We could add custom tasks here after MoBu finish standard deformation evaluation tasks 
        // (if not use GPU deformation). E.g, update the accelerated spatial scene structure for rendering. 
        //

        break;
    default:
        break;
    }   
}

#endif

bool MOPhysicsSolver::WritePhysNodeData(PhysNode &node, bool writedata, FBEvaluateInfo* pEvaluateInfo)
{
	if (node.body == nullptr || node.tr == nullptr || node.rot == nullptr)
		return false;

	FBMatrix m;
	FBTVector T;
	FBRVector R;
	FBSVector S;

	m.Set( node.body->GetMatrix( GetWorldPtr()) );
	FBMatrixToTRS( T, R, S, m );

	if (writedata)
	{
		node.tr->WriteData(T, pEvaluateInfo);
		node.rot->WriteData(R, pEvaluateInfo);
	}

	return true;
}

//--- Real-Time Engine

bool MOPhysicsSolver::EvaluateAnimationNodes( FBEvaluateInfo *pEvaluateInfo )
{
	bool result = ParentClass::EvaluateAnimationNodes(pEvaluateInfo);
	/*
	for (auto iter=begin(mCars); iter!=end(mCars); ++iter)
	{
		// send input
		double torque=0.0;
		double steering=0.0;
		double brake=0.0;
		double handbrake=0.0;
		double gear = 0.0;

		if (iter->torque)
			iter->torque->ReadData( &torque, pEvaluateInfo );
		if (iter->steering)
			iter->steering->ReadData( &steering, pEvaluateInfo );
		if (iter->brake)
			iter->brake->ReadData( &brake, pEvaluateInfo );
		if (iter->handbrake)
			iter->handbrake->ReadData( &handbrake, pEvaluateInfo );
		if (iter->gear)
			iter->gear->ReadData( &gear, pEvaluateInfo );

		iter->car->SetPlayerControl( 0.01*torque, 0.01*steering, 0.01*brake, 0.01*handbrake, gear );

		//
		// output speed and rpm
		double speed = 3.6 * iter->car->GetSpeed();
		double rpm = iter->car->GetRPM();

		
		if (iter->speedProp)
			iter->speedProp->SetData( &speed );
		if (iter->rpmProp)
			iter->rpmProp->SetData( &rpm );
	}
	*/
	return result;
}

bool MOPhysicsSolver::UpdateInput(FBEvaluateInfo* pEvaluateInfo)
{
	for (auto iter=begin(mCars); iter!=end(mCars); ++iter)
	{
			// send input
			double torque=0.0;
			double clutch=0.0;
			double steering=0.0;
			double steeringBlend=0.0;
			double brake=0.0;
			double handbrake=0.0;
			double gear = 0.0;

			bool readSuccess = true;

			if (iter->torque)
				readSuccess &= iter->torque->ReadData( &torque, pEvaluateInfo );
			if (iter->clutch)
				readSuccess &= iter->clutch->ReadData( &clutch, pEvaluateInfo );
			if (iter->steering)
				readSuccess &= iter->steering->ReadData( &steering, pEvaluateInfo );
			if (iter->steeringBlend)
				readSuccess &= iter->steeringBlend->ReadData( &steeringBlend, pEvaluateInfo );
			if (iter->brake)
				readSuccess &= iter->brake->ReadData( &brake, pEvaluateInfo );
			if (iter->handbrake)
				readSuccess &= iter->handbrake->ReadData( &handbrake, pEvaluateInfo );
			if (iter->gear)
				readSuccess &= iter->gear->ReadData( &gear, pEvaluateInfo );

			if (readSuccess)
			{
				
				if (torque == mLastTorque)
				{
					mLastTorque = torque + 1.0;
					iter->car->SetPlayerControl( 0.01*mLastTorque, 0.01*clutch, 0.01*steering, 0.01*steeringBlend, 0.01*brake, 0.01*handbrake, gear );
					
				}
				else
				{
					iter->car->SetPlayerControl( 0.01*torque, 0.01*clutch, 0.01*steering, 0.01*steeringBlend, 0.01*brake, 0.01*handbrake, gear );
					mLastTorque = torque;
				}

				
			}
			else
			{
				printf( "failed to read car input properties\n" );
			}
	}

	return true;
}

bool MOPhysicsSolver::UpdatePhysics(FBEvaluateInfo* pEvaluateInfo)
{
	
	FBTime evaluateTime;

	bool isStop = pEvaluateInfo->IsStop();
	bool localMode = LiveState == false;

	if (localMode)
		evaluateTime = pEvaluateInfo->GetLocalTime();
	else
		evaluateTime = pEvaluateInfo->GetSystemTime();

	double evalTimeSecs = evaluateTime.GetSecondDouble();

	bool isRecording = mPlayerControl.IsRecording;

	if (isRecording && isRecording != mLastIsRecording)
	{
		mHardware->Reset();
	}
	else
	if (isStop && isStop != mLastIsStop )
	{
		// Reset ?!
		mHardware->Reset();
	}

	if (mHardware.get() != nullptr)
	{
		mPhysTimeSecs = mHardware->GetCurrPhysTimeSecs();
		mLastPhysTimeSecs = mHardware->GetLastPhysTimeSecs();
	}

	// TODO: we are including a RESET time here
	if (evalTimeSecs <= 0.0)
	{
		return false;
	}

	if ( isStop == false && evalTimeSecs > mPhysTimeSecs )
	{
		if (mHardware.get() != nullptr)
		{
			mHardware->FetchDataPacket( evalTimeSecs );

			mPhysTimeSecs = mHardware->GetCurrPhysTimeSecs();
			mLastPhysTimeSecs = mHardware->GetLastPhysTimeSecs();
		}
	}

	if (evalTimeSecs < mLastPhysTimeSecs)
	{
		return false;
	}
	if (evalTimeSecs > mPhysTimeSecs)
	{
		return false;
	}
	if (mPhysTimeSecs < mLastPhysTimeSecs)
	{
		return false;
	}

	mLastIsRecording = isRecording;
	mLastIsStop = isStop;
	mLastTimeLocal = localMode;
	mAnimTimeSecs = evalTimeSecs;

	//
	//
	if (mHardware.get() != nullptr)
		mHardware->WaitForUpdateToFinish();

	return true;
}

bool MOPhysicsSolver::UpdateAllCars(FBEvaluateInfo *pEvaluateInfo)
{
	// 2 - write a result to animation node
	FBMatrix m;
	FBTVector T;
	FBRVector R;
	FBSVector S;

	bool isLive = (Live == true);
	bool writedata = (Live == true || (RecordState == true && !pEvaluateInfo->IsStop() ) );
	bool result = false;

	for (auto iter=begin(mCars); iter!=end(mCars); ++iter)
	{
		// interpolated value, we are using last anim time passed
		m.Set( iter->car->GetChassisMatrix() );

		FBMatrixToTRS( T, R, S, m );

		if (writedata)
		{
			//if (pAnimationNode == iter->chassis.tr)
			{
				iter->chassis.tr->WriteData(T, pEvaluateInfo);
				result = true;
				//return true;
			}

			//if (pAnimationNode == iter->chassis.rot)
			{
				iter->chassis.rot->WriteData(R, pEvaluateInfo);
				result = true;
				//return true;
			}
		}

		//
		// write data for each wheel
		//
		
		for (int i=1; i<=4; ++i)
		{
			const double *wheelMatrix = iter->car->GetWheelMatrix(i-1, true);
			if (wheelMatrix == nullptr)
				continue;

			m.Set( wheelMatrix );
			FBMatrixToTRS( T, R, S, m );

			if (writedata)
			{
				//if (pAnimationNode == iter->wheels[i-1].tr )
				{
					iter->wheels[i-1].tr->WriteData(T, pEvaluateInfo);
					result = true;
					//return true;
				}

				//if (pAnimationNode == iter->wheels[i-1].rot )
				{
					iter->wheels[i-1].rot->WriteData(R, pEvaluateInfo);
					result = true;
					//return true;
				}
			}
		}
		
		// output speed and rpm
		double speed = 3.6 * iter->car->GetSpeed();
		double rpm = iter->car->GetRPM();
		int gear = iter->car->GetCurrentGear();
		
		if (iter->speedProp)
			iter->speedProp->SetData( &speed );
		if (iter->rpmProp)
			iter->rpmProp->SetData( &rpm );
		if (iter->currentGearProp)
			iter->currentGearProp->SetData( &gear );
	}

	return true;
}

//! Real-time evaluation engine function.
bool MOPhysicsSolver::AnimationNodeNotify ( FBAnimationNode* pAnimationNode, FBEvaluateInfo* pEvaluateInfo, FBConstraintInfo* pConstraintInfo )
{
	if (pAnimationNode == nullptr || mHardware.get() == nullptr )
		return false;

	// 1 - evaluate physics

	bool result = false;

	UpdateInput(pEvaluateInfo);
	result = UpdatePhysics(pEvaluateInfo);

	//
	// all scene simple rigid bodies
	/*
	for (auto iter=begin(mRigidBodies); iter!=end(mRigidBodies); ++iter)
	{
		WritePhysNodeData(*iter, writedata, pEvaluateInfo);
	}
	
	//
	// all scene dynamic chains (socket joints inbetween)
	for (auto iter=begin(mChainNodes); iter!=end(mChainNodes); ++iter)
	{

		for (int i=0; i<MAX_NUMBER_OF_CHAIN_JOINS; ++i)
		{
			WritePhysNodeData(iter->nodes[i], writedata, pEvaluateInfo);
		}
	}
	*/
	//
	// all scene cars

	if (result)
	{
		result = UpdateAllCars(pEvaluateInfo);
	}

	if (result == false)
	{
		pAnimationNode->DisableIfNotWritten(pEvaluateInfo);
		//AnimationNodesOutDisableIfNotWritten(pEvaluateInfo);
	}

	return result;
}


void MOPhysicsSolver::FreezeSRT	( FBModel* pModel, bool pS, bool pR, bool pT )
{
}

bool MOPhysicsSolver::ReferenceAddNotify	( int pGroupIndex, FBModel* pModel )
{
	return true;
}

bool MOPhysicsSolver::ReferenceRemoveNotify ( int pGroupIndex, FBModel* pModel )
{
	return true;
}


//--- FBX Interface
bool MOPhysicsSolver::FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	return true;
}

bool MOPhysicsSolver::FbxRetrieve ( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat )
{
	Active = false;
	return true;
}

void MOPhysicsSolver::LiveChanged()
{
}



void MOPhysicsSolver::DoRebuildCollisions()
{
	mNeedRebuild = true;
}

void MOPhysicsSolver::DoRestart()
{

	// rebuild collisions
	mNeedRebuild = true;
}

bool MOPhysicsSolver::LoadLevel( FBComponentList &list )
{
	if (mHardware.get() == nullptr) return false;

	LevelGeometry	info(list);
	mHardware->LoadLevel( &info );

	DoUpdateDefaultMaterialParams();
	mNeedRebuild = false;

	return true;
}

void MOPhysicsSolver::DoUpdateDefaultMaterialParams()
{
	double softness = 0.01 * DefaultSoftness;
	double elasticity = 0.01 * DefaultElasticity;
	double collidable = 0.01 * DefaultCollidable;
	double staticFriction = 0.01 * DefaultStaticFriction;
	double dynamicFriction = 0.01 * DefaultDynamicFriction;

	if (mHardware.get() != nullptr)
	{
		mHardware->SetDefaultMaterialParams(softness, elasticity, collidable, staticFriction, dynamicFriction);
	}
}

PHYSICS_INTERFACE::IBody *MOPhysicsSolver::CreateNewBody( PHYSICS_INTERFACE::BodyOptions *pOptions, const PHYSICS_INTERFACE::IQueryGeometry *pGeometry, bool convexhull  )
{
	if (mHardware.get() != nullptr)
		return mHardware->CreateNewBody( pOptions, pGeometry, convexhull );

	return nullptr;
}

PHYSICS_INTERFACE::ICar *MOPhysicsSolver::CreateNewCar( PHYSICS_INTERFACE::CarOptions *pOptions, const PHYSICS_INTERFACE::IQueryGeometry *pCarGeometry[5], const PHYSICS_INTERFACE::IQueryPath *curve )
{
	if (mHardware.get() != nullptr)
		return mHardware->CreateNewCar( pOptions, pCarGeometry, curve );

	return nullptr;
}

void MOPhysicsSolver::AllocateRigidBodies(const int count)
{
	FreeRigidBodies();

	mRigidBodies.resize(count);

	for (auto iter=mRigidBodies.begin(); iter!=mRigidBodies.end(); ++iter)
	{
		iter->tr = nullptr;
		iter->rot = nullptr;
		iter->body = nullptr;
	}
}

void MOPhysicsSolver::FreePhysNode(PhysNode &pNode)
{
	if (pNode.tr != nullptr)
	{
		//AnimationNodeDestroy(iter->tr);
		pNode.tr = nullptr;
	}
	if (pNode.rot != nullptr)
	{
		//AnimationNodeDestroy(iter->rot);
		pNode.rot = nullptr;
	}

	if (pNode.body != nullptr)
	{
		delete pNode.body;
		pNode.body = nullptr;
	}
}

void MOPhysicsSolver::FreeRigidBodies()
{
	for (auto iter=mRigidBodies.begin(); iter!=mRigidBodies.end(); ++iter)
	{
		FreePhysNode(*iter);
	}
}

void MOPhysicsSolver::AllocateChainNodes(const int count)
{
	FreeChainNodes();

	mChainNodes.resize(count);

	for (auto iter=begin(mChainNodes); iter!=end(mChainNodes); ++iter)
	{
		for (int i=0; i<MAX_NUMBER_OF_CHAIN_JOINS; ++i)
		{
			auto &node = iter->nodes[i];
			
			node.tr = nullptr;
			node.rot = nullptr;
			node.body = nullptr;
		}
	}
}

void MOPhysicsSolver::FreeChainNodes()
{
	for (auto iter=begin(mChainNodes); iter!=end(mChainNodes); ++iter)
	{
		for (int i=0; i<MAX_NUMBER_OF_CHAIN_JOINS; ++i)
		{
			FreePhysNode(iter->nodes[i]);
		}
	}
}

void MOPhysicsSolver::AllocateCars(const int count)
{
	FreeCars();

	mCars.resize(count);

	for (auto iter=mCars.begin(); iter!=mCars.end(); ++iter)
	{
		iter->props = nullptr;
		iter->car = nullptr;
		iter->chassis.rot = nullptr;
		iter->chassis.tr = nullptr;

		for (int i=0; i<4; ++i)
		{
			iter->wheels[i].tr = nullptr;
			iter->wheels[i].rot = nullptr;
		}
	}
}

void MOPhysicsSolver::FreeCar(std::vector<CarNode>::iterator	&iter)
{
	// enable some car physics properties
	if (iter->props != nullptr)
	{
		( (MOCarPhysProperties*) iter->props )->StopOnline();
		iter->props = nullptr;
	}

	// remove newton body
	if (iter->car != nullptr)
	{
		delete iter->car;
		iter->car = nullptr;
	}
}

void MOPhysicsSolver::FreeCars()
{
	for (auto iter=mCars.begin(); iter!=mCars.end(); ++iter)
	{
		FreeCar(iter);
	}
}

/*
void ComputeAlignMatrix(FBModel *pModel, float *localFront, float *localUp, float *localRight, FBMatrix &alignMatrix)
{
	if (pModel == nullptr || localFront == nullptr || localUp == nullptr || localRight == nullptr)
		return;

	alignMatrix.Identity();

	FBMatrix m;
	pModel->GetMatrix(m);

	FBVector4d base4(0.0, 0.0, 0.0, 1.0);
	FBVector4d front4(localFront[0], localFront[1], localFront[2], 1.0);
	FBVector4d up4(localUp[0], localUp[1], localUp[2], 1.0);
	FBVector4d right4(localRight[0], localRight[1], localRight[2], 1.0);
	
	FBVectorMatrixMult(base4, m, base4);
	FBVectorMatrixMult(front4, m, front4);
	FBVectorMatrixMult(up4, m, up4);
	FBVectorMatrixMult(right4, m, right4);

	front4 = FBVector4d(front4[0]-base4[0], front4[1]-base4[1], front4[2]-base4[2]);
	up4 = FBVector4d(up4[0]-base4[0], up4[1]-base4[1], up4[2]-base4[2]);
	right4 = FBVector4d(right4[0]-base4[0], right4[1]-base4[1], right4[2]-base4[2]);

	double len;
	
	len = FBLength(front4);
	if (len > 0.0)
		FBMult(front4, front4, 1.0 / len );

	len = FBLength(up4);
	if (len > 0.0)
		FBMult(up4, up4, 1.0 / len );

	len = FBLength(right4);
	if (len > 0.0)
		FBMult(right4, right4, 1.0 / len );

	FBMatrix matrix, matrix2;

	matrix.Identity();
	matrix2.Identity();

	matrix2[0] = front4[0];
	matrix2[1] = front4[1];
	matrix2[2] = front4[2];
	matrix2[4] = up4[0];
	matrix2[5] = up4[1];
	matrix2[6] = up4[2];
	matrix2[8] = right4[0];
	matrix2[9] = right4[1];
	matrix2[10] = right4[2];

	matrix2.Inverse();

	// get local matrix for object align
	//FBGetLocalMatrix( alignMatrix, matrix, matrix2 );
	FBMatrixMult( alignMatrix, matrix, matrix2 );
	FBRVector rot;
	FBMatrixToRotation(rot, alignMatrix );
	//alignMatrix.Inverse();
}
*/

void FillRowWithAxis( EAxis localAxis, double *front4, double *up4, double *right4, double *row )
{
	switch(localAxis)
	{
	case kAxisPositiveX:
		memcpy(row, front4, sizeof(double) * 3);
		break;
	case kAxisNegativeX:
		row[0] = -front4[0];
		row[1] = -front4[1];
		row[2] = -front4[2];
		break;
	//
	case kAxisPositiveY:
		memcpy(row, up4, sizeof(double) * 3);
		break;
	case kAxisNegativeY:
		row[0] = -up4[0];
		row[1] = -up4[1];
		row[2] = -up4[2];
		break;
	//
	case kAxisPositiveZ:
		memcpy(row, right4, sizeof(double) * 3);
		break;
	case kAxisNegativeZ:
		row[0] = -right4[0];
		row[1] = -right4[1];
		row[2] = -right4[2];
		break;
	}
}

void ComputeAlignMatrix( FBModel *pModel, EAxis localFront, EAxis localUp, EAxis localRight, FBMatrix &alignMatrix, FBMatrix &visualAlignMatrix )
{
	if (pModel == nullptr)
		return;

	alignMatrix.Identity();

	FBMatrix m;
	pModel->GetMatrix(m, kModelRotation);

	FBVector4d base4(0.0, 0.0, 0.0, 1.0);
	FBVector4d front4(	m[0], m[1], m[2], 1.0);
	FBVector4d up4(		m[4], m[5], m[6], 1.0);
	FBVector4d right4(	m[8], m[9], m[10], 1.0);
	
	FBMatrix matrix, matrix2;

	matrix.Identity();
	matrix2.Identity();

	FillRowWithAxis( localFront, front4, up4, right4, &matrix2[0] );
	FillRowWithAxis( localUp, front4, up4, right4, &matrix2[4] );
	FillRowWithAxis( localRight, front4, up4, right4, &matrix2[8] );

	matrix2.Inverse();

	// get local matrix for object align
	//FBGetLocalMatrix( alignMatrix, matrix, matrix2 );
	FBMatrixMult( alignMatrix, matrix, matrix2 );
	FBRVector rot;
	FBMatrixToRotation(rot, alignMatrix );
	//alignMatrix.Inverse();

	// calculate align matrix for visual part
	//matrix.Identity();
	//matrix2.Identity();

	matrix = m;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = 0.0;

	//FBMatrixInverse( matrix2, alignMatrix );
	FBMatrixMult( visualAlignMatrix, matrix, matrix2 );
}

void MOPhysicsSolver::EnterOnline()
{
	WaitForUpdateToFinish();

	//
	// DONE: prepare all physics properties

	// calculate number of nodes
	int numberOfNodes = 0;
	int numberOfChains = 0;
	int numberOfCars = 0;

	for (int i=0; i<mSystem.Scene->PhysicalProperties.GetCount(); ++i)
	{
		FBPhysicalProperties *pPhysProps = mSystem.Scene->PhysicalProperties[i];
		int count = pPhysProps->GetDstCount();

		if (strcmp(pPhysProps->ClassName(), "KxL_RigidBodyProperty") == 0)
		{
			for (int j=1; j<count; ++j)
			{
				FBPlug *plug = pPhysProps->GetDst(j);

				if (FBIS(plug, FBModel) )
				{
					numberOfNodes += 1;
				}
			}
		}
		else if (FBIS( pPhysProps, MOChainPhysProperties ) )
		{
			MOChainPhysProperties *chainProps = (MOChainPhysProperties*) pPhysProps;

			if (chainProps->Active == true && chainProps->IsAssigned() )
			{
				numberOfChains += 1;
			}
		}
		else if (FBIS( pPhysProps, MOCarPhysProperties) )
		{
			MOCarPhysProperties *carProps = (MOCarPhysProperties*) pPhysProps;

			if (carProps->Active==true && carProps->IsAssigned() )
			{
				numberOfCars += 1;
			}
		}
	}

	AllocateRigidBodies(numberOfNodes);
	AllocateChainNodes(numberOfChains);
	AllocateCars( numberOfCars );

	// create out animation nodes for controlling elements in the scene
	numberOfNodes = 0;

	auto &rigidbody = begin(mRigidBodies);
	
	for (int i=0; i<mSystem.Scene->PhysicalProperties.GetCount(); ++i)
	{
		FBPhysicalProperties *pPhysProps = mSystem.Scene->PhysicalProperties[i];

		if (strcmp(pPhysProps->ClassName(), "KxL_RigidBodyProperty") == 0)
		{
			// DONE: take body options from rigid body property object
			PHYSICS_INTERFACE::BodyOptions options;
			options.mass = 5.0;
			options.friction = 1.0;

			int count = pPhysProps->GetDstCount();

			for (int j=1; j<count; ++j)
			{
				FBPlug *plug = pPhysProps->GetDst(j);

				if (FBIS(plug, FBModel) )
				{
					FBModel *pmodel = (FBModel*) plug;

					rigidbody->geometry.Prep(pmodel, pmodel, false, nullptr, nullptr, nullptr);

					rigidbody->tr = AnimationNodeInCreate(numberOfNodes, pmodel, ANIMATIONNODE_TYPE_TRANSLATION);
					rigidbody->rot = AnimationNodeInCreate(numberOfNodes*2+1, pmodel, ANIMATIONNODE_TYPE_ROTATION);
					numberOfNodes += 2;

					rigidbody->body = CreateNewBody(&options, &rigidbody->geometry, true);

					rigidbody->tr->Reference = (kReference) rigidbody->body;
					rigidbody->rot->Reference = (kReference) rigidbody->body;

					rigidbody->body->UserData1 = rigidbody->tr;
					rigidbody->body->UserData2 = rigidbody->rot;

					rigidbody++;
				}
			}
		}
	}

	auto &supercar = begin(mCars);

	for (int i=0; i<mSystem.Scene->PhysicalProperties.GetCount(); ++i)
	{
		FBPhysicalProperties *pPhysProps = mSystem.Scene->PhysicalProperties[i];

		if (FBIS( pPhysProps, MOCarPhysProperties ) )
		{
			MOCarPhysProperties *carProps = (MOCarPhysProperties*) pPhysProps;

			if (carProps->Active==true && carProps->IsAssigned() )
			{
				PHYSICS_INTERFACE::CarOptions	options;
				carProps->FillCarInfo(options);

				//
				FBMatrix alignMatrix, visualAlignMatrix;
				ComputeAlignMatrix(carProps->GetChassisObject(), (EAxis)options.localFrontAxis, (EAxis)options.localUpAxis, (EAxis)options.localRightAxis, alignMatrix, visualAlignMatrix);

				//
				if ( (false == supercar->chassis.geometry.Prep( carProps->GetChassisObject(), carProps->GetChassisCollision(), true, nullptr, &alignMatrix, &visualAlignMatrix ) )
					|| (false == supercar->wheels[0].geometry.Prep( carProps->GetWheelObject(0), carProps->GetWheelCollision(0), true, carProps->GetChassisObject(), &alignMatrix, &visualAlignMatrix ) )
					|| (false == supercar->wheels[1].geometry.Prep( carProps->GetWheelObject(1), carProps->GetWheelCollision(1), true, carProps->GetChassisObject(), &alignMatrix, &visualAlignMatrix ) )
					|| (false == supercar->wheels[2].geometry.Prep( carProps->GetWheelObject(2), carProps->GetWheelCollision(2), true, carProps->GetChassisObject(), &alignMatrix, &visualAlignMatrix ) )
					|| (false == supercar->wheels[3].geometry.Prep( carProps->GetWheelObject(3), carProps->GetWheelCollision(3), true, carProps->GetChassisObject(), &alignMatrix, &visualAlignMatrix ) ) )
				{
					// failed to prepare some part of a car, skip this car simulation
					continue;
				}

				supercar->steeringCurve.Prep( carProps->GetSteeringPath(), mWorldScaling, carProps->CurveSamples );

				const PHYSICS_INTERFACE::IQueryGeometry *ptr[] = {&supercar->chassis.geometry, &supercar->wheels[0].geometry, &supercar->wheels[1].geometry, &supercar->wheels[2].geometry, &supercar->wheels[3].geometry};
				supercar->car = CreateNewCar(&options, ptr, &supercar->steeringCurve);

				// create animation node conenctions
				supercar->chassis.tr = AnimationNodeInCreate(numberOfNodes, carProps->GetChassisObject(), ANIMATIONNODE_TYPE_TRANSLATION);
				supercar->chassis.rot = AnimationNodeInCreate(numberOfNodes+1, carProps->GetChassisObject(), ANIMATIONNODE_TYPE_ROTATION);
				numberOfNodes += 2;

				supercar->wheels[0].tr = AnimationNodeInCreate(numberOfNodes, carProps->GetWheelObject(0), ANIMATIONNODE_TYPE_TRANSLATION);
				supercar->wheels[0].rot = AnimationNodeInCreate(numberOfNodes+1, carProps->GetWheelObject(0), ANIMATIONNODE_TYPE_ROTATION);
				numberOfNodes += 2;

				supercar->wheels[1].tr = AnimationNodeInCreate(numberOfNodes, carProps->GetWheelObject(1), ANIMATIONNODE_TYPE_TRANSLATION);
				supercar->wheels[1].rot = AnimationNodeInCreate(numberOfNodes+1, carProps->GetWheelObject(1), ANIMATIONNODE_TYPE_ROTATION);
				numberOfNodes += 2;

				supercar->wheels[2].tr = AnimationNodeInCreate(numberOfNodes, carProps->GetWheelObject(2), ANIMATIONNODE_TYPE_TRANSLATION);
				supercar->wheels[2].rot = AnimationNodeInCreate(numberOfNodes+1, carProps->GetWheelObject(2), ANIMATIONNODE_TYPE_ROTATION);
				numberOfNodes += 2;

				supercar->wheels[3].tr = AnimationNodeInCreate(numberOfNodes, carProps->GetWheelObject(3), ANIMATIONNODE_TYPE_TRANSLATION);
				supercar->wheels[3].rot = AnimationNodeInCreate(numberOfNodes+1, carProps->GetWheelObject(3), ANIMATIONNODE_TYPE_ROTATION);
				numberOfNodes += 2;

				supercar->torque = carProps->Torque.GetAnimationNode();
				supercar->clutch = carProps->Clutch.GetAnimationNode();
				supercar->steering = carProps->Steering.GetAnimationNode();
				supercar->steeringBlend = carProps->CurveSteeringWeight.GetAnimationNode();
				supercar->brake = carProps->Brake.GetAnimationNode();
				supercar->handbrake = carProps->HandBrake.GetAnimationNode();
				supercar->gear = carProps->Gear.GetAnimationNode();

				//supercar->speed = AnimationNodeInCreate( numberOfNodes, &carProps->Speed );
				//supercar->rpm = AnimationNodeInCreate( numberOfNodes+1, &carProps->RPM );
				supercar->speed = nullptr; // carProps->Speed.GetAnimationNode();
				supercar->rpm = nullptr; // carProps->RPM.GetAnimationNode();
				supercar->speedProp = &carProps->Speed;
				supercar->rpmProp = &carProps->RPM;
				supercar->currentGearProp = &carProps->DisplayGear;
				numberOfNodes += 2;

				supercar->props = carProps;
				carProps->StartOnline( mHardware.get(), supercar->car );
				
				supercar++;
			}
		}
	}

	//
	// properties disable

	WorldScale.SetEnable(false);
	StaticCollisions.SetEnable(false);

	PhysicsThreads.SetEnable(false);
	PhysicsSamples.SetEnable(false);
	PhysicsFPS.SetEnable(false);

	LiveState.SetEnable(true);
	RecordState.SetEnable(true);

	

	//
	// static collisions
	//
	FBProperty *pProperty = &StaticCollisions;
	FBPropertyListObject *pPropList = (FBPropertyListObject*) pProperty;

	
	mCollisions.SetCount( pPropList->GetCount() );

	for (int i=0; i<mCollisions.GetCount(); ++i)
		mCollisions.SetAt(i, pPropList->GetAt(i) );

	LoadLevel( mCollisions );
}

void MOPhysicsSolver::LeaveOnline()
{
	WorldScale.SetEnable(true);
	StaticCollisions.SetEnable(true);

	PhysicsThreads.SetEnable(true);
	PhysicsSamples.SetEnable(true);
	PhysicsFPS.SetEnable(true);

	LiveState.SetEnable(false);
	RecordState.SetEnable(false);

	//ClearLevel();

	FreeRigidBodies();
	FreeChainNodes();
	FreeCars();
}

void MOPhysicsSolver::DrawDebug() const 
{ 
	if (mHardware.get()) mHardware->DrawDebug(); 

	for (auto iter=begin(mCars); iter!=end(mCars); ++iter)
	{
		iter->steeringCurve.DrawDebug();
	}
}

bool MOPhysicsSolver::DisconnectACar(FBPhysicalProperties	*props)
{

	for (auto iter=begin(mCars); iter!=end(mCars); ++iter)
	{
		if (iter->props == props)
		{
			FreeCar(iter);
			mCars.erase(iter);
			return true;
		}
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// MO Physics Layout

bool MOPhysicsSolverLayout::FBCreate()
{
	return true;
}

void MOPhysicsSolverLayout::FBDestroy()
{
}