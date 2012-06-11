/**
 * \brief ���������ӵ�����
 *
 * 
 */
#include <zebra/ScenesServer.h>

SceneTaskManager *SceneTaskManager::instance = NULL;

/**
 * \brief �����������һ��������
 *
 * \param task ����������
 * \return ����Ƿ�ɹ�
 */
bool SceneTaskManager::uniqueAdd(SceneTask *task)
{
  SceneTaskHashmap_const_iterator it;
  rwlock.wrlock();
  it = sceneTaskSet.find(task->getID());
  if (it != sceneTaskSet.end())
  {
    rwlock.unlock();
    return false;
  }
  sceneTaskSet.insert(SceneTaskHashmap_pair(task->getID(),task));
  rwlock.unlock();
  char Buf[zSocket::MAX_DATASIZE];
  bzero(Buf,sizeof(Buf));
  Cmd::Scene::t_fresh_MapIndex *map = (Cmd::Scene::t_fresh_MapIndex *)Buf;
  constructInPlace(map);
  struct GateMapAddExec : public SceneCallBack
  {
    Cmd::Scene::t_fresh_MapIndex *_data;
    GateMapAddExec(Cmd::Scene::t_fresh_MapIndex *data):_data(data){_data->dwSize=0;} 
    bool exec(Scene *scene)
    {
      if (scene)
      {
        _data->mps[_data->dwSize].maptempid=scene->tempid;
        _data->mps[_data->dwSize].mapx=scene->getScreenX();
        _data->mps[_data->dwSize].mapy=scene->getScreenY();
        _data->dwSize++; 
      }
      return true; 
    }
  };
  GateMapAddExec exec(map);
  SceneManager::getInstance().execEveryScene(exec);
  task->sendCmd(map,sizeof(Cmd::Scene::t_fresh_MapIndex) + map->dwSize * sizeof(Cmd::Scene::MapIndexXY));
  return true;
}

/**
 * \brief ��������ɾ��һ��������
 *
 * \param task ����������
 * \return ɾ���Ƿ�ɹ�
 */
bool SceneTaskManager::uniqueRemove(SceneTask *task)
{
  SceneTaskHashmap_iterator it;
  //SceneUserManager::getMe().removeUserByTask(task);
  rwlock.wrlock();
  it = sceneTaskSet.find(task->getID());
  if (it != sceneTaskSet.end())
  {
    sceneTaskSet.erase(it);
  }
  else
  {
    Zebra::logger->warn("SceneTaskManager::uniqueRemove");
  }
  rwlock.unlock();
  /*
  char Buf[zSocket::MAX_DATASIZE];
  bzero(Buf,sizeof(Buf));
  Cmd::Scene::t_Remove_MapIndex *map = (Cmd::Scene::t_Remove_MapIndex *)Buf;
  constructInPlace(map);
  struct GateMapRemoveExec : public SceneCallBack
  {
    Cmd::Scene::t_Remove_MapIndex *_data;
    GateMapRemoveExec(Cmd::Scene::t_Remove_MapIndex *data):_data(data){_data->dwSize=0;} 
    bool exec(Scene *scene)
    {
      if (scene)
      {
        _data->dwMapTempID[_data->dwSize]=scene->tempid;
        _data->dwSize++; 
      }
      return true; 
    }
  };
  GateMapRemoveExec exec(map);
  SceneManager::getInstance().execEveryScene(exec);
  task->sendCmd(map,sizeof(Cmd::Scene::t_Remove_MapIndex) + map->dwSize * sizeof(DWORD));
  // */
  return true;
}

/**
 * \brief �������������и��ݷ�������Ż�ȡһ����������
 *
 * \param wdServerID ���������
 * \return ����������
 */
SceneTask *SceneTaskManager::uniqueGet(WORD wdServerID)
{
  SceneTaskHashmap_iterator it;
  SceneTask *ret=NULL;
  rwlock.rdlock();
  it = sceneTaskSet.find(wdServerID);
  if (it != sceneTaskSet.end() && it->second->getState() == zTCPTask::okay)
  {
    ret=it->second;
  }
  rwlock.unlock();
  return ret;
}

void SceneTaskManager::execEvery()
{
  SceneTaskHashmap_iterator it;
  SceneTask *task=NULL;
  rwlock.rdlock();
  it = sceneTaskSet.begin();
  for (; it != sceneTaskSet.end() ; it ++)
  {
    task=it->second;
    if (!task->checkRecycle())
    {
      task->doCmd();
    }
  }
  rwlock.unlock();

}
bool SceneTaskManager::broadcastCmd(const void *pstrCmd,const int nCmdLen)
{
  SceneTaskHashmap_iterator it;
  SceneTask *task=NULL;
  rwlock.rdlock();
  it = sceneTaskSet.begin();
  for (; it != sceneTaskSet.end() ; it ++)
  {
    task=it->second;
    task->sendCmd(pstrCmd,nCmdLen);
  }
  rwlock.unlock();

  return true;
}
