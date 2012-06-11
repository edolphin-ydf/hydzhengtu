/*
*   副本管理器
*/
#include <zebra/ScenesServer.h>
#include <list>
#include <hash_map>
#include <vector>

#ifndef _DUPLICATEMANAGER_H
#define _DUPLICATEMANAGER_H

const unsigned short MAXDUP = 4096;


struct userDupMap
{
	  std::map<DWORD,unsigned short> tempDups;//临时性副本
	  std::map<DWORD,unsigned short> circleDups;//周期性副本
	  unsigned int _index;//用户当前在哪,0世界地图,>0,副本
	  Scene *currentScene;//用户当前所有在scene
};


struct dupScenePair
{
	unsigned int dupIndex;
	DWORD mapid;
};

struct clearDupStruct
{
	unsigned short dupIndex;
	DWORD timeStamp;
};


class duplicateManager : public SingletonBase<duplicateManager> 
{
	friend class SingletonBase<duplicateManager>;
	
public:
    

    //注册一个地图,把那个地图的格子信息保存下来
	void registerMap(DWORD id,zTiles*);

	//生成一个地图格子信息的拷贝
	zTiles * generateOneMap(DWORD id);
	


	//创建一个副本
	unsigned short CreateDup();

	//进入副本
	bool EnterDup(SceneUser *user,unsigned short index);


	//离开副本
	bool leaveDup(SceneUser *user,unsigned short index);

	//添加NPC到副本
	bool addNPC(SceneNpc *npc);	

	bool removeNpc(SceneNpc *npc,unsigned short index);

	//添加物品到副本
	bool addObj(zSceneObject *obj);

	bool removeObj(zSceneObject *obj,unsigned short index);

	bool userQuestEnterDup(SceneUser *user,DWORD mapId);


	dupScenePair checkUserDup(SceneUser *user);


	//执行清理工作
	void doClear();



	
	void clearDup(std::list<clearDupStruct>::iterator &it);



	void putARecycleDup(clearDupStruct _clearDupStruct)
	{
		zMutex_scope_lock scope_lock(mutex);
		_recycleList.push_back(_clearDupStruct);
	}


protected:
	
	duplicateManager()
	{
		zMutex_scope_lock scope_lock(mutex);
		for(unsigned short i = 1; i <= MAXDUP; ++i)
		{
			freeList.push_back(i);
			_dupScenes.push_back(NULL);
		}
	}



private:

	



	//1:副本号,2:用户列表
	typedef hash_map<unsigned short,std::list<SceneUser*>* > dupToUserMap;

	typedef hash_map<unsigned short,std::list<SceneUser*>* >::iterator dupToUserMap_it;
      
	typedef std::list<unsigned short> dupIndexList;

	typedef std::list<clearDupStruct> recycleList;

	//保存地图格子信息
	typedef hash_map<unsigned short,zTiles*> dupMapCopy;

	typedef hash_map<unsigned short,zTiles*>::iterator dupMapCopy_it;

	//副本场景列表
	typedef std::vector<hash_map<DWORD,Scene*>* > dupScenes;
	typedef std::vector<hash_map<DWORD,Scene*>* >::iterator dupScenes_it;
	//(副本,NPC)
	typedef hash_map<unsigned short,hash_map<DWORD,SceneNpc*>* > dupMapNpc;
	typedef hash_map<unsigned short,hash_map<DWORD,SceneNpc*>* >::iterator dupMapNpc_it;


	//(副本,object)
	typedef hash_map<unsigned short,hash_map<DWORD,zSceneObject*>*> dupMapObj;
	typedef hash_map<unsigned short,hash_map<DWORD,zSceneObject*>* >::iterator dupMapObj_it;

	//(副本,地图)
	//typedef hash_map<unsigned short,dupMapNpc*> dupMaps;

	typedef hash_map<DWORD,userDupMap*> userDupMaps;
	typedef hash_map<DWORD,userDupMap*>::iterator userDupMaps_it;

	
	userDupMaps _userDupMaps;


	recycleList _recycleList;//等待清理的副本列表

	dupIndexList freeList;//可用的副本号表

	dupToUserMap _dupToUserMap;

	dupMapCopy _dupMapCopy;

	dupMapNpc _dupMapNpc;

	dupMapObj _dupMapObj;

	dupScenes _dupScenes;

	zMutex mutex;


};

#endif