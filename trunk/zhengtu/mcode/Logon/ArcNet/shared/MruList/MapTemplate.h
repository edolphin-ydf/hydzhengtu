//////////////////////////////////////////////////
/// @file : MapTemplate.h
/// @brief : MRU�㷨ʵ�֣�McMruContainter����CMapTemplate
/// @date:  2012/10/10
/// @author : hyd
//////////////////////////////////////////////////
#ifndef _MAPTEMPLATE_H
#define _MAPTEMPLATE_H

#include <map>
#include "ListTemplate.h"

using namespace std;

//����ģ�廯map����
template <class Key, class T>
class CMapTemplate
{
private:
	typedef map<Key, T*>          mapData;          //����map�ṹ
	typedef map<CListNode*, Key>  mapNode2Key;      //���������Key�Ķ�Ӧ��ϵ
	typedef map<Key, CListNode*>  mapKey2Node;      //���������Key�Ķ�Ӧ��ϵ
	CLinkList                     m_LinkList;       //����ʹ�õ�����
	CLinkList                     m_FreeLinkList;   //���е�����
	mapData                       m_mapData;
	mapNode2Key                   m_mapNode2Key;
	mapKey2Node                   m_mapKey2Node;
	int                           m_nMaxCount;
	
public:
	CMapTemplate(int nCount)
	{
		m_nMaxCount = nCount;
		for(int i = 0; i < nCount; i++)
		{
			CListNode* pListNode = new CListNode();
			m_FreeLinkList.Add(pListNode, NULL);
		}
	}

	~CMapTemplate() 
	{
		//Clear();
	}

	bool DelLastMapData()
	{
		CListNode* pListNode = m_LinkList.GetLast();
		if(NULL == pListNode)
		{
			return false;
		}

		Key* pKey = FindNode2Key(pListNode);

		return DelMapData(*pKey);
	};

	//�������
	bool AddMapData(Key mapkey, T* pData)
	{
		if((int)m_mapData.size() >= m_nMaxCount)
		{
			//���������㷨
			DelLastMapData();
		}

		//��Ӷ�Ӧ��ϵ������
		//CListNode* pListNode = new CListNode();
		CListNode* pListNode = m_FreeLinkList.GetLastNode();
		if(NULL == pListNode)
		{
			return false;
		}

		m_LinkList.Add(pListNode, NULL);
		if(AddNode2Key(pListNode, mapkey) == false)
		{
			return false;
		}

		if(AddKey2Node(pListNode, mapkey) == false)
		{
			return false;
		}
		m_LinkList.MoveTop(pListNode);

		typename mapData::iterator f = m_mapData.find(mapkey);
		if(f != m_mapData.end())
		{
			return false;
		}

		m_mapData.insert(typename mapData::value_type(mapkey, pData));
		return true;
	}

	//ɾ������
	bool DelMapData(Key mapkey)
	{
		CListNode* pListNode = FindKey2Node(mapkey);
		if(NULL == pListNode)
		{
			return false;
		}

		DelKey2Node(mapkey);
		DelNode2Key(pListNode);
		m_LinkList.DelNode(pListNode, false);
		m_FreeLinkList.Add(pListNode, NULL);

		typename mapData::iterator f = m_mapData.find(mapkey);

		if(f != m_mapData.end())
		{
			T* pData = (T*)f->second;
			if(pData != NULL)
			{
				delete pData;
			}
			m_mapData.erase(f);

			return true;
		}
		else
		{
			return false;
		}
	}

	//���map�ڵ���������
	void Clear()
	{
		typename mapData::iterator b = m_mapData.begin();
		typename mapData::iterator e = m_mapData.end();

		for(b; b != e; b++)
		{
			T* pData = (T*)b->second;
			if(pData != NULL)
			{
				delete pData;
			}
		}

		m_mapData.clear();
	}

	//��������
	T* SearchMapData(Key mapkey)
	{
		typename mapData::iterator f = m_mapData.find(mapkey);

		if(f != m_mapData.end())
		{
			CListNode* pListNode = FindKey2Node(mapkey);
			if(NULL == pListNode)
			{
				return false;
			}
			m_LinkList.MoveTop(pListNode);

			T* pData = (T*)f->second;
			return pData;
		}
		else
		{
			return NULL;
		}
	}

	//�õ���ǰ��map�ĸ���
	int GetSize()
	{
		return (int)m_mapData.size();
	}

private:
	bool AddNode2Key(CListNode* pListNode, Key mapkey)
	{
		typename mapNode2Key::iterator fNode = m_mapNode2Key.find(pListNode);
		if(fNode != m_mapNode2Key.end())
		{
			return false;
		}
		m_mapNode2Key.insert(typename mapNode2Key::value_type(pListNode, mapkey));
		return true;
	}

	bool AddKey2Node(CListNode* pListNode, Key mapkey)
	{
		typename mapKey2Node::iterator fNode = m_mapKey2Node.find(mapkey);
		if(fNode != m_mapKey2Node.end())
		{
			return false;
		}
		m_mapKey2Node.insert(typename mapKey2Node::value_type(mapkey, pListNode));
		return true;
	}

	bool DelNode2Key(CListNode* pListNode)
	{
		typename mapNode2Key::iterator fNode = m_mapNode2Key.find(pListNode);
		if(fNode != m_mapNode2Key.end())
		{
			m_mapNode2Key.erase(fNode);
			return true;
		}

		return false;
	}

