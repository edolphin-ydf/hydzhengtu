#ifndef __SOCKET_PACKET_H__
#define __SOCKET_PACKET_H__

#define MAX_PACKET_SIZE		32768

#pragma pack(1)

struct FS_API FS_PACKET
{
	FS_PACKET() : wHeader(0xFFFF), nSize(0), nID(0xFFFFFFFF)
	{
		
	}

	WORD wHeader;
	DWORD nSize;
	DWORD nID;
};

#pragma pack()

const int PID_SOCKET_DISCONNECT		=	-1;

#endif//__SOCKET_PACKET_H__
