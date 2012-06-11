/**
 * \brief Npc��������
 *
 * 
 */

#include <zebra/ScenesServer.h>

/// SceneNpcManager��Ψһʵ��
SceneNpcManager *SceneNpcManager::snm(NULL);

/**
 * \brief ��ʼ��
 *
 */
bool SceneNpcManager::init()
{
  return true;
}

/**
 * \brief ���캯��
 *
 */
SceneNpcManager::SceneNpcManager()
{
  if (!loadNpcCommonChatTable())
    Zebra::logger->error("��ȡnpc˵������ʧ��");
#ifdef _DEBUG  
  Zebra::logger->debug("SceneNpcManager::SceneNpcManager");
#endif    
}

SceneNpcManager::~SceneNpcManager()
{
#ifdef _DEBUG  
  Zebra::logger->debug("SceneNpcManager::~SceneNpcManager()");
#endif    

}

/**
 * \brief ����������һ��npc
 *
 *
 * \param sceneNpc Ҫ��ӵ�npcָ��
 * \return �Ƿ���ӳɹ�
 */
bool SceneNpcManager::addSceneNpc(SceneNpc *sceneNpc)
{
  rwlock.wrlock();
  bool ret=addEntry(sceneNpc);
  //addSpecialNpc(sceneNpc);
  rwlock.unlock();
  return ret;
}

/**
 * \brief �������npc
 * ����npc�������boss���й̶��ű���npc
 *
 * \param sceneNpc Ҫ��ӵ�npc
 * \return �Ƿ���ӳɹ�
 */
bool SceneNpcManager::addSpecialNpc(SceneNpc *sceneNpc,bool force)
{
  rwlock.wrlock();
  if (sceneNpc && (sceneNpc->isSpecialNpc()||force))
  {
    specialNpc.insert(sceneNpc);
#ifdef _DEBUG
    //Zebra::logger->debug("addSceneNpc(): ��������npc %s",sceneNpc->name);
#endif
    rwlock.unlock();
    return true;
  }
  rwlock.unlock();
  return false;
}

/**
 * \brief ɾ��һ��npc
 *
 *
 * \param sceneNpc Ҫɾ����npc
 * \return 
 */
void SceneNpcManager::removeSceneNpc(SceneNpc *sceneNpc)
{
  rwlock.wrlock();
  removeEntry(sceneNpc);
  rwlock.unlock();
}

/**
 * \brief ɾ��һ������npc
 *
 *
 * \param sceneNpc Ҫɾ����npc
 * \return 
 */
void SceneNpcManager::removeSpecialNpc(SceneNpc *sceneNpc)
{
  if (!sceneNpc) return;

  rwlock.wrlock();
  specialNpc.erase(sceneNpc);
#ifdef _DEBUG
  //Zebra::logger->debug("removeSpecialNpc(): ɾ������npc %s(%u)",sceneNpc->name,sceneNpc->tempid);
#endif
  rwlock.unlock();
}

/**
 * \brief �ر����еĹ���NPCΪͣ����׼��
 */
void SceneNpcManager::closeFunctionNpc()
{
  struct SearchSceneExec :public execEntry<SceneNpc>
  {
    std::vector<DWORD> del_vec;
    SearchSceneExec()
    {
    }
    bool exec(SceneNpc *n)
    {
      switch(n->npc->id)
      {
        case 522:
        case 56025:
          {
            del_vec.push_back(n->tempid);
          }
          break;
        default:
          break;
      }
      return true;
    }
  };
  SearchSceneExec exec;
  SceneNpcManager::getMe().execEveryNpc(exec);
  for(std::vector<DWORD>::iterator iter = exec.del_vec.begin() ; iter != exec.del_vec.end() ; iter ++)
  {
    SceneNpc *pNpc=SceneNpcManager::getMe().getNpcByTempID(*iter);
    if (pNpc)
    {
      DWORD tempid=0;
      pNpc->toDie(tempid);
    }
  }
}


/**
 * \brief ������ʱid�õ�npc��ָ��
 *
 *
 * \param tempid npc����ʱid
 * \return �ҵ���ָ��,ʧ�ܷ���0
 */
SceneNpc *SceneNpcManager::getNpcByTempID(DWORD tempid)
{
  rwlock.rdlock();
  SceneNpc *ret = (SceneNpc *)getEntryByTempID(tempid);
  rwlock.unlock();
  return ret;
}

