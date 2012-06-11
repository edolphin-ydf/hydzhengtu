#include "duplicateManager.h"


//����һ������
unsigned short duplicateManager::CreateDup()
{	
	zMutex_scope_lock scope_lock(mutex);
	unsigned short dupIndex = 0;
	if(!freeList.empty())
	{
		dupIndex = freeList.back();
		freeList.pop_back();

		
		dupToUserMap_it it = _dupToUserMap.find(dupIndex);
		if(it == _dupToUserMap.end())
		{
			std::list<SceneUser*> *users = new std::list<SceneUser*>;
			_dupToUserMap[dupIndex] = users;
		}

		fprintf(stderr,"�û���������%u\n",dupIndex);
		return dupIndex;
	}

	return dupIndex;
}


//���븱��
bool duplicateManager::EnterDup(SceneUser *user,unsigned short index)
{
	zMutex_scope_lock scope_lock(mutex);
	if(user)
	{
		dupToUserMap_it it = _dupToUserMap.find(index);
		if(it != _dupToUserMap.end())
		{
			_dupToUserMap[index]->push_back(user);
			fprintf(stderr,"�û����븱��%u\n",index);
			if(_dupScenes[index-1] == NULL)
			{
				hash_map<DWORD,Scene*> *_l = new hash_map<DWORD,Scene*>;
				(*_l)[(DWORD)user->scene] = user->scene;
				_dupScenes[index-1] = _l;
				//ˢ��,ˢNPC
				execute_script_event(user,"summonNPC");

			}
			else
			{
				hash_map<DWORD,Scene*>::iterator it = _dupScenes[index-1]->find((DWORD)user->scene);
				if(it == _dupScenes[index-1]->end())
				{
					(*_dupScenes[index-1])[(DWORD)user->scene] = user->scene;
					//ˢ��,ˢNPC
					execute_script_event(user,"summonNPC");
				}

				
			}
			return true;
		}
	}

	return false;
}

//�뿪����
bool duplicateManager::leaveDup(SceneUser *user,unsigned short index)
{
	zMutex_scope_lock scope_lock(mutex);
	if(user)
	{
		
		dupToUserMap_it it = _dupToUserMap.find(index);
		if(it != _dupToUserMap.end())
		{
			//_dupToUserMap[index]->push_back(user);
			std::list<SceneUser*>::iterator it = _dupToUserMap[index]->begin();
			std::list<SceneUser*>::iterator end = _dupToUserMap[index]->end();
			for(  ; it != end; ++it)
			{
				if((*it) == user)
				{
					_dupToUserMap[index]->erase(it);
					break;
				}
			}

			//�Ҳ����û�
			if(it == end)
				return false;




			fprintf(stderr,"�û��뿪����%u\n",index);


			userDupMaps_it _userDupMaps_it = _userDupMaps.find(user->id);

			if(_userDupMaps_it != _userDupMaps.end())
			{
				userDupMap *_userDupMap = _userDupMaps_it->second;
				_userDupMap->_index = 0;
			}

			//�û��Ѿ�ȫ���뿪����
			if(_dupToUserMap[index]->empty())
			{
				fprintf(stderr,"�û���ȫ���뿪����%u\n",index);
				clearDupStruct _clearDupStruct;
				_clearDupStruct.dupIndex = index;
				_clearDupStruct.timeStamp = time(NULL);
				putARecycleDup(_clearDupStruct);
				
				
			}
		}
	}

	return false;
}



void duplicateManager::registerMap(DWORD id,zTiles* _zTiles)
{
    zMutex_scope_lock scope_lock(mutex);
	dupMapCopy_it it = _dupMapCopy.find(id);
	if(it != _dupMapCopy.end())
	{
		zTiles * newTiles = new zTiles(_zTiles->begin(),_zTiles->end());
		_dupMapCopy[id] = newTiles;
	}
}


	//���NPC������
bool duplicateManager::addNPC(SceneNpc *npc)
{
	zMutex_scope_lock scope_lock(mutex);
	dupMapNpc_it it = _dupMapNpc.find(npc->dupIndex);
	fprintf(stderr,"���һ��NPC������%u\n",npc->dupIndex);
	if(it == _dupMapNpc.end())
	{
		hash_map<DWORD,SceneNpc*>* _list = new hash_map<DWORD,SceneNpc*>;
		(*_list)[(DWORD)npc] = npc;
		_dupMapNpc[npc->dupIndex] = _list;
	}
	else
	{
		(*_dupMapNpc[npc->dupIndex])[(DWORD)npc] = npc;
	}
	return true;
}


