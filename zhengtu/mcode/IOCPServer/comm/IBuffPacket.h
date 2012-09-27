#ifndef _IBUFFPACKET_H
#define _IBUFFPACKET_H

#include "define.h"

//BuffPacket的接口类
class IBuffPacket
{
public:
	virtual ~IBuffPacket() {};

	virtual uint32 GetPacketSize()      = 0;    //得到数据包的格式化长度
	virtual uint32 GetPacketLen()       = 0;    //得到数据包的实际长度
	virtual uint32 GetReadLen()         = 0;    //得到包读取的长度
	virtual uint32 GetWriteLen()        = 0;    //得到包写入的长度
	virtual uint32 GetHeadLen()         = 0;    //得到数据包头的长度
	virtual uint32 GetPacketCount()     = 0;    //得到缓存数据包的个数

	virtual bool Init(int nSize)        = 0;
	virtual bool Close()                = 0;
	virtual bool Clear()                = 0;

	virtual bool AddBuff(uint32 u4Size) = 0;
	virtual const char* GetData()       = 0;

	virtual void SetReadPtr(uint32 u4Pos)                                = 0;    //设置读指针的位置
	virtual void SetPacketCount(uint32 u4PacketCount)                    = 0;    //设置缓存数据包的个数
	virtual void ReadPtr(uint32 u4Size)                                  = 0;
	virtual void WritePtr(uint32 u4Size)                                 = 0;
	virtual bool WriteStream(const char* szData, uint32 u4Len)           = 0;
	virtual bool ReadStream(char*& pData, uint32 u4MaxLen, uint32 u4Len) = 0;
	virtual char* ReadPtr()                                              = 0;    //获得读指针
	virtual char* WritePtr()                                             = 0;    //获得写指针
	virtual bool RollBack(uint32 u4Len)                                  = 0;    //将取出的数据删除，将后面的数据加上

	virtual void ReadBuffPtr(uint32 u4Size)                              = 0;
	virtual void WriteBuffPtr(uint32 u4Size)                             = 0;

	//读取
	virtual IBuffPacket& operator >> (uint8& u1Data)   = 0;
	virtual IBuffPacket& operator >> (uint16& u2Data)  = 0;
	virtual IBuffPacket& operator >> (uint32& u4Data)  = 0;
	virtual IBuffPacket& operator >> (uint64 &u8Data)  = 0;

	virtual IBuffPacket& operator >> (float32& f4Data) = 0;
	virtual IBuffPacket& operator >> (float64& f8Data) = 0;

	virtual IBuffPacket& operator >> (VCHARS_STR& str) = 0;
	virtual IBuffPacket& operator >> (VCHARM_STR& str) = 0;
	virtual IBuffPacket& operator >> (VCHARB_STR& str) = 0;

	//写入
	virtual IBuffPacket& operator << (uint8 u1Data)    = 0;
	virtual IBuffPacket& operator << (uint16 u2Data)   = 0;
	virtual IBuffPacket& operator << (uint32 u4Data)   = 0;
	virtual IBuffPacket& operator << (uint64 u8Data)   = 0;

	virtual IBuffPacket& operator << (float32 f4Data)  = 0;
	virtual IBuffPacket& operator << (float64 f8Data)  = 0;

	virtual IBuffPacket& operator << (VCHARS_STR &str) = 0;
	virtual IBuffPacket& operator << (VCHARM_STR &str) = 0;
	virtual IBuffPacket& operator << (VCHARB_STR &str) = 0;
};

#endif
