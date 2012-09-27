#include "stdafx.h"
#include "MolSocket.h"

#include "MolSocketOps.h"
#include "MolSocketMgrWin32.h"
#include "MolListenSocketWin32.h"
#include "../GameFrameManager.h"

#pragma pack(push, 1)
typedef struct
{
	uint16 opcode;
	uint32 size;
}logonpacket;
#pragma pack(pop)

initialiseSingleton(SocketGarbageCollector);

MolNetworkUpdate *m_NetworkUpdate = NULL;

/** 
 * 构造函数
 *
 * @param fd socket的文件描述
 * @param sendbuffersize 发送缓冲区的大小
 * @param recvbuffersize 接收缓冲区的大小
 */
Socket::Socket(SOCKET fd,uint32 sendbuffersize,uint32 recvbuffersize)
{
	readBuffer.Allocate(recvbuffersize);
	writeBuffer.Allocate(sendbuffersize);

	//m_writeLock = 0;
	m_completionPort = 0;

	m_BytesSent = 0;
	m_BytesRecieved = 0;
	m_fd = fd;
	m_connected.SetVal(false);
	m_deleted.SetVal(false);
	m_writeLock.SetVal(false);
	removedFromSet = false;

	if(m_fd == 0)
	{
		m_fd = SocketOps::CreateTCPFileDescriptor();
	}

	m_heartJitter = (uint32)time(NULL);
}

/** 
 * 析构函数
 */
Socket::~Socket()
{

}

/** 
 * 连接指定的机器
 *
 * @param Address 要连接的服务器的网络地址
 * @param Port 要连接的服务器的端口
 *
 * @return 如果服务器连接成功返回真，否则返回假
 */
bool Socket::Connect(const char * Address,uint32 Port)
{
	struct hostent * ci = gethostbyname(Address);
	if(ci == 0)
		return false;

	m_client.sin_family = ci->h_addrtype;
	m_client.sin_port = ntohs((u_short)Port);
	memcpy(&m_client.sin_addr.s_addr,ci->h_addr_list[0],ci->h_length);

	SocketOps::Blocking(m_fd);
	if(connect(m_fd,(const sockaddr*)&m_client,sizeof(m_client)) == -1)
		return false;

	m_completionPort = sSocketMgr.GetCompletionPort();

    _OnConnect();

	return true;
}

/** 
 * 接收已经设置的文件描述符
 *
 * @param address 要接收的客户端的地址
 */
void Socket::Accept(sockaddr_in * address)
{
	memcpy(&m_client,address,sizeof(*address));
	_OnConnect();
}

/** 
 * 当连接打开时调用
 */
void Socket::_OnConnect()
{
	SocketOps::Nonblocking(m_fd);
	SocketOps::DisableBuffering(m_fd);

	m_connected.SetVal(true);

	AssignToCompletionPort();
	SetupReadEvent();

	sSocketMgr.AddSocket(this);	
}

/** 
 * 发送数据
 *
 * @param out 要发送的数据
 *
 * @return 如果数据发送成功返回真,否则返回假
 */
bool Socket::Send(CMolMessageOut &out)
{
	if(out.getLength() <= 0 || out.getLength() > MOL_REV_BUFFER_SIZE)
		return false;

	if(IsConnected() == false || IsDeleted() == true) return false;

	//int pSendCount = 5;
	bool rv = true;

	//while(pSendCount > 0)
	//{
	try
	{
		if(BurstBegin())
		{
			logonpacket header;
			header.opcode = 100;
			header.size = out.getLength();

			rv = BurstSend((uint8*)&header,sizeof(logonpacket));
			if(rv)
			{
				int uSendSize = out.getLength();
				BYTE *bufferData = new BYTE[uSendSize];
				memcpy(bufferData,(BYTE*)out.getData(),uSendSize);
				Encrypto(bufferData,uSendSize);

				rv = BurstSend(bufferData,uSendSize);

				delete [] bufferData;
				bufferData = NULL;
			}

			if(rv)
				BurstPush();
			BurstEnd();
		}
	}
	catch (std::exception e)
	{
		BurstEnd();
		TRACE("发送数据异常:%s\n",e.what());
	}
	//	if(rv) break;
	//	else 
	//	{
	//		Sleep(10);
	//		pSendCount-=1;
	//	}
	//}

	//Sleep(1);

	return rv;
}

/** 
 * 锁定发送互斥锁,发送数据，解锁互斥锁
 *
 * @param Bytes 要发送的数据
 * @param Size 要发送的数据的大小
 *
 * @return 如果数据发送成功返回真，否则返回假
 */
bool Socket::Send(const uint8 * Bytes,uint32 Size)
{
	bool rv;

	if(BurstBegin())
	{
		rv = BurstSend(Bytes,Size);
		if(rv)
			BurstPush();
		BurstEnd();
	}

	return rv;
}