/**
 * \brief �õ�SceneNpcManager��ʵ��
 *
 * \return SceneNpcManager��ʵ������
 */
SceneNpcManager &SceneNpcManager::getMe()
{
  if (snm==NULL)
  {
    snm=new SceneNpcManager();
  }
  return *snm;
}

/**
 * \brief ɾ��SceneNpcManager��Ψһʵ��
 */
void SceneNpcManager::destroyMe()
{
  SAFE_DELETE(snm);
}

/**
 * \brief ɾ��һ�������ڵ�����npc 
 *
 *
 * \param scene Ҫɾ��npc�ĳ���
 * \return 
 */
void SceneNpcManager::removeNpcInOneScene(Scene *scene)
{
#ifdef _DEBUG  
  Zebra::logger->debug("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!SceneNpcManger::removeNpcInOneScene");
#endif    

  struct UnloadSceneExec :public execEntry<SceneNpc>
  {
    Scene *scene;
    std::vector<DWORD> del_vec;
    UnloadSceneExec(Scene *s):scene(s)
    {
    }
    bool exec(SceneNpc *n)
    {
      if (n->scene->tempid == scene->tempid)
      {
        del_vec.push_back(n->tempid);
      }
      return true;
    }
  };
  UnloadSceneExec exec(scene);
  SceneNpcManager::getMe().execEveryNpc(exec);
  for(std::vector<DWORD>::iterator iter = exec.del_vec.begin() ; iter != exec.del_vec.end() ; iter ++)
  {
    SceneNpc *pNpc=SceneNpcManager::getMe().getNpcByTempID(*iter);
    if (pNpc)
    {
      SceneNpcManager::getMe().removeSceneNpc(pNpc);
      SAFE_DELETE(pNpc);
    }
  }
}

void SceneNpcManager::SpecialAI()
{
}

/**
 * \brief ��ȡnpc˵������
 *
 * \return �Ƿ��ȡ�ɹ�
 */
bool SceneNpcManager::loadNpcCommonChatTable()
{
  zXMLParser xml;
  if (!xml.initFile(Zebra::global["confdir"] + "NpcCommonChat.xml"))
  {
    Zebra::logger->error("���ܶ�ȡNpcCommonChat.xml");
    return false;
  }

  xmlNodePtr root = xml.getRootNode("root");
  if (!root) return false;

  xmlNodePtr chatNode = xml.getChildNode(root,"chat");
  int itemCount = 0;
  while (chatNode)
  {
    int type,rate;
    xml.getNodePropNum(chatNode,"type",&type,sizeof(type));
    if (0!=type)
    {
      xml.getNodePropNum(chatNode,"rate",&rate,sizeof(rate));
      NpcCommonChatRate[type] = rate;
      char str[MAX_CHATINFO];
      std::vector<std::string> strTable;
      xmlNodePtr contentNode = xml.getChildNode(chatNode,"content");
      while (contentNode)
      {
        bzero(str,sizeof(str));
        xml.getNodePropStr(contentNode,"str",str,sizeof(str));
        strTable.push_back(std::string(str));
        //Zebra::logger->debug("loadNpcCommonChatTable:%s",std::string(str).c_str());
        itemCount++;
        contentNode = xml.getNextNode(contentNode,"content");
      }
      NpcCommonChatTable[type] = strTable;
    }
    chatNode = xml.getNextNode(chatNode,"chat");
  }

  Zebra::logger->info("����npc��������ļ��ɹ�,��%d��%d��",NpcCommonChatTable.size(),itemCount);
  return true;
}

/**
 * \brief ����õ�һ��npc˵�Ļ�
 *
 * \param type Ҫȡ�õ�˵������
 * \param content ���,ȡ�õ�����
 * \return �õ���˵������
 */
bool SceneNpcManager::getNpcCommonChat(DWORD type,char * content)
{
  if (NpcCommonChatTable[type].empty() || !selectByPercent(NpcCommonChatRate[type])) return false;

  int index = randBetween(0,NpcCommonChatTable[type].size()-1);
  strncpy(content,NpcCommonChatTable[type][index].c_str(),MAX_CHATINFO-1);
  return true;
}
