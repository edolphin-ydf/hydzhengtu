#ifndef _METERIALSMANAGER_H
#define _METERIALSMANAGER_H

#include <zebra/ScenesServer.h>
#include <map>

/*
* 将物品改造需要的信息以字符串的形式返回
*/


const unsigned short meterialSize = 13;

class meterialsManager : public SingletonBase<meterialsManager>
{
	friend class SingletonBase<meterialsManager>;

public:

	char *get(DWORD id)
	{
		zMutex_scope_lock scope_lock(mutex);
		iterator it = _map.find(id);
		if(it != _map.end())
		{
			return it->second;
		}

		zObjectB *base = objectbm.get(id);
		if(base)
		{
			DWORD size = base->need_material.stuffs.size();
			char *temp = new char[meterialSize*size + size + 5];
			char temp1[10];
			sprintf(temp1,"%u,",base->need_material.gold);
			strncpy(temp,temp1,sizeof(temp));
			for( int i =0; i < size; ++i)
			{
				char temp2[13];
				if(i != size-1)
					sprintf(temp2,"%u,%u,%u,",(base->need_material.stuffs)[i].id,(base->need_material.stuffs)[i].number,(base->need_material.stuffs)[i].level);
				else
					sprintf(temp2,"%u,%u,%u",(base->need_material.stuffs)[i].id,(base->need_material.stuffs)[i].number,(base->need_material.stuffs)[i].level);
				strcat(temp,temp2);
			}
			_map[id] = temp;
			return temp;
		}
		
		return NULL;
	}


protected:



	meterialsManager()
	{
		
	}

	~meterialsManager()
	{
		iterator it = _map.begin();
		iterator end = _map.end();
		for( ; it != end; ++it)
		{
			delete[] it->second;

		}

		_map.clear();
	}

private:
    
	zMutex mutex;
	std::map<DWORD,char*> _map;
	typedef std::map<DWORD,char*>::iterator iterator;



	
};



#endif