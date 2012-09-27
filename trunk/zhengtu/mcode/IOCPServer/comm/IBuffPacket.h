#ifndef _IBUFFPACKET_H
#define _IBUFFPACKET_H

#include "define.h"

//BuffPacket�Ľӿ���
class IBuffPacket
{
public:
	virtual ~IBuffPacket() {};

	virtual uint32 GetPacketSize()      = 0;    //�õ����ݰ��ĸ�ʽ������
	virtual uint32 GetPacketLen()       = 0;    //�õ����ݰ���ʵ�ʳ���
	virtual uint32 GetReadLen()         = 0;    //�õ�����ȡ�ĳ���
	virtual uint32 GetWriteLen()        = 0;    //�õ���д��ĳ���
	virtual uint32 GetHeadLen()         = 0;    //�õ����ݰ�ͷ�ĳ���
	virtual uint32 GetPacketCount()     = 0;    //�õ��������ݰ��ĸ���

	virtual bool Init(int nSize)        = 0;
	virtual bool Close()                = 0;
	virtual bool Clear()                = 0;

	virtual bool AddBuff(uint32 u4Size) = 0;
	virtual const char* GetData()       = 0;

	virtual void SetReadPtr(uint32 u4Pos)                                = 0;    //���ö�ָ���λ��
	virtual void SetPacketCount(uint32 u4PacketCount)                    = 0;    //���û������ݰ��ĸ���
	virtual void ReadPtr(uint32 u4Size)                                  = 0;
	virtual void WritePtr(uint32 u4Size)                                 = 0;
	virtual bool WriteStream(const char* szData, uint32 u4Len)           = 0;
	virtual bool ReadStream(char*& pData, uint32 u4MaxLen, uint32 u4Len) = 0;
	virtual char* ReadPtr()                                              = 0;    //��ö�ָ��
	virtual char* WritePtr()                                             = 0;    //���дָ��
	virtual bool RollBack(uint32 u4Len)                                  = 0;    //��ȡ��������ɾ��������������ݼ���

	virtual void ReadBuffPtr(uint32 u4Size)                              = 0;
	virtual void WriteBuffPtr(uint32 u4Size)                             = 0;

	//��ȡ
	virtual IBuffPacket& operator >> (uint8& u1Data)   = 0;
	virtual IBuffPacket& operator >> (uint16& u2Data)  = 0;
	virtual IBuffPacket& operator >> (uint32& u4Data)  = 0;
	virtual IBuffPacket& operator >> (uint64 &u8Data)  = 0;

	virtual IBuffPacket& operator >> (float32& f4Data) = 0;
	virtual IBuffPacket& operator >> (float64& f8Data) = 0;

	virtual IBuffPacket& operator >> (VCHARS_STR& str) = 0;
	virtual IBuffPacket& operator >> (VCHARM_STR& str) = 0;
	virtual IBuffPacket& operator >> (VCHARB_STR& str) = 0;

	//д��
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
