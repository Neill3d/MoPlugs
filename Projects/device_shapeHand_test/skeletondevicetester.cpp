
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: skeletondevicetester.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#define WS_VERSION_REQUIRED 0x0101
#include <winsock.h>
#include <windows.h>
#include <fstream>
#include <iostream>

/*
 received data

 156,109,113,63,215,44,210,59,150,64,149,60,250,252,169,62,59,177,122,63,47,180,145,189,72,65,62,190,136,55,28,61,85,207,125,63,168,168,5,62,0,0,0,128,0,0,128,49,206,202,111,63,0,0,0,0,190,72,179,62,0,0,128,49,74,14,123,63,0,121,113,183,237,86,151,184,71,71,72,62,44,184,126,63,0,0,0,0,0,64,138,176,125,151,204,61,0,0,128,63,186,171,77,176,152,218,15,47,164,7,0,179,42,8,121,63,59,60,77,59,203,194,87,60,34,226,108,62,22,241,121,63,0,0,136,178,0,0,240,176,88,116,93,62,0,0,128,63,0,0,0,0,0,0,0,128,0,0,0,128,246,112,120,63,0,16,233,186,47,137,234,187,87,225,118,62,97,243,122,63,0,0,80,179,0,0,40,49,78,96,74,62,242,4,53,63,0,0,160,50,0,128,193,179,244,4,53,63,186,135,118,63,104,164,12,186,10,80,251,186,27,247,137,62,53,63,120,63,0,0,32,51,0,128,36,50,96,26,122,62,0,0,128,63,43,14,28,176,136,159,159,47,161,3,128,51,76,100,7,0,0

*/

//--- SDK include
//#include <fbsdk/fbsdk.h>

#include "DeviceBuffer.h"

int pluginPort = 3103;
int handShapePort = 3100;

std::ofstream gLog("C:\\logfile.txt", std::ios_base::app | std::ios_base::out);

unsigned char received_data[261] = {156,109,113,63,215,44,210,59,150,64,149,60,250,252,169,62,59,177,122,63,47,180,145,189,72,65,62,190,136,55,28,61,85,207,125,63,168,168,5,62,0,0,0,128,0,0,128,49,206,202,111,63,0,0,0,0,190,72,179,62,0,0,128,49,74,14,123,63,0,121,113,183,237,86,151,184,71,71,72,62,44,184,126,63,0,0,0,0,0,64,138,176,125,151,204,61,0,0,128,63,186,171,77,176,152,218,15,47,164,7,0,179,42,8,121,63,59,60,77,59,203,194,87,60,34,226,108,62,22,241,121,63,0,0,136,178,0,0,240,176,88,116,93,62,0,0,128,63,0,0,0,0,0,0,0,128,0,0,0,128,246,112,120,63,0,16,233,186,47,137,234,187,87,225,118,62,97,243,122,63,0,0,80,179,0,0,40,49,78,96,74,62,242,4,53,63,0,0,160,50,0,128,193,179,244,4,53,63,186,135,118,63,104,164,12,186,10,80,251,186,27,247,137,62,53,63,120,63,0,0,32,51,0,128,36,50,96,26,122,62,0,0,128,63,43,14,28,176,136,159,159,47,161,3,128,51,76,100,7,0,0};

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

int StartClient(int shapeHandPort)
{
	int lSocket;
    struct protoent* lP;
	
	
	lP = getprotobyname("tcp");
	lSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP /*IPPROTO_TCP/*lP->p_proto*/);
    /*
    if (lSocket)
    {
        bzero((char *)&serv_addr, sizeof(serv_addr));

		
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
		serv_addr.sin_port = htons( pPort );

		//connect to server
		if (connect( lSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr) ) < 0)
		{
			printf("connect failed with error: %d\n", WSAGetLastError() );
			closesocket(lSocket);
			return 0;
		}
	}
	*/

	if (lSocket)
	{
		sClientRequestPacket	packet;
		sClientRequestPacket::SetData( packet.data, pluginPort, 127, 0, 0, 1 );

		if (0 == ClientSend(lSocket, shapeHandPort, packet) )
		{
			closesocket(lSocket);
			return 0;
		}

	}

	return lSocket;
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

