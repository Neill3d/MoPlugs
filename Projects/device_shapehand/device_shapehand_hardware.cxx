
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: device_shapehand_hardware.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//--- Class declaration
#include "device_shapehand_hardware.h"
#include <winsock.h>

bool Cleanup()
{
    if (WSACleanup()) 
	{
//         GetErrorStr();
        WSACleanup();
        
        return false;
    }
    return true;
}
bool Initialize()
{
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2,2), &wsadata)) 
	{
//          GetErrorStr();
        Cleanup();
        return false;
    }
    return true;
}

void bzero(char *b, size_t length)
{
    memset( b,0,length );
}

int ClientSend(int socket, int shapeHandPort, sClientRequestPacket &packet)
{
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	serv_addr.sin_port = htons( shapeHandPort );

	int nbytes_send = sendto( socket, 
		(const char*) &packet.data,
		6, 
		0, 
		(struct sockaddr*)&serv_addr, 
		sizeof(serv_addr) );

	if (nbytes_send != sizeof(packet) )
	{
		printf( " > error sending a request packet\n" );
		printf("sendto failed with error: %d\n", WSAGetLastError() );
		return 0;
	}
	if (nbytes_send == SOCKET_ERROR )
	{
		printf (" > socket error while sending a request\n" );
		return 0;
	}

	return 1;
}

int StartServer(int pluginPort)
{
    int lSocket;
    struct protoent* lP;
    struct sockaddr_in  lSin;

    lP = getprotobyname("tcp");
    
	lSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP /*lP->p_proto*/);
    
    if (lSocket)
    {
        bzero((char *)&lSin, sizeof(lSin));

		lSin.sin_family = AF_INET;
		lSin.sin_port = htons(pluginPort);
		lSin.sin_addr.s_addr = 0L;

		//Bind socket
		if( bind(lSocket, (struct sockaddr*)&lSin, sizeof(lSin)) < 0 )
		{
			printf("failed to bind a socket\n");
			closesocket(lSocket);
			return 0;
		}

		DWORD nonBlocking = 1;
        if ( ioctlsocket( lSocket, FIONBIO, &nonBlocking ) != 0 )
        {
            printf( "failed to set non-blocking socket\n" );
			closesocket(lSocket);
            return 0;
        }
	}

	return lSocket;
}

/************************************************
 *	Constructor.
 ************************************************/
DeviceShapeHandHardware::DeviceShapeHandHardware() :
    mChannelCount( 0 ),
	mPassCount( 0 ),
    mNetworkUse( true ),
	mNetworkPortPlugin( 3103 ),
	mNetworkPortShapeHand( 3100 ),
    mSocket( 0 )	
{
	mNetworkAddress	= "127.0.0.1";

	// 1 Mb cache
	mBigBufferSize = 1 * 1024 * 1024;
	mBigBuffer = new char[mBigBufferSize];
	memset(mBigBuffer, 0, sizeof(char) * mBigBufferSize );

	Initialize();
}


/************************************************
 *	Destructor.
 ************************************************/
DeviceShapeHandHardware::~DeviceShapeHandHardware()
{
	if (mBigBuffer)
	{
		delete [] mBigBuffer;
		mBigBuffer = nullptr;
	}

	Cleanup();
}


/************************************************
 *	Open device communications.
 ************************************************/
bool DeviceShapeHandHardware::Open()
{
	return true;
}


/************************************************
 *	Get device setup information.
 ************************************************/
