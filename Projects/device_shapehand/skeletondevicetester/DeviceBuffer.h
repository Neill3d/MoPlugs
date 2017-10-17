
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: DeviceBuffer.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#if defined( WIN32 )
typedef unsigned __int64 nsTime;
#else
typedef unsigned long long nsTime;
#endif

/*
 This format is available for use with Measurand's plug-ins to Autodesk's
MotionsBuilder software, etc.
 The coordinate system has the X-axis pointing to the person's left,
 the Y-axis pointing upwards, and the Z-axis pointing forwards.
  The base position is defined for a person standing straight up with arms at sides,
 and head straight up. The data is organized as one line of text per sample.

*/

/*

Hips
Left shoulder
Left collarbone
Left elbow
Left upperarm
Left wrist
Left forearm
Left knuckle
Left hand
Left fingers
Right shoulder
Right collarbone
Right elbow
Right upperarm
Right wrist
Right forearm
Right knuckle
Right hand
Right fingers

Center of head
Head

Left knee
Left shin
Left ankle
Left knuckle
Left Toe

Right knee
Right shin
Right ankle
Right knuckle
Right Toe

Chest

NOTE! next elements have only rotation channels

Left Hand
Left Thumb Proximal Bone
Left Thumb Middle Bone
Left Thumb Distal Bone
Left Index Proximal Bone
Left Index Middle Bone
Left Index Distal Bone
Left Middle Proximal Bone
Left Middle Middle Bone
Left Middle Distal Bone
Left Ring Proximal Bone
Left Ring Middle Bone
Left Ring Distal Bone
Left Small Proximal Bone
Left Small Middle Bone
Left Small Distal Bone

Right Hand
Right Thumb Proximal Bone
Right Thumb Middle Bone
Right Thumb Distal Bone
Right Index Proximal Bone
Right Index Middle Bone
Right Index Distal Bone
Right Middle Proximal Bone
Right Middle Middle Bone
Right Middle Distal Bone
Right Ring Proximal Bone
Right Ring Middle Bone
Right Ring Distal Bone
Right Small Proximal Bone
Right Small Middle Bone
Right Small Distal Bone

Time Stamp (ms)


*/

struct sClientRequestPacket
{
	unsigned char data[6];

	static unsigned int GetLength()
	{
		return 6;
	}
	static void SetData( unsigned char *data, int port, unsigned char addr1=127, unsigned char addr2=0, unsigned char addr3=0, unsigned char addr4=1 )
	{
		data[0] = addr1;
		data[1] = addr2;
		data[2] = addr3;
		data[3] = addr4;
		
		// port
		data[4] = port / 256;
		data[5] = port % 256;
	}
};

struct sSKDataBuffer
{
	// Port number to use for server
	static const int PORTNUMBER		= 3100;		// Port number for communication
	static const int PORTNUMBER_RECEIVER = 3110;
	static const int SIM_FPS		= 80;		// Tested for 30,60,120
	static const int ChannelCount	= 21;	// 24 base (for characterize) and 32 only hands (5 finges 3 bones each and left/right hand bone)

	nsTime 	mTime;				//!< Time Stamp in ms
	int		mCounter;
	struct  
	{
		double		mT[3];
		double		mR[4];		// Quaternions are of the form: Q = w + xi + yj + zk
	} mChannel[ChannelCount];

	sSKDataBuffer()
	{
		mTime		  = 0;
		mCounter	  = 0;
		SetupBuffer();
	}

	void Simulate(nsTime pTime)
	{
		mTime = pTime;

		// Send last data + 0.5 degree global rotation in the Y direction

		mCounter++;
	}