/** 
 * 爆裂系统 - 添加数据到发送缓冲区
 *
 * @param Bytes 要发送的数据
 * @param Size 要发送的数据的大小
 *
 * @return 如果数据发送成功返回真，否则返回假
 */
bool Socket::BurstSend(const uint8 * Bytes,uint32 Size)
{
	return writeBuffer.Write(Bytes,Size);
}

std::string Socket::GetRemoteIP()
{
	char* ip = (char*)inet_ntoa( m_client.sin_addr );
	if( ip != NULL )
		return std::string( ip );
	else
		return std::string( "noip" );
}

void Socket::Disconnect(bool isDel)
{
	//if returns false it means it's already disconnected
	if(!m_connected.SetVal(false))
		return;

	if(isDel) sSocketMgr.RemoveSocket(this);

	if(!IsDeleted()) 
		Delete();
}

void Socket::Delete()
{
	//if returns true it means it's already delete
	if(m_deleted.SetVal(true))
		return;

	if(IsConnected()) Disconnect();

	sSocketGarbageCollector.QueueSocket(this);
}

/** 
 * 构造函数
 *
 * @param fd socket的文件描述
 */
NetClient::NetClient(SOCKET fd)
: Socket(fd, 32768, 10240)
{
	remaining=0;
	opcode = 0;
}

/** 
 * 析构函数
 */
NetClient::~NetClient()
{

}

/**
* 加密数据
*
* @param data 要加密的数据
* @param length 要加密的数据的长度
*/
void Socket::Encrypto(unsigned char *data,unsigned long length)
{
	if(data == NULL || length <= 0) return;

	unsigned char pKeyList[] = {76,225,112,120,103,92,84,105,8,12,238,122,206,165,222,21,117,217,106,214,239,66,32,3,85,67,224,180,
		240,233,236,171,89,13,52,109,123,99,132,213,15,137,226,69,231,228,60,28,190,193,74,144,81,53,17,101,230,207,79,93,88,36,30,
		141,115,110,20,169,173,243,219,80,72,184,125,175,174,139,95,24,148,48,113,182,50,223,61,118,140,14,78,181,16,4,121,73,187,
		147,168,9,116,23,63,216,215,244,232,59,195,154,200,55,62,220,75,161,196,68,159,6,167,40,45,0,22,155,64,127,27,237,192,212,58,
		26,98,201,41,209,179,130,211,208,82,152,172,7,35,205,107,46,33,146,185,87,199,25,2,77,39,156,164,102,194,163,241,96,166,10,11,
		235,198,157,229,126,94,56,189,134,5,153,133,242,1,31,119,37,145,47,178,18,177,176,86,129,197,65,210,111,54,43,70,188,128,90,
		227,162,104,186,108,114,158,142,57,218,151,202,170,234,150,100,183,71,135,160,42,203,49,97,138,91,124,29,149,83,44,51,19,143,
		131,38,34,136,221,191,204,245,246,247,248,249,250,251,252,253,254,255};

	for(int i=0;i<(int)length;i++)
	{
		data[i] = pKeyList[data[i]];
	}
}

/**
* 解密数据
*
* @param data 要解密的数据
* @param length 要解密的数据的长度
*/
void Socket::Decrypto(unsigned char *data,unsigned long length)
{
	if(data == NULL || length <= 0) return;

	unsigned char pKeyList[] = {123,182,156,23,93,178,119,145,8,99,167,168,9,33,89,40,92,54,189,236,66,15,124,101,79,155,133,128,47,231,
		62,183,22,150,240,146,61,185,239,158,121,136,224,199,234,122,149,187,81,226,84,235,34,53,198,111,175,212,132,107,46,86,112,102,
		126,195,21,25,117,43,200,221,72,95,50,114,0,157,90,58,71,52,142,233,6,24,192,153,60,32,203,229,5,59,174,78,165,227,134,37,219,55,
		161,4,206,7,18,148,208,35,65,197,2,82,209,64,100,16,87,184,3,94,11,36,230,74,173,127,202,193,139,238,38,180,177,222,241,41,228,77,
		88,63,211,237,51,186,151,97,80,232,218,214,143,179,109,125,159,171,210,118,223,115,205,163,160,13,166,120,98,67,216,31,144,68,76,75,
		191,190,188,138,27,91,83,220,73,152,207,96,201,176,48,243,130,49,162,108,116,194,170,154,110,135,215,225,244,147,12,57,141,137,196,
		140,131,39,19,104,103,17,213,70,113,242,14,85,26,1,42,204,45,172,56,44,106,29,217,169,30,129,10,20,28,164,181,69,105,245,246,247,248,
		249,250,251,252,253,254,255};

	for(int i=0;i<(int)length;i++)
	{
		data[i] = pKeyList[data[i]];
	}
}