bool DeviceShapeHandHardware::GetSetupInfo()
{
	//This should be read from hardware.
	//Here we used simple script to generate this data from skeleton:
/*
from pyfbsdk import *

lModels = FBModelList()
FBGetSelectedModels(lModels)

def SetupHierarchy(pModel, pParentIdx, pNextIndex):
    lIdx = pNextIndex
    pNextIndex += 1
    lT = FBVector3d()
    lR = FBVector3d()
    pModel.GetVector(lT,FBModelTransformationType.kModelTranslation)
    pModel.GetVector(lR,FBModelTransformationType.kModelRotation)
    print '//'+str(lIdx)+':'+pModel.Name
    print 'mChannel['+str(lIdx)+'].mName = "' + pModel.Name + '";'
    print 'mChannel['+str(lIdx)+'].mParentChannel = ' +  str(pParentIdx) + ';'
    print 'mChannel['+str(lIdx)+'].mT[0] = ' +  str(lT[0]) + ';'
    print 'mChannel['+str(lIdx)+'].mT[1] = ' +  str(lT[1]) + ';'
    print 'mChannel['+str(lIdx)+'].mT[2] = ' +  str(lT[2]) + ';'
    print 'mChannel['+str(lIdx)+'].mR[0] = ' +  str(lR[0]) + ';'
    print 'mChannel['+str(lIdx)+'].mR[1] = ' +  str(lR[1]) + ';'
    print 'mChannel['+str(lIdx)+'].mR[2] = ' +  str(lR[2]) + ';'
    print ''
    for lChild in pModel.Children:
        pNextIndex = SetupHierarchy(lChild,lIdx,pNextIndex)
    return pNextIndex

if len(lModels) == 1:
    SetupHierarchy(lModels[0],-1,0)
*/
	mChannelCount	= 21;

	for (int i=0; i<mChannelCount; ++i)
		mChannel[i].mR[3] = 1.0;

	//0:palm
	mChannel[0].mName = "palm";
	mChannel[0].mParentChannel = -1;
	mChannel[0].mT[0] = 0.0;
	mChannel[0].mT[1] = 0.0;
	mChannel[0].mT[2] = 0.0;
	mChannel[0].mR[0] = 0.0;
	mChannel[0].mR[1] = -0.0;
	mChannel[0].mR[2] = 0.0;

	//1:thumbA
	mChannel[1].mName = "thumbA";
	mChannel[1].mParentChannel = 0;
	mChannel[1].mT[0] = 2.47000002861;
	mChannel[1].mT[1] = -2.20000004768;
	mChannel[1].mT[2] = 0.0;
	mChannel[1].mR[0] = 0.0;
	mChannel[1].mR[1] = -0.0;
	mChannel[1].mR[2] = 0.0;

	//2:thumbB
	mChannel[2].mName = "thumbB";
	mChannel[2].mParentChannel = 1;
	mChannel[2].mT[0] = 2.47000002861;
	mChannel[2].mT[1] = -2.20000004768;
	mChannel[2].mT[2] = 6.59999990463;
	mChannel[2].mR[0] = 0.0;
	mChannel[2].mR[1] = -0.0;
	mChannel[2].mR[2] = 0.0;

	//3:thumbC
	mChannel[3].mName = "thumbC";
	mChannel[3].mParentChannel = 2;
	mChannel[3].mT[0] = 2.47000002861;
	mChannel[3].mT[1] = -2.20000004768;
	mChannel[3].mT[2] = 9.89999985695;
	mChannel[3].mR[0] = 0.0;
	mChannel[3].mR[1] = -0.0;
	mChannel[3].mR[2] = 0.0;

	//4:thumbC_End
	mChannel[4].mName = "thumbC_End";
	mChannel[4].mParentChannel = 3;
	mChannel[4].mT[0] = 2.47000002861;
	mChannel[4].mT[1] = -2.20000004768;
	mChannel[4].mT[2] = 12.6999998093;
	mChannel[4].mR[0] = 0.0;
	mChannel[4].mR[1] = -0.0;
	mChannel[4].mR[2] = 0.0;

	//5:indexA
	mChannel[5].mName = "indexA";
	mChannel[5].mParentChannel = 0;
	mChannel[5].mT[0] = 2.47000002861;
	mChannel[5].mT[1] = 0.0;
	mChannel[5].mT[2] = 10.1000003815;
	mChannel[5].mR[0] = 0.0;
	mChannel[5].mR[1] = -0.0;
	mChannel[5].mR[2] = 0.0;

	//6:indexB
	mChannel[6].mName = "indexB";
	mChannel[6].mParentChannel = 5;
	mChannel[6].mT[0] = 2.47000002861;
	mChannel[6].mT[1] = 0.0;
	mChannel[6].mT[2] = 14.5000004768;
	mChannel[6].mR[0] = 0.0;
	mChannel[6].mR[1] = -0.0;
	mChannel[6].mR[2] = 0.0;

	//7:indexC
	mChannel[7].mName = "indexC";
	mChannel[7].mParentChannel = 6;
	mChannel[7].mT[0] = 2.47000002861;
	mChannel[7].mT[1] = 0.0;
	mChannel[7].mT[2] = 17.1000003815;
	mChannel[7].mR[0] = 0.0;
	mChannel[7].mR[1] = -0.0;
	mChannel[7].mR[2] = 0.0;

	//8:indexC_End
	mChannel[8].mName = "indexC_End";
	mChannel[8].mParentChannel = 7;
	mChannel[8].mT[0] = 2.47000002861;
	mChannel[8].mT[1] = 0.0;
	mChannel[8].mT[2] = 19.5000004768;
	mChannel[8].mR[0] = 0.0;
	mChannel[8].mR[1] = -0.0;
	mChannel[8].mR[2] = 0.0;

	//9:middleA
	mChannel[9].mName = "middleA";
	mChannel[9].mParentChannel = 0;
	mChannel[9].mT[0] = 0.550000011921;
	mChannel[9].mT[1] = 0.0;
	mChannel[9].mT[2] = 9.39999961853;
	mChannel[9].mR[0] = 0.0;
	mChannel[9].mR[1] = -0.0;
	mChannel[9].mR[2] = 0.0;

	//10:middleB
	mChannel[10].mName = "middleB";
	mChannel[10].mParentChannel = 9;
	mChannel[10].mT[0] = 0.550000011921;
	mChannel[10].mT[1] = 0.0;
	mChannel[10].mT[2] = 14.0999994278;
	mChannel[10].mR[0] = 0.0;
	mChannel[10].mR[1] = -0.0;
	mChannel[10].mR[2] = 0.0;

	//11:middleC
	mChannel[11].mName = "middleC";
	mChannel[11].mParentChannel = 10;
	mChannel[11].mT[0] = 0.550000011921;
	mChannel[11].mT[1] = 0.0;
	mChannel[11].mT[2] = 16.9999995232;
	mChannel[11].mR[0] = 0.0;
	mChannel[11].mR[1] = -0.0;
	mChannel[11].mR[2] = 0.0;

	//12:middleC_End
	mChannel[12].mName = "middleC_End";
	mChannel[12].mParentChannel = 11;
	mChannel[12].mT[0] = 0.550000011921;
	mChannel[12].mT[1] = 0.0;
	mChannel[12].mT[2] = 19.5999994278;
	mChannel[12].mR[0] = 0.0;
	mChannel[12].mR[1] = -0.0;
	mChannel[12].mR[2] = 0.0;

	//13:ringA
	mChannel[13].mName = "ringA";
	mChannel[13].mParentChannel = 0;
	mChannel[13].mT[0] = -1.70000004768;
	mChannel[13].mT[1] = 0.0;
	mChannel[13].mT[2] = 8.89999961853;
	mChannel[13].mR[0] = 0.0;
	mChannel[13].mR[1] = -0.0;
	mChannel[13].mR[2] = 0.0;

	//14:ringB
	mChannel[14].mName = "ringB";
	mChannel[14].mParentChannel = 13;
	mChannel[14].mT[0] = -1.70000004768;
	mChannel[14].mT[1] = 0.0;
	mChannel[14].mT[2] = 13.1999998093;
	mChannel[14].mR[0] = 0.0;
	mChannel[14].mR[1] = -0.0;
	mChannel[14].mR[2] = 0.0;

	//15:ringC
	mChannel[15].mName = "ringC";
	mChannel[15].mParentChannel = 14;
	mChannel[15].mT[0] = -1.70000004768;
	mChannel[15].mT[1] = 0.0;
	mChannel[15].mT[2] = 15.7999997139;
	mChannel[15].mR[0] = 0.0;
	mChannel[15].mR[1] = -0.0;
	mChannel[15].mR[2] = 0.0;

	//16:ringC_End
	mChannel[16].mName = "ringC_End";
	mChannel[16].mParentChannel = 15;
	mChannel[16].mT[0] = -1.70000004768;
	mChannel[16].mT[1] = 0.0;
	mChannel[16].mT[2] = 18.1999998093;
	mChannel[16].mR[0] = 0.0;
	mChannel[16].mR[1] = -0.0;
	mChannel[16].mR[2] = 0.0;

	//17:littleA
	mChannel[17].mName = "littleA";
	mChannel[17].mParentChannel = 0;
	mChannel[17].mT[0] = -3.20000004768;
	mChannel[17].mT[1] = 0.0;
	mChannel[17].mT[2] = 8.0;
	mChannel[17].mR[0] = 0.0;
	mChannel[17].mR[1] = -0.0;
	mChannel[17].mR[2] = 0.0;

	//18:littleB
	mChannel[18].mName = "littleB";
	mChannel[18].mParentChannel = 17;
	mChannel[18].mT[0] = -3.20000004768;
	mChannel[18].mT[1] = 0.0;
	mChannel[18].mT[2] = 11.5999999046;
	mChannel[18].mR[0] = 0.0;
	mChannel[18].mR[1] = -0.0;
	mChannel[18].mR[2] = 0.0;

	//19:littleC
	mChannel[19].mName = "littleC";
	mChannel[19].mParentChannel = 18;
	mChannel[19].mT[0] = -3.20000004768;
	mChannel[19].mT[1] = 0.0;
	mChannel[19].mT[2] = 13.4999998808;
	mChannel[19].mR[0] = 0.0;
	mChannel[19].mR[1] = -0.0;
	mChannel[19].mR[2] = 0.0;

	//20:littleC_End
	mChannel[20].mName = "littleC_End";
	mChannel[20].mParentChannel = 19;
	mChannel[20].mT[0] = -3.20000004768;
	mChannel[20].mT[1] = 0.0;
	mChannel[20].mT[2] = 15.6999999285;
	mChannel[20].mR[0] = 0.0;
	mChannel[20].mR[1] = -0.0;
	mChannel[20].mR[2] = 0.0;

	for(int i = 0; i < mChannelCount; i++)
	{
		memcpy(mChannel[i].mDefaultT,mChannel[i].mT, sizeof(double)*3);
		memcpy(mChannel[i].mDefaultR,mChannel[i].mR, sizeof(double)*3);
	}
	
	mSkDataBuffer.SetupBuffer();

	return true;
}