	void SetupBuffer()
	{
		for (int i=0; i<ChannelCount; ++i)
			mChannel[i].mR[3] = 1.0;

		//0:palm
		mChannel[0].mT[0] = 0.0;
		mChannel[0].mT[1] = 0.0;
		mChannel[0].mT[2] = 0.0;
		mChannel[0].mR[0] = 0.0;
		mChannel[0].mR[1] = -0.0;
		mChannel[0].mR[2] = 0.0;

		//1:thumbA
		mChannel[1].mT[0] = 2.47000002861;
		mChannel[1].mT[1] = -2.20000004768;
		mChannel[1].mT[2] = 0.0;
		mChannel[1].mR[0] = 0.0;
		mChannel[1].mR[1] = -0.0;
		mChannel[1].mR[2] = 0.0;

		//2:thumbB
		mChannel[2].mT[0] = 2.47000002861;
		mChannel[2].mT[1] = -2.20000004768;
		mChannel[2].mT[2] = 6.59999990463;
		mChannel[2].mR[0] = 0.0;
		mChannel[2].mR[1] = -0.0;
		mChannel[2].mR[2] = 0.0;

		//3:thumbC
		mChannel[3].mT[0] = 2.47000002861;
		mChannel[3].mT[1] = -2.20000004768;
		mChannel[3].mT[2] = 9.89999985695;
		mChannel[3].mR[0] = 0.0;
		mChannel[3].mR[1] = -0.0;
		mChannel[3].mR[2] = 0.0;

		//4:thumbC_End
		mChannel[4].mT[0] = 2.47000002861;
		mChannel[4].mT[1] = -2.20000004768;
		mChannel[4].mT[2] = 12.6999998093;
		mChannel[4].mR[0] = 0.0;
		mChannel[4].mR[1] = -0.0;
		mChannel[4].mR[2] = 0.0;

		//5:indexA
		mChannel[5].mT[0] = 2.47000002861;
		mChannel[5].mT[1] = 0.0;
		mChannel[5].mT[2] = 10.1000003815;
		mChannel[5].mR[0] = 0.0;
		mChannel[5].mR[1] = -0.0;
		mChannel[5].mR[2] = 0.0;

		//6:indexB
		mChannel[6].mT[0] = 2.47000002861;
		mChannel[6].mT[1] = 0.0;
		mChannel[6].mT[2] = 14.5000004768;
		mChannel[6].mR[0] = 0.0;
		mChannel[6].mR[1] = -0.0;
		mChannel[6].mR[2] = 0.0;

		//7:indexC
		mChannel[7].mT[0] = 2.47000002861;
		mChannel[7].mT[1] = 0.0;
		mChannel[7].mT[2] = 17.1000003815;
		mChannel[7].mR[0] = 0.0;
		mChannel[7].mR[1] = -0.0;
		mChannel[7].mR[2] = 0.0;

		//8:indexC_End
		mChannel[8].mT[0] = 2.47000002861;
		mChannel[8].mT[1] = 0.0;
		mChannel[8].mT[2] = 19.5000004768;
		mChannel[8].mR[0] = 0.0;
		mChannel[8].mR[1] = -0.0;
		mChannel[8].mR[2] = 0.0;

		//9:middleA
		mChannel[9].mT[0] = 0.550000011921;
		mChannel[9].mT[1] = 0.0;
		mChannel[9].mT[2] = 9.39999961853;
		mChannel[9].mR[0] = 0.0;
		mChannel[9].mR[1] = -0.0;
		mChannel[9].mR[2] = 0.0;

		//10:middleB
		mChannel[10].mT[0] = 0.550000011921;
		mChannel[10].mT[1] = 0.0;
		mChannel[10].mT[2] = 14.0999994278;
		mChannel[10].mR[0] = 0.0;
		mChannel[10].mR[1] = -0.0;
		mChannel[10].mR[2] = 0.0;

		//11:middleC
		mChannel[11].mT[0] = 0.550000011921;
		mChannel[11].mT[1] = 0.0;
		mChannel[11].mT[2] = 16.9999995232;
		mChannel[11].mR[0] = 0.0;
		mChannel[11].mR[1] = -0.0;
		mChannel[11].mR[2] = 0.0;

		//12:middleC_End
		mChannel[12].mT[0] = 0.550000011921;
		mChannel[12].mT[1] = 0.0;
		mChannel[12].mT[2] = 19.5999994278;
		mChannel[12].mR[0] = 0.0;
		mChannel[12].mR[1] = -0.0;
		mChannel[12].mR[2] = 0.0;

		//13:ringA
		mChannel[13].mT[0] = -1.70000004768;
		mChannel[13].mT[1] = 0.0;
		mChannel[13].mT[2] = 8.89999961853;
		mChannel[13].mR[0] = 0.0;
		mChannel[13].mR[1] = -0.0;
		mChannel[13].mR[2] = 0.0;

		//14:ringB
		mChannel[14].mT[0] = -1.70000004768;
		mChannel[14].mT[1] = 0.0;
		mChannel[14].mT[2] = 13.1999998093;
		mChannel[14].mR[0] = 0.0;
		mChannel[14].mR[1] = -0.0;
		mChannel[14].mR[2] = 0.0;

		//15:ringC
		mChannel[15].mT[0] = -1.70000004768;
		mChannel[15].mT[1] = 0.0;
		mChannel[15].mT[2] = 15.7999997139;
		mChannel[15].mR[0] = 0.0;
		mChannel[15].mR[1] = -0.0;
		mChannel[15].mR[2] = 0.0;

		//16:ringC_End
		mChannel[16].mT[0] = -1.70000004768;
		mChannel[16].mT[1] = 0.0;
		mChannel[16].mT[2] = 18.1999998093;
		mChannel[16].mR[0] = 0.0;
		mChannel[16].mR[1] = -0.0;
		mChannel[16].mR[2] = 0.0;

		//17:littleA
		mChannel[17].mT[0] = -3.20000004768;
		mChannel[17].mT[1] = 0.0;
		mChannel[17].mT[2] = 8.0;
		mChannel[17].mR[0] = 0.0;
		mChannel[17].mR[1] = -0.0;
		mChannel[17].mR[2] = 0.0;

		//18:littleB
		mChannel[18].mT[0] = -3.20000004768;
		mChannel[18].mT[1] = 0.0;
		mChannel[18].mT[2] = 11.5999999046;
		mChannel[18].mR[0] = 0.0;
		mChannel[18].mR[1] = -0.0;
		mChannel[18].mR[2] = 0.0;

		//19:littleC
		mChannel[19].mT[0] = -3.20000004768;
		mChannel[19].mT[1] = 0.0;
		mChannel[19].mT[2] = 13.4999998808;
		mChannel[19].mR[0] = 0.0;
		mChannel[19].mR[1] = -0.0;
		mChannel[19].mR[2] = 0.0;

		//20:littleC_End
		mChannel[20].mT[0] = -3.20000004768;
		mChannel[20].mT[1] = 0.0;
		mChannel[20].mT[2] = 15.6999999285;
		mChannel[20].mR[0] = 0.0;
		mChannel[20].mR[1] = -0.0;
		mChannel[20].mR[2] = 0.0;
	}


};
