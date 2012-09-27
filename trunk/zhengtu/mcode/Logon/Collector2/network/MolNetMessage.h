#ifndef _MOL_NET_MESSAGE_H_INCLUDE
#define _MOL_NET_MESSAGE_H_INCLUDE

/** 
* MolNet��������
*
* ����:ϵͳ���õ���������Ϣ
* ����:akinggw
* ����:2010.2.13
*/

#include "MolCommon.h"
#include "MolMutex.h"
#include "MolMessageIn.h"
#include "MolThreadStarter.h"

/** 
* ���紦�����Ϣ����
*/
enum MessageType
{
	MES_TYPE_ON_CONNECTED = 0,        // ���ӽ����ɹ���
	MES_TYPE_ON_DISCONNECTED,         // ���ӶϿ���
	MES_TYPE_ON_READ,                 // ���ݵ����
	MES_TYPE_NULL
};

/** 
* ϵͳ�õ�����Ϣ�ṹ
*/
struct MessageStru
{
	MessageStru()
		: type(MES_TYPE_NULL),socket(0),mes(NULL)
	{

	}
	MessageStru(MessageType t,uint32 s)
		: type(t),socket(s),mes(NULL)
	{

	}
	MessageStru(MessageType t,uint32 s,CMolMessageIn *in)
		: type(t),socket(s),mes(in)
	{

	}

	/// �õ���Ϣ���Ϳͻ���ID
	inline uint32 GetSocket(void)
	{
		return socket;
	}
	/// �õ���Ϣ����
	MessageType GetType(void)
	{
		return type;
	}
	/// �õ��ͻ��˷��͵���Ϣ
	CMolMessageIn* GetMes(void)
	{
		return mes;
	}

	uint32 socket;                    // ��Ϣ���Ϳͻ���
	MessageType type;                 // ��Ϣ����
	CMolMessageIn *mes;                // Ҫ�������Ϣ
};

/** 
* ����Ҫ�������Ϣ�ṹ
*/
class NetMessage
{
public:
	/// �������Ĺ��캯��
	NetMessage(int count=5000);
	/// ��������
	~NetMessage();

	/// ���������Ϣ����
	inline void SetMaxCount(int count) { m_MaxCount = count; }
	/// �õ������Ϣ����
	inline int GetMaxCount(void) { return m_MaxCount; }
	/// �õ�ʵ����Ϣ����
	inline int GetCount(void) { return (int)m_MesList.size(); }

	/// ���һ����Ϣ����Ϣ�б���
	void AddMessage(MessageStru mes);
	/// �õ���Ϣ�б�
	std::vector<MessageStru>& GetMessage(void);	
	/// �õ�ָ��id����Ϣ
	MessageStru* GetMesById(int id);
	/// �����Ϣ�б�
	void Clear(void);

private:
	int m_MaxCount;               /**< ���õ������� */
	std::vector<MessageStru> m_MesList;       /**< ���ڴ洢��Ϣ����Ϣ�б� */
};

#endif