/************************************************
 *	Close device communications.
 ************************************************/
bool DeviceShapeHandHardware::Close()
{
	// TODO: Close network connection to server
	if (mSocket) closesocket(mSocket);
	mSocket = 0;
	return true;
}

/************************************************
 *	Take current DataBuffer time as a time offset
 ************************************************/
void DeviceShapeHandHardware::ResetPacketTimeOffset(FBTime &pTime)
{
	mPacketTimeOffset = mSkDataBuffer.mTime;
	mStartRecordingTime = pTime;
}

/************************************************
 *	Fetch a data packet from the device.
 ************************************************/
struct hand_data
{
	struct quat
	{
		float w;
		float x;
		float y;
		float z;
	};

	quat	joints[16];
};

bool DeviceShapeHandHardware::FetchDataPacket(FBTime &pTime)
{
	if (mSocket)
	{
		sockaddr_in	lClientAddr;
		int		lSize;
			
		lSize = sizeof(lClientAddr);
	    bzero((char *)&lClientAddr, sizeof(lClientAddr));

		lClientAddr.sin_family = AF_INET;
		lClientAddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
		lClientAddr.sin_port = htons( mNetworkPortShapeHand );

		int bytes_received = recvfrom(mSocket, mBigBuffer, mBigBufferSize, 0, (struct sockaddr*) &lClientAddr, &lSize );
		FBTrace( "bytes received - %d\n", bytes_received );
		
		if (bytes_received != 261)
		{
			mLastSimulationTime = mSystem.SystemTime;
			return false;
		}

		// 261 bytes packet from shapeHand
		if (bytes_received == 261)
		{
			// we have received a correct packet

			mLastSimulationTime = mSystem.SystemTime;
			mLastSimulationTime.SetMilliSeconds(mLastSimulationTime.GetMilliSeconds()+(1000/sSKDataBuffer::SIM_FPS));

			// Simulate Skeleton 
			mSkDataBuffer.Simulate((mLastSimulationTime.GetMilliSeconds())*1000000);

			// Copy data
			hand_data *pData = (hand_data*) mBigBuffer;

			SkeletonNodeInfo* lIter;
			int inputIndex=0;
			for(int i = 0; i < GetChannelCount(); i++)
			{
				// skip end joints, not need to write rotation for them
				if (i==4 || i==8 || i==12 || i==16 || i==20)
					continue;

				lIter = &mChannel[i];
				//memcpy(lIter->mT,mSkDataBuffer.mChannel[i].mT,sizeof(lIter->mT));
				//memcpy(lIter->mR,mSkDataBuffer.mChannel[i].mR,sizeof(lIter->mR));

				hand_data::quat *q = &pData->joints[inputIndex];
				lIter->mR[0] = q->x;
				lIter->mR[1] = q->y;
				lIter->mR[2] = q->z;
				lIter->mR[3] = q->w;

				inputIndex += 1;
				if (inputIndex >= 16)
					break;
			}
		}

	} else
	// Otherwise use a simulated source
	{
		mPassCount++;
		if(mPassCount % 2==0)
		{
			mLastSimulationTime = mSystem.SystemTime;
			return false; // Nothing else to read.
		}

		mLastSimulationTime.SetMilliSeconds(mLastSimulationTime.GetMilliSeconds()+(1000/sSKDataBuffer::SIM_FPS));

		// Simulate Skeleton 
		mSkDataBuffer.Simulate((mLastSimulationTime.GetMilliSeconds())*1000000);

		// Copy data
		SkeletonNodeInfo* lIter;
		for(int i = 0; i < GetChannelCount(); i++)
		{
			lIter = &mChannel[i];
			memcpy(lIter->mT,mSkDataBuffer.mChannel[i].mT,sizeof(lIter->mT));
			memcpy(lIter->mR,mSkDataBuffer.mChannel[i].mR,sizeof(lIter->mR));
		}
	}

	// Update packet relative time
	pTime.SetMilliSeconds((mSkDataBuffer.mTime-mPacketTimeOffset)/1000000);
	pTime += mStartRecordingTime;
	return true;
}

/************************************************
 *	Start data streaming from device.
 ************************************************/


bool DeviceShapeHandHardware::StartDataStream()
{
	// TODO: ask server to start sending data

	if (mNetworkUse)
	{
		mSocket = StartServer(mNetworkPortPlugin);
		if (mSocket)
		{
			
			sClientRequestPacket packet;
			sClientRequestPacket::SetData( packet.data, mNetworkPortPlugin );

			ClientSend( mSocket, mNetworkPortShapeHand, packet );
		}
		FBTrace ("mSocket - %d\n", mSocket );
		return mSocket!=0;
	}
	return true;
}

/************************************************
 *	Stop data streaming from device.
 ************************************************/
bool DeviceShapeHandHardware::StopDataStream()
{
	// TODO: ask server to stop sending data
	if (mSocket) closesocket(mSocket);
	mSocket = 0;
	
	return true;
}
