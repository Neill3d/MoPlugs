#ifndef __ORDEVICE_SKELETON_HARDWARE_H__
#define __ORDEVICE_SKELETON_HARDWARE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: device_shapehand_hardware.h
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

// Test server include
#include "skeletondevicetester/DeviceBuffer.h"

//! Simple hardware.
class DeviceShapeHandHardware
{
public:
	//! Constructor & destructor
	DeviceShapeHandHardware();
	~DeviceShapeHandHardware();

	//--- Opens and closes connection with data server. returns true if successful
	bool	Open();								//!< Open the connection.
	bool	Close();							//!< Close connection.

	bool	GetSetupInfo	();						//!< Get the setup information.

	//--- Hardware communication
	bool	StartDataStream	();						//!< Put the device in streaming mode.
	bool	StopDataStream	();						//!< Take the device out of streaming mode.

	void	ResetPacketTimeOffset(FBTime &pTime);	//!< Reset time offset for recording
	bool	FetchDataPacket	(FBTime &pTime);		//!< Fetch a data packet from the computer.

	//--- Attribute management
	void		SetNetworkUse(bool pUse)					{ mNetworkUse =  pUse;						}
	bool		GetNetworkUse()								{ return mNetworkUse;						}
	void		SetNetworkAddress	(const char *pIPAddress){ mNetworkAddress =  pIPAddress;			}
	const char*	GetNetworkAddress	()						{ return mNetworkAddress;			}
	void		SetNetworkPort		(int pPort)				{ mNetworkPortShapeHand = pPort;						}
	int			GetNetworkPort		()						{ return mNetworkPortShapeHand;						}

	//--- Channel & Channel data management
	int		GetChannelCount	()					{	return mChannelCount;						}
	const char*	GetChannelName	(int pChannel)	{	return mChannel[pChannel].mName;	}
	int		GetChannelParent(int pChannel)		{	return mChannel[pChannel].mParentChannel;	}
	FBVector3d GetDefaultT	(int pChannel)		{	SkeletonNodeInfo& lSI = mChannel[pChannel]; return FBVector3d(lSI.mDefaultT[0],lSI.mDefaultT[1],lSI.mDefaultT[2]);		}
	FBVector4d GetDefaultR	(int pChannel)		{	SkeletonNodeInfo& lSI = mChannel[pChannel]; return FBVector4d(lSI.mDefaultR[0],lSI.mDefaultR[1],lSI.mDefaultR[2], lSI.mDefaultR[3]);		}
	double	GetDataTX		(int pChannel)		{	return mChannel[pChannel].mT[0];		}
	double	GetDataTY		(int pChannel)		{	return mChannel[pChannel].mT[1];		}
	double	GetDataTZ		(int pChannel)		{	return mChannel[pChannel].mT[2];		}
	double	GetDataRX		(int pChannel)		{	return mChannel[pChannel].mR[0];		}
	double	GetDataRY		(int pChannel)		{	return mChannel[pChannel].mR[1];		}
	double	GetDataRZ		(int pChannel)		{	return mChannel[pChannel].mR[2];		}
	double	GetDataRQ		(int pChannel)		{	return mChannel[pChannel].mR[3];		}
	FBVector4d GetDataQ (int pChannel) { return FBVector4d( mChannel[pChannel].mR ); }
private:
	struct SkeletonNodeInfo
	{
		const char*	mName;
		int			mParentChannel;
		double		mDefaultT[3];
		double		mDefaultR[4];
		double		mT[3];
		double		mR[4];
	};

	SkeletonNodeInfo	mChannel[sSKDataBuffer::ChannelCount];//!< Channel data & info.
	int			mChannelCount;								//!< Channel count.
	sSKDataBuffer mSkDataBuffer;							//!< IO buffer
	sClientRequestPacket	packet;

	int					mBigBufferSize;
	char				*mBigBuffer;

	nsTime		mPacketTimeOffset;							//!< Packet time offset for recording
	FBTime		mStartRecordingTime;						//!< Time at start of recording

	FBTime		mLastSimulationTime;						//!< Last simulation time for internal simulation
	int			mPassCount;									//!< Pass counter for internal simulation

	bool		mNetworkUse;								//!< Try to use network
	FBString	mNetworkAddress;							//!< Network address.
	int			mNetworkPortPlugin;								//!< Network port.
	int			mNetworkPortShapeHand;
	FBSystem	mSystem;									//!< System interface.

	int			mSocket;									//!< Socket for communication
	

	//FBTCPIP		mTCPReceiver;
};

#endif /* __ORDEVICE_SKELETON_HARDWARE_H__ */
