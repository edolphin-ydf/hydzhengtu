//////////////////////////////////////////////////
/// @file : MapTemplate.h
/// @brief : MRU算法实现，McMruContainter优于CMapTemplate
/// @date:  2012/10/10
/// @author : hyd
//////////////////////////////////////////////////
#ifndef _MAPTEMPLATE_H
#define _MAPTEMPLATE_H

#include <map>
#include "ListTemplate.h"

using namespace std;

//负责模板化map的类
template <class Key, class T>
class CMapTemplate
{
private:
	typedef map<Key, T*>          mapData;          //定义map结构
	typedef map<CListNode*, Key>  mapNode2Key;      //定义链表和Key的对应关系
	typedef map<Key, CListNode*>  mapKey2Node;      //定义链表和Key的对应关系
	CLinkList                     m_LinkList;       //正在使用的链表
	CLinkList                     m_FreeLinkList;   //空闲的链表
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

	//添加数据
	bool AddMapData(Key mapkey, T* pData)
	{
		if((int)m_mapData.size() >= m_nMaxCount)
		{
			//运行清退算法
			DelLastMapData();
		}

		//添加对应关系和数据
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

	//删除数据
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

	//清除map内的所有数据
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

	//查找数据
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

	//得到当前的map的个数
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
/// @brief MRU算法的容易类：删除最后最不常访问的对象
/// 
/// @note 在STL中，list因为是用链表实现，比较容易删除，用于维护删除队列
class McMruContainter
{
	typedef list<K*> LIST_KEY;//经常访问队列，经常访问的在前面
	typedef typename LIST_KEY::iterator LIST_KEY_ITERATOR;
	struct MsItem//map元素
	{
		V* data;//数据指针
		LIST_KEY_ITERATOR key_pos;//在经常访问队列中的位置，便于通过元素在经常访问队列中找到相应的元素
	};
	typedef map<K, MsItem> MAP_DATA;
	MAP_DATA map_data;//data表
	LIST_KEY list_key;//经常访问队列
	int m_nMaxCount;  //最大元素个数
public:
	/** @brief 参数nCount表示这个容易的容量 */
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
		if(it != map_data.end())//已经存在
			return false;

		int nSize = (int)map_data.size();
		if(nSize >= m_nMaxCount)//超过当前容器的大小
		{
			//释放最后一个元素
			K* last = *list_key.rbegin();//最后一个元素的key指针
			list_key.pop_back();//经常访问队列中删除最后一个元素
			it = map_data.find(*last);//data表中查找相应的元素
			assert(it != map_data.end());//一定能找到
			delete it->second.data;//清除相应的数据
			map_data.erase(it);//data表中删除相应的元素
		}

		//现在肯定有空间了
		list_key.push_front(NULL);//新增的总是在经常访问队列的头部
		LIST_KEY_ITERATOR new_key_pos = list_key.begin();//获取k在列表中的位置
		MsItem item;
		item.data = v;
		item.key_pos = new_key_pos;
		MAP_DATA::iterator itNew = (map_data.insert(make_pair(k, item))).first;//在data表中增加元素
		*new_key_pos = const_cast<K*>(&(itNew->first));//将新的元素的k的地址，赋值到新的经常访问队列中去
		//一开始用的是NULL，因为那时还不知道key的地址，现在知道了，在data表中，为新元素分配了空间来存放k和v
		return true;
	}
	bool DelMapData(const K& k)
	{
		MAP_DATA::iterator it = map_data.find(k);
		if(it == map_data.end())//不存在
			return false;

		MsItem& item = it->second;
		delete item.data;//删除相应的元素的数据
		list_key.erase(item.key_pos);//删除经常访问队列中的元素
		map_data.erase(it);//删除data表中的元素
		return true;
	}
	V* SearchMapData(const K& k)
	{
		MAP_DATA::iterator it = map_data.find(k);
		if(it == map_data.end())//不存在
			return NULL;

		MsItem& item = it->second;
		LIST_KEY_ITERATOR it_key = item.key_pos;
		list_key.push_front(*it_key);//将当前的值插入到队头
		list_key.erase(it_key);//删除当前的元素。这两句相当于将当前元素移到了队头
		item.key_pos = list_key.begin();//修改item中的key_pos，它等于队头

		return item.data;//返回数据指针
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