/** 
 * 当数据达到时调用
 *
 * @param size 接收到的数据的大小
 */
void NetClient::OnRead(uint32 size)
{
	//m_readMutex.Acquire();
	while(true)
	{
		try
		{
			if(!remaining)
			{
				if(GetReadBuffer().GetSize() < sizeof(logonpacket)) 
				{
					//m_readMutex.Release();
					return;
				}

				// 首先取得版本号
				GetReadBuffer().Read((uint8*)&opcode,sizeof(uint16));

				if(opcode != 100) 
				{
					//m_readMutex.Release();
					return;
				}

				// 首先取得包头
				GetReadBuffer().Read((uint8*)&remaining,sizeof(uint32));
			}

			if(GetReadBuffer().GetSize() < remaining)
			{
				//m_readMutex.Release();
				return;
			}

			char buffer[MOL_REV_BUFFER_SIZE];                /**< 用于存储收到的数据 */
			memset(buffer,0,MOL_REV_BUFFER_SIZE);

			// 取得实际数据包
			GetReadBuffer().Read((uint8*)buffer,remaining);

			int dlength = remaining;
			//sSocketMgr.uncompress((unsigned char*)myBuffer,myheader.nDataLen,&dlength);
			//char* rdata = sSocketMgr.uncompress((unsigned char*)buffer,remaining,&dlength);
			Decrypto((uint8*)buffer,dlength);

			if(dlength > 0 && dlength < MOL_REV_BUFFER_SIZE_TWO)
			{
				m_heartJitter = (uint32)time(NULL);

				CMolMessageIn *in = new CMolMessageIn(buffer,dlength);
				//sSocketMgr.AddMessage(MessageStru(MES_TYPE_ON_READ,(uint32)GetFd(),in));

				ServerGameFrameManager.OnProcessNetMes(this,in);

				delete in;
				in = NULL;
			}

			remaining = 0;
			opcode = 0;
		}
		catch (std::exception e)
		{
			//m_readMutex.Release();
			TRACE("接收数据异常:%s\n",e.what());
		}
	}
	//m_readMutex.Release();
}

/** 
 * 当一个连接成功建立时调用
 */
void NetClient::OnConnect()
{
	//sSocketMgr.AddMessage(MessageStru(MES_TYPE_ON_CONNECTED,(uint32)GetFd()));
	ServerGameFrameManager.OnProcessConnectedNetMes(this);
}

/** 
 * 当一个连接断开时调用
 */
void NetClient::OnDisconnect()
{
	//sSocketMgr.AddMessage(MessageStru(MES_TYPE_ON_DISCONNECTED,(uint32)GetFd()));
	ServerGameFrameManager.OnProcessDisconnectNetMes(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
 * 构造函数
 */
MolNetworkUpdate::MolNetworkUpdate()
: m_curTime(0),m_TimeSpeed(10),m_threadTimer(0),m_threadTimeSpeed(20000),
	m_UpdateTime(0),m_UpdateTimeSpeed(2000)
{

}

/** 
 * 析构函数
 */
MolNetworkUpdate::~MolNetworkUpdate()
{

}

bool MolNetworkUpdate::run()
{
	if(m_ServerSocket == NULL) return true;

	while(m_ServerSocket->IsOpen())
	{
		//if(m_curTime == 0)
		//	m_curTime = GetTickCount();

		//if(GetTickCount() > m_curTime + m_TimeSpeed)
		//{
			sSocketMgr.Update();

		//	m_curTime = 0;
		//}

		//if(m_UpdateTime == 0)
		//	m_UpdateTime = GetTickCount();

		//if(GetTickCount() > m_UpdateTime + m_UpdateTimeSpeed)
		//{
			sSocketGarbageCollector.Update();

		//	m_UpdateTime = 0;
		//}

		if(m_threadTimer == 0)
			m_threadTimer = GetTickCount();

		if(GetTickCount() > m_threadTimer + m_threadTimeSpeed)
		{
			ThreadPool.IntegrityCheck();

			m_threadTimer = 0;
		}

		Sleep(1);
	}

	return false;
}

void SocketGarbageCollector::Update()
{
	std::map<Socket*,time_t>::iterator i;
	time_t t = time(NULL);
	lock.Acquire();
	for(i=deletonQueue.begin();i!=deletonQueue.end();)
	{
		if(i->second <= t)
		{
			Socket *pSocket = i->first;
			if (pSocket && 
				pSocket->removedFromSet)
			{
				delete pSocket;
				pSocket = NULL;	
				deletonQueue.erase(i++);	
			}					
			else
				++i;
		}
		else
			++i;
	}
	lock.Release();
}
