#ifndef __ORDEVICE_SKELETON_DEVICE_H__
#define __ORDEVICE_SKELETON_DEVICE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: device_shapeHand_device.h
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

//--- Class declaration
#include "device_shapehand_hardware.h"

//--- Registration defines
#define DEVICESHAPEHAND_CLASSNAME		DeviceShapeHand
#define DEVICESHAPEHAND_CLASSSTR		"DeviceShapeHand"

//! Data channel class.
class DataChannel
{
public:
	//! Constructor.
	DataChannel()
	{
		mTAnimNode		= NULL;
		mRAnimNode		= NULL;
		mModelTemplate	= NULL;
	}

	//! Destructor.
	~DataChannel()
	{
	}

public:
	FBAnimationNode*	mTAnimNode;			//!< Position animation node.
	FBAnimationNode*	mRAnimNode;			//!< Rotation animation node.
	FBModelTemplate*	mModelTemplate;		//!< Marker model template driven by the data channel.
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
//! ShapeHand skeleton device.
class DeviceShapeHand : public FBDevice
{
	//--- Declaration
	FBDeviceDeclare( DeviceShapeHand, FBDevice );

public:
	virtual bool FBCreate() override;
	virtual void FBDestroy() override;

    //--- Standard device virtual functions evaluation
	virtual bool DeviceOperation	( kDeviceOperations pOperation									) override;		//!< Operate device.
	virtual bool AnimationNodeNotify( FBAnimationNode* pAnimationNode,FBEvaluateInfo* pEvaluateInfo	) override;		//!< Real-time evaluation function.
	virtual void DeviceIONotify		( kDeviceIOs  pAction, FBDeviceNotifyInfo &pDeviceNotifyInfo	) override;		//!< Hardware I/O notification.
	virtual void DeviceTransportNotify( kTransportMode pMode, FBTime pTime, FBTime pSystem ) override;				//!< Transport notification.

	//--- Recording of frame information
	virtual void	RecordingDoneAnimation( FBAnimationNode* pAnimationNode) override;
	void	DeviceRecordFrame			(FBTime &pTime,FBDeviceNotifyInfo &pDeviceNotifyInfo);

	//--- Load/Save.
	virtual bool FbxStore	( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Storage.
	virtual bool FbxRetrieve( FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat ) override;	//!< FBX Retrieval.

	//--- Initialisation/Shutdown
    bool	Init	();		//!< Initialize device.
    bool	Start	();		//!< Start device.
    bool	Stop	();		//!< Stop device.
    bool	Reset	();		//!< Reset device.
    bool	Done	();		//!< Device removal.

	//--- Attributes management
	void	SetNetworkUse(bool pUse)					{ mHardware.SetNetworkUse( pUse );				}
	bool	GetNetworkUse()								{ return mHardware.GetNetworkUse();				}
	void	SetNetworkAddress	(const char* pIPAddress){ mHardware.SetNetworkAddress(pIPAddress);		}
	const char*	GetNetworkAddress	()					{ return mHardware.GetNetworkAddress();			}
	void	SetNetworkPort		(int pPort)				{ mHardware.SetNetworkPort(pPort);				}
	int		GetNetworkPort		()						{ return mHardware.GetNetworkPort();			}

	//--- Get data from hardware.
	double GetDataTX(int pChannelIndex)			{	return mHardware.GetDataTX(pChannelIndex);		}
	double GetDataTY(int pChannelIndex)			{	return mHardware.GetDataTY(pChannelIndex);		}
	double GetDataTZ(int pChannelIndex)			{	return mHardware.GetDataTZ(pChannelIndex);		}
	double GetDataRX(int pChannelIndex)			{	return mHardware.GetDataRX(pChannelIndex);		}
	double GetDataRY(int pChannelIndex)			{	return mHardware.GetDataRY(pChannelIndex);		}
	double GetDataRZ(int pChannelIndex)			{	return mHardware.GetDataRZ(pChannelIndex);		}
	FBVector4d GetDataQ(int pChannelIndex)		{	return mHardware.GetDataQ(pChannelIndex);		}

	//--- Marker set management.
	void	DefineHierarchy				();				//!< Define model template hierarchy.
	void	Bind();										//!< Bind models
	void	UnBind();									//!< Unbind models

	//--- Channel list manipulation.
	int		GetChannelCount()					{	return mHardware.GetChannelCount();				}
	int		GetChannelParent(int pMarkerIndex)	{	return mHardware.GetChannelParent(pMarkerIndex);}
	const char*	GetChannelName(int pMarkerIndex){	return mHardware.GetChannelName(pMarkerIndex);	}

    virtual bool ModelTemplateUnBindNotify( int pIndex, FBModelTemplate* pModelTemplate );							//!< Model Template unbinding notification callback.

	bool	ReadyForCharacterize		();				//!< Test if characterization process can be start.

	//--- Stop displaying process to local message on model unbind
	void	EventUIIdle( HISender pSender, HKEvent pEvent );

	DeviceShapeHandHardware		mHardware;					//!< Hardware abstraction object.
	FBModelTemplate*			mRootTemplate;				//!< Root model binding.
private:
	DataChannel					mChannels[sSKDataBuffer::ChannelCount];	//!< Data channels.
	FBPropertyBool				UseReferenceTransformation;	// !< Apply reference transformation on incoming global data.
	bool						mHierarchyIsDefined;		//!< True if the hierarchy is already defined

	FBPlayerControl				mPlayerControl;				//!< To get play mode for recording.
	FBSystem					mSystem;
	FBApplication				mApplication;
	bool						mHasAnimationToTranspose;
	bool						mPlotting;

	// Process global data on template models to local
	void		ProcessGlobalToLocal();
	void SetupLocalGlobal(bool pGlobal);
};

#endif /* __ORDEVICE_SKELETON_DEVICE_H__ */
