/*
*   ����������
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
	  std::map<DWORD,unsigned short> tempDups;//��ʱ�Ը���
	  std::map<DWORD,unsigned short> circleDups;//�����Ը���
	  unsigned int _index;//�û���ǰ����,0�����ͼ,>0,����
	  Scene *currentScene;//�û���ǰ������scene
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
    

    //ע��һ����ͼ,���Ǹ���ͼ�ĸ�����Ϣ��������
	void registerMap(DWORD id,zTiles*);

	//����һ����ͼ������Ϣ�Ŀ���
	zTiles * generateOneMap(DWORD id);
	


	//����һ������
	unsigned short CreateDup();

	//���븱��
	bool EnterDup(SceneUser *user,unsigned short index);


	//�뿪����
	bool leaveDup(SceneUser *user,unsigned short index);

	//���NPC������
	bool addNPC(SceneNpc *npc);	

	bool removeNpc(SceneNpc *npc,unsigned short index);

	//�����Ʒ������
	bool addObj(zSceneObject *obj);

	bool removeObj(zSceneObject *obj,unsigned short index);

	bool userQuestEnterDup(SceneUser *user,DWORD mapId);


	dupScenePair checkUserDup(SceneUser *user);


	//ִ��������
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

	



	//1:������,2:�û��б�
	typedef hash_map<unsigned short,std::list<SceneUser*>* > dupToUserMap;

	typedef hash_map<unsigned short,std::list<SceneUser*>* >::iterator dupToUserMap_it;
      
	typedef std::list<unsigned short> dupIndexList;

	typedef std::list<clearDupStruct> recycleList;

	//�����ͼ������Ϣ
	typedef hash_map<unsigned short,zTiles*> dupMapCopy;

	typedef hash_map<unsigned short,zTiles*>::iterator dupMapCopy_it;

	//���������б�
	typedef std::vector<hash_map<DWORD,Scene*>* > dupScenes;
	typedef std::vector<hash_map<DWORD,Scene*>* >::iterator dupScenes_it;
	//(����,NPC)
	typedef hash_map<unsigned short,hash_map<DWORD,SceneNpc*>* > dupMapNpc;
	typedef hash_map<unsigned short,hash_map<DWORD,SceneNpc*>* >::iterator dupMapNpc_it;


	//(����,object)
	typedef hash_map<unsigned short,hash_map<DWORD,zSceneObject*>*> dupMapObj;
	typedef hash_map<unsigned short,hash_map<DWORD,zSceneObject*>* >::iterator dupMapObj_it;

	//(����,��ͼ)
	//typedef hash_map<unsigned short,dupMapNpc*> dupMaps;

	typedef hash_map<DWORD,userDupMap*> userDupMaps;
	typedef hash_map<DWORD,userDupMap*>::iterator userDupMaps_it;

	
	userDupMaps _userDupMaps;


	recycleList _recycleList;//�ȴ�����ĸ����б�

	dupIndexList freeList;//���õĸ����ű�

	dupToUserMap _dupToUserMap;

	dupMapCopy _dupMapCopy;

	dupMapNpc _dupMapNpc;

	dupMapObj _dupMapObj;

	dupScenes _dupScenes;

	zMutex mutex;


};

#endif