bool duplicateManager::addObj(zSceneObject *obj)
{
	zMutex_scope_lock scope_lock(mutex);
	dupMapObj_it it = _dupMapObj.find(obj->dupIndex);
	fprintf(stderr,"���һ��Obj������%u\n",obj->dupIndex);
	if(it == _dupMapObj.end())
	{
		hash_map<DWORD,zSceneObject*>* _list = new hash_map<DWORD,zSceneObject*>;
		(*_list)[(DWORD)obj]= obj;
		_dupMapObj[obj->dupIndex] = _list;
	}
	else
	{
		//_dupMapObj[obj->dupIndex]->push_back(obj);
		(*_dupMapObj[obj->dupIndex])[(DWORD)obj] = obj;
	}
	return true;
}


bool duplicateManager::removeNpc(SceneNpc *npc,unsigned short index)
{
	zMutex_scope_lock scope_lock(mutex);
	hash_map<DWORD,SceneNpc*>::iterator it = _dupMapNpc[index]->find((DWORD)npc);
	if(it != _dupMapNpc[index]->end())
	{
		_dupMapNpc[index]->erase(it);
		return true;
	}

	return false;
}

bool duplicateManager::removeObj(zSceneObject *obj,unsigned short index)
{
	zMutex_scope_lock scope_lock(mutex);
	hash_map<DWORD,zSceneObject*>::iterator it = _dupMapObj[index]->find((DWORD)obj);
	if(it != _dupMapObj[index]->end())
	{
		_dupMapObj[index]->erase(it);
		return true;
	}

	return false;
}


dupScenePair duplicateManager::checkUserDup(SceneUser *user)
{
	dupScenePair _dupScenePair;
	_dupScenePair.dupIndex = 0;
	_dupScenePair.mapid = 0;
	
	userDupMaps_it it = _userDupMaps.find(user->id);
	if(it == _userDupMaps.end())
	{

		return _dupScenePair;
	}
	userDupMap *_userDupMap = it->second;
	std::map<DWORD,unsigned short>::iterator it1 = _userDupMap->tempDups.begin();
	std::map<DWORD,unsigned short>::iterator end1 = _userDupMap->tempDups.end();
	if(it1 != end1)
	{
		_dupScenePair.dupIndex = it1->second;
		_dupScenePair.mapid = it1->first;	
	}

	return _dupScenePair;
	
	//for( ; it1 != end1; ++it1)
	//{
		
	//}

}

bool duplicateManager::userQuestEnterDup(SceneUser *user,DWORD mapId)
{
		userDupMaps_it it = _userDupMaps.find(user->id);
		if(it == _userDupMaps.end())
		{
			userDupMap *_userDupMap = new userDupMap;
			
			//����鿴���ж������Ƿ������ڸ�����,��������Ǹ�����


			TeamManager * team = SceneManager::getInstance().GetMapTeam(user->TeamThisID);

			if( team == NULL ) return false; // [ranqd] �û��޶���ʱ����

			int tSize = team->getSize();

			for( int i = 0; i < tSize; ++i)
			{
				const TeamMember * member = user->getMember(i);
				SceneUser *u = SceneUserManager::getMe().getUserByID(member->id);
				if(NULL == u)
					continue;
				
				dupScenePair _dupScenePair = checkUserDup(u);

				if(_dupScenePair.dupIndex != 0)
				{
					//DWORD mapid = u->scene->getRealMapID();
					(_userDupMap->tempDups)[mapId] = _dupScenePair.dupIndex;
					return user->userEnterDup(_dupScenePair.dupIndex,_dupScenePair.mapid,_userDupMap);
				}

			}			
			//�����¸���
			unsigned short _dupIndex = duplicateManager::getInstance().CreateDup();
			_userDupMaps[user->id] = _userDupMap;
			if(_dupIndex != 0)
				return user->userEnterDup(_dupIndex,mapId,_userDupMap);

		}
		else
		{
			userDupMap *_userDupMap = it->second;
			std::map<DWORD,unsigned short>::iterator it1 = (_userDupMap->tempDups).find(mapId);
			//���ڶ�Ӧ��ͼ�ĸ��������
			if(it1 != (_userDupMap->tempDups).end())
			{
				return user->userEnterDup(it1->second,mapId,_userDupMap);
			}

			//����鿴���ж������Ƿ������ڸ�����,��������Ǹ�����
			TeamManager * team = SceneManager::getInstance().GetMapTeam(user->TeamThisID);
			
			int tSize = 0;

			if(team)
				tSize = team->getSize();

			for( int i = 0; i < tSize; ++i)
			{
				const TeamMember * member = user->getMember(i);
				SceneUser *u = SceneUserManager::getMe().getUserByID(member->id);
				if(NULL == u)
					continue;
				
				dupScenePair _dupScenePair = checkUserDup(u);

				if(_dupScenePair.dupIndex != 0)
				{
					//DWORD mapid = u->scene->getRealMapID();
					(_userDupMap->tempDups)[mapId] = _dupScenePair.dupIndex;
					return user->userEnterDup(_dupScenePair.dupIndex,_dupScenePair.mapid,_userDupMap);
				}

			}

            //�����¸���
			unsigned short _dupIndex = duplicateManager::getInstance().CreateDup();
			_userDupMaps[user->id] = _userDupMap;
			if(_dupIndex != 0)
				return user->userEnterDup(_dupIndex,mapId,_userDupMap);

		}
	
		
 }


