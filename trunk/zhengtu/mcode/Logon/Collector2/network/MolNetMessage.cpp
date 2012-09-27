#include "stdafx.h"
#include "MolNetMessage.h"

/** 
 * �������Ĺ��캯��
 *
 * @param count �������õ���Ϣ�б�Ҫ���յ���Ϣ����
 */
NetMessage::NetMessage(int count)
: m_MaxCount(count)
{

}

/** 
 * ��������
 */
NetMessage::~NetMessage()
{
	Clear();
}

/** 
 * �����Ϣ�б�
 */
void NetMessage::Clear(void)
{
	if(m_MesList.empty()) return;

	std::vector<MessageStru>::iterator iter = m_MesList.begin();
	for(;iter != m_MesList.end();++iter)
	{
		if((*iter).mes)
			delete (*iter).mes;
	}

	m_MesList.clear();
}

/** 
 * ���һ����Ϣ����Ϣ�б���
 *
 * @param mes Ҫ��ӵ���Ϣ
 */
void NetMessage::AddMessage(MessageStru mes)
{
	if(m_MaxCount <= 0 || 
		(int)m_MesList.size() > m_MaxCount)
		return;

	m_MesList.push_back(mes);
}

/** 
 * �õ�ָ��id����Ϣ
 *
 * @param id Ҫ�õ���Ϣ��ID
 *
 * @return �����ϢΪ�շ���NULL�����򷵻������Ϣ
 */
MessageStru* NetMessage::GetMesById(int id)
{
	if(id < 0 || id >= (int)m_MesList.size())
		return NULL;

	return &m_MesList[id];
}

/** 
 * �õ���Ϣ�б�
 *
 * @return ���ص�ǰ����Ϣ�б�
 */
std::vector<MessageStru>& NetMessage::GetMessage(void)
{
	return m_MesList;
}