	bool DelKey2Node(Key mapKey)
	{
		typename mapKey2Node::iterator fNode = m_mapKey2Node.find(mapKey);
		if(fNode != m_mapKey2Node.end())
		{
			m_mapKey2Node.erase(fNode);
			return true;
		}
		return false;
	}

	CListNode* FindKey2Node(Key mapKey)
	{
		typename mapKey2Node::iterator fNode = m_mapKey2Node.find(mapKey);
		if(fNode != m_mapKey2Node.end())
		{
			return (CListNode* )fNode->second;
		}
		return NULL;
	}

	Key* FindNode2Key(CListNode* pListNode)
	{
		typename mapNode2Key::iterator fNode = m_mapNode2Key.find(pListNode);
		if(fNode != m_mapNode2Key.end())
		{
			return(Key* )&fNode->second;
		}

		return NULL;
	}
};



#include <list>
#include <assert.h>
template<class K, class V>
////////////////////////////////////////////////////////////////
/// @class McMruContainter
/// @brief MRU�㷨�������ࣺɾ�����������ʵĶ���
/// 
/// @note ��STL�У�list��Ϊ��������ʵ�֣��Ƚ�����ɾ��������ά��ɾ������
class McMruContainter
{
	typedef list<K*> LIST_KEY;//�������ʶ��У��������ʵ���ǰ��
	typedef typename LIST_KEY::iterator LIST_KEY_ITERATOR;
	struct MsItem//mapԪ��
	{
		V* data;//����ָ��
		LIST_KEY_ITERATOR key_pos;//�ھ������ʶ����е�λ�ã�����ͨ��Ԫ���ھ������ʶ������ҵ���Ӧ��Ԫ��
	};
	typedef map<K, MsItem> MAP_DATA;
	MAP_DATA map_data;//data��
	LIST_KEY list_key;//�������ʶ���
	int m_nMaxCount;  //���Ԫ�ظ���
public:
	/** @brief ����nCount��ʾ������׵����� */
	McMruContainter(int nCount)
	{
		m_nMaxCount = nCount;
	}
	~McMruContainter()
	{
		Clear();
	}
	bool AddMapData(const K& k, V* v)
	{
		MAP_DATA::iterator it = map_data.find(k);
		if(it != map_data.end())//�Ѿ�����
			return false;

		int nSize = (int)map_data.size();
		if(nSize >= m_nMaxCount)//������ǰ�����Ĵ�С
		{
			//�ͷ����һ��Ԫ��
			K* last = *list_key.rbegin();//���һ��Ԫ�ص�keyָ��
			list_key.pop_back();//�������ʶ�����ɾ�����һ��Ԫ��
			it = map_data.find(*last);//data���в�����Ӧ��Ԫ��
			assert(it != map_data.end());//һ�����ҵ�
			delete it->second.data;//�����Ӧ������
			map_data.erase(it);//data����ɾ����Ӧ��Ԫ��
		}

		//���ڿ϶��пռ���
		list_key.push_front(NULL);//�����������ھ������ʶ��е�ͷ��
		LIST_KEY_ITERATOR new_key_pos = list_key.begin();//��ȡk���б��е�λ��
		MsItem item;
		item.data = v;
		item.key_pos = new_key_pos;
		MAP_DATA::iterator itNew = (map_data.insert(make_pair(k, item))).first;//��data��������Ԫ��
		*new_key_pos = const_cast<K*>(&(itNew->first));//���µ�Ԫ�ص�k�ĵ�ַ����ֵ���µľ������ʶ�����ȥ
		//һ��ʼ�õ���NULL����Ϊ��ʱ����֪��key�ĵ�ַ������֪���ˣ���data���У�Ϊ��Ԫ�ط����˿ռ������k��v
		return true;
	}
	bool DelMapData(const K& k)
	{
		MAP_DATA::iterator it = map_data.find(k);
		if(it == map_data.end())//������
			return false;

		MsItem& item = it->second;
		delete item.data;//ɾ����Ӧ��Ԫ�ص�����
		list_key.erase(item.key_pos);//ɾ���������ʶ����е�Ԫ��
		map_data.erase(it);//ɾ��data���е�Ԫ��
		return true;
	}
	V* SearchMapData(const K& k)
	{
		MAP_DATA::iterator it = map_data.find(k);
		if(it == map_data.end())//������
			return NULL;

		MsItem& item = it->second;
		LIST_KEY_ITERATOR it_key = item.key_pos;
		list_key.push_front(*it_key);//����ǰ��ֵ���뵽��ͷ
		list_key.erase(it_key);//ɾ����ǰ��Ԫ�ء��������൱�ڽ���ǰԪ���Ƶ��˶�ͷ
		item.key_pos = list_key.begin();//�޸�item�е�key_pos�������ڶ�ͷ

		return item.data;//��������ָ��
	}
	void Clear()
	{
		MAP_DATA::iterator itEnd = map_data.end();
		for(MAP_DATA::iterator it = map_data.begin(); it != itEnd; it++)
			delete it->second.data;
		map_data.clear();
		list_key.clear();
	}
	int GetSize()
	{
		return (int)map_data.size();
	}
};

#endif