void duplicateManager::clearDup(std::list<clearDupStruct>::iterator &it)
{
		clearDupStruct _clearDupStruct = (*it);
		//ʱ�䵽��,������
		if(time(NULL) - _clearDupStruct.timeStamp >= 50)
		{
			//�ٴμ�鸱�����Ƿ������û�
			if(_dupToUserMap[_clearDupStruct.dupIndex]->empty())
			{
			    fprintf(stderr,"������%u\n",_clearDupStruct.dupIndex);
				//������ʱ����,ʱ�䵽��ʱ������			
				//ɾ�������е�����NPC
				if(_dupMapNpc.find(_clearDupStruct.dupIndex) != _dupMapNpc.end() )
				{
					hash_map<DWORD,SceneNpc*>::iterator it0 = _dupMapNpc[_clearDupStruct.dupIndex]->begin();
					hash_map<DWORD,SceneNpc*>::iterator end0 = _dupMapNpc[_clearDupStruct.dupIndex]->end();
					for( ; it0 != end0; ++it0)
					{
						fprintf(stderr,"ɾ��һ��NPC\n");
						it0->second->setState(zSceneEntry::SceneEntry_Death);
						//_dupMapNpc[index]->erase(it);
						//delete (*it);

					}
					_dupMapNpc[_clearDupStruct.dupIndex]->clear();
				}

				//ɾ�������е�����object
				if(_dupMapObj.find(_clearDupStruct.dupIndex) != _dupMapObj.end())
				{
					hash_map<DWORD,zSceneObject*>::iterator it1 = _dupMapObj[_clearDupStruct.dupIndex]->begin();
					hash_map<DWORD,zSceneObject*>::iterator end1 = _dupMapObj[_clearDupStruct.dupIndex]->end();
					for( ; it1 != end1; ++it1)
					{				
						hash_map<DWORD,Scene*>::iterator it2 = _dupScenes[_clearDupStruct.dupIndex-1]->begin();
						hash_map<DWORD,Scene*>::iterator end2= _dupScenes[_clearDupStruct.dupIndex-1]->end();
						for( ; it2 != end2; ++it2)
						{
							if (it2->second->removeSceneEntry(it1->second))
							{
								it2->second->clearObjectBlock(it1->second->getPos());
                                break;
							}
						}

						zObject::destroy(it1->second->getObject());
					

					  }

					_dupMapObj[_clearDupStruct.dupIndex]->clear();
				}

				freeList.push_back(_clearDupStruct.dupIndex);	
				
			}

			it = _recycleList.erase(it);
		}
		else
		{
			++it;
		}

		
}


void duplicateManager::doClear()
{
		zMutex_scope_lock scope_lock(mutex);
		std::list<clearDupStruct>::iterator it = _recycleList.begin();
	    std::list<clearDupStruct>::iterator end = _recycleList.end();
		for( ; it != end;)
		{
			clearDup(it);
		}
}