nsTime GetNanoSeconds()
{
	static double dmUnits = 0.0;

	LARGE_INTEGER	t;

	if(QueryPerformanceCounter(&t)) 
	{
		double	count;
		count = (double) t.QuadPart;
		if (!dmUnits) 
		{
			QueryPerformanceFrequency(&t);
			dmUnits = 1000000000.0 / (double) t.QuadPart;
		}
		return (unsigned __int64) (count * dmUnits);
	}
	return 0;
}

int recv_all(int sockfd, void *buf, size_t len, int flags)
{
    size_t toread = len;
    char  *bufptr = (char*) buf;

    while (toread > 0)
    {
        size_t rsz = recv(sockfd, bufptr, toread, flags);
        if (rsz <= 0)
            return rsz;  /* Error or other end closed cnnection */

        toread -= rsz;  /* Read less next time */
        bufptr += rsz;  /* Next buffer position to read into */
    }

    return len;
}

int main(int argc, char* argv[])
{
	printf("Server ...\n");

	hand_data *pData = (hand_data*) received_data;

    int ServerSoc=0;
	int ClientSoc=0;
	int StreamSoc=0;

	sClientRequestPacket	packet;
	sClientRequestPacket::SetData( packet.data, pluginPort, 127, 0, 0, 1 );

	// 1 Mb
	int bigBufferSize = 1 * 1024 * 1024;
	char	*bigBuffer = new char[bigBufferSize];
	memset( bigBuffer, 0, sizeof(char) * bigBufferSize );

	if (false == Initialize() )
		return 0;

    while (1)
    {
		static double ServerStartedOn = 0.0;
        if (!ServerSoc) 
        {
			ServerSoc = StartServer( pluginPort );
			if (ServerSoc) 
			{
				printf("Server started on port %d\n", pluginPort );
				gLog << "server started on port " << pluginPort << std::endl;
			}
			
			// send a client request
			ClientSend( ServerSoc, handShapePort, packet );

		} else
		{
			
			printf("Waiting for connection\n");
			gLog << "Waiting for connection\n";
			sockaddr_in	lClientAddr;
			int		lSize;
			
			lSize = sizeof(lClientAddr);
	        bzero((char *)&lClientAddr, sizeof(lClientAddr));

			lClientAddr.sin_family = AF_INET;
			lClientAddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
			lClientAddr.sin_port = htons( handShapePort );

			printf("Connection established\n");
			gLog << "Connection established\n";
			ServerStartedOn = (double)GetNanoSeconds();

			
			for (;;)
			{
				int bytes_received = recvfrom(ServerSoc, bigBuffer, bigBufferSize, 0, (struct sockaddr*) &lClientAddr, &lSize );
				if (bytes_received <= 0)
				{
					break;
				}
				else
				{
					gLog << "== New SAMPLE ==\n";

					for (int i=0; i<bytes_received; ++i)
					{
						unsigned char b = bigBuffer[i];
						gLog << (unsigned int) b;
						
						if (i < bytes_received-1)
							gLog << ",";
					}
					gLog << std::endl;
				}
				printf( "bytes received - %d\n", bytes_received );
				
				gLog << "bytes received - " << bytes_received << std::endl;

				Sleep( 1000/sSKDataBuffer::SIM_FPS );
			}
	
			printf("Connection closed, connection time = %f ms\n",(GetNanoSeconds()-ServerStartedOn)/1000000.0);
		}
	}
	
    if (ServerSoc) 
    {
        closesocket( ServerSoc );
	}
	if (ClientSoc)
	{
		closesocket( ClientSoc );
    }
	Cleanup();

	if (bigBuffer)
	{
		delete [] bigBuffer;
		bigBuffer = nullptr;
	}

	return 0;
}

