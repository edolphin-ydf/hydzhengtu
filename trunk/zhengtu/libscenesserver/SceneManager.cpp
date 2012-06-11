/**
 * \brief SceneManager��ʵ��
 *
 * 
 */
#include <zebra/ScenesServer.h>

std::map<WORD, stRangMap*> RangMapData;
std::vector<DWORD> BmapBaseID;

using namespace Cmd::Session;
using namespace Zebra;
///SceneManager��Ψһʵ��
SceneManager *SceneManager::sm(new SceneManager());

/**
 * \brief ����һ��ΨһID
 *
 * \param tempid �����ȡ�õ�ID
 * \return �Ƿ�ɹ�
 */
bool SceneManager::getUniqeID(DWORD &tempid)
{
  tempid=sceneUniqeID->get();
  return (tempid!=sceneUniqeID->invalid());
}

/**
 * \brief �ͷ�ΨһID
 *
 * \param tempid Ҫ�ͷŵ�ID
 */
void SceneManager::putUniqeID(const DWORD &tempid)
{
  sceneUniqeID->put(tempid);
}

/**
 * \brief ����һ������ΨһID
 *
 * \param tempid �����ȡ�õ�ID
 * \return �Ƿ�ɹ�
 */
bool SceneManager::getTeamID(DWORD &tempid)
{
  tempid=sceneTeamID->get();
  return (tempid!=sceneTeamID->invalid());
}

/**
 * \brief �ͷŶ���ΨһID
 *
 * \param tempid Ҫ�ͷŵ�ID
 */
void SceneManager::putTeamID(const DWORD &tempid)
{
  sceneTeamID->put(tempid);
}

/**
 * \brief ���캯��
 *
 */
SceneManager::SceneManager()
{
  inited=false;
  newzone=false; 
  ScenTeamMap.clear();
}

/**
 * \brief ��������
 *
 */
SceneManager::~SceneManager()
{
	if(!ScenTeamMap.empty())
	{
		std::map<DWORD, TeamManager*>::iterator iter;
		for(iter=ScenTeamMap.begin(); iter!=ScenTeamMap.end(); iter++)
		{
			delete iter->second;
		}

		ScenTeamMap.clear();
	}

	final();
}

/**
 * \brief �õ�SceneManager��ָ��
 * ���ָ��Ϊ0����г�ʼ��
 *
 */
SceneManager & SceneManager::getInstance()
{
	if (sm==NULL)
		sm=new SceneManager();
	return *sm;
}

/**
 * \brief ɾ��SceneManager
 *
 */
void SceneManager::delInstance()
{
  if (sm!=NULL)
  {
    sm->final();
    SAFE_DELETE(sm);
  }
}

/**
 * \brief �ͷ������Ѿ����صĵ�ͼ
 *
 */
void SceneManager::final()
{
  if (inited)
  {
    inited=false;
    unloadAllScene();
  }
}

/**
 * \brief ��ʼ��
 * �������е�ͼ
 *
 */
bool SceneManager::init()
{
	if (inited) return inited;

	//Ϊÿ���������������ɲ��ཻ��ĳ�����ʱID������,��С�Ĵ�10000��ʼ,ÿ����49998��ID����
	DWORD firstTempID=10000+(ScenesService::getInstance().getServerID()%100)*50000;
	sceneUniqeID=new zUniqueDWORDID(firstTempID,firstTempID+49998);

	//sky ���ɲ��ཻ�ĳ���������ʱID��������С��10000��ʼ,ÿ����4998��ID����
	firstTempID = 500000+(ScenesService::getInstance().getServerID()%100)*5000;
	sceneTeamID=new zUniqueDWORDID(firstTempID,firstTempID+4998);

	// ��ʼ�����е�ͼ
	zXMLParser parser;
	if (parser.initFile(Zebra::global["confdir"] + "scenesinfo.xml"))
	{
		xmlNodePtr root=parser.getRootNode("ScenesInfo");
		xmlNodePtr countryNode=parser.getChildNode(root,"countryinfo");
		if (countryNode)
		{
			xmlNodePtr subnode = parser.getChildNode(countryNode,"country");
			while(subnode)
			{
				if (strcmp((char*)subnode->name,"country") == 0)
				{
					CountryInfo info;
					bzero(&info,sizeof(info));
					parser.getNodePropNum(subnode,"id",&info.id,sizeof(info.id));
					parser.getNodePropStr(subnode,"name",info.name,sizeof(info.name));
					parser.getNodePropNum(subnode,"mapID",&info.mapid,sizeof(info.mapid));
					parser.getNodePropNum(subnode,"function",&info.function,sizeof(info.function));
					Zebra::logger->info("���ع�������(%u,%s,%u,%u)",info.id,info.name,info.mapid,info.function);
					country_info.insert(CountryMap_value_type(info.id,info));
				}
				subnode = parser.getNextNode(subnode,NULL);
			}
		}
		xmlNodePtr mapNode=parser.getChildNode(root,"mapinfo");
		if (mapNode)
		{
			xmlNodePtr subnode = parser.getChildNode(mapNode,"map");
			while(subnode)
			{
				if (strcmp((char*)subnode->name,"map") == 0)
				{
					MapInfo info;
					bzero(&info,sizeof(info));
					parser.getNodePropNum(subnode,"mapID",&info.id,sizeof(info.id));
					parser.getNodePropStr(subnode,"name",info.name,sizeof(info.name));
					parser.getNodePropStr(subnode,"fileName",info.filename,sizeof(info.filename));
					parser.getNodePropNum(subnode,"backto",&info.backto,sizeof(info.backto));
					parser.getNodePropNum(subnode,"backtocity",&info.backtoCity,sizeof(info.backtoCity));
					parser.getNodePropNum(subnode,"foreignerbackto",&info.foreignbackto,sizeof(info.foreignbackto));
					parser.getNodePropNum(subnode,"countrydarebackto",&info.countrydarebackto,
						sizeof(info.countrydarebackto));

					parser.getNodePropNum(subnode,"countrydefbackto",&info.countrydefbackto,
						sizeof(info.countrydefbackto));
					parser.getNodePropNum(subnode,"pklevel",&info.pklevel,
						sizeof(info.pklevel));



					parser.getNodePropNum(subnode,"commoncountrybackto",&info.commoncountrybackto,sizeof(info.commoncountrybackto));
					parser.getNodePropNum(subnode,"commonuserbackto",&info.commonuserbackto,sizeof(info.commonuserbackto));
					parser.getNodePropNum(subnode,"backtodare",&info.backtodare,sizeof(info.backtodare));
					parser.getNodePropNum(subnode,"function",&info.function,sizeof(info.function));
					parser.getNodePropNum(subnode,"level",&info.level,sizeof(info.level));
					parser.getNodePropNum(subnode,"exprate",&info.exprate,sizeof(info.exprate));
					map_info.insert(MapMap_value_type(info.id,info));
				}
				subnode = parser.getNextNode(subnode,NULL);
			}
		}
		xmlNodePtr serverNode=parser.getChildNode(root,"server");
		while (serverNode) {
			int id = 0;
			parser.getNodePropNum(serverNode,"id",&id,sizeof(id));
			if (ScenesService::getInstance().getServerID() == id) {
				int mapCount=0;
				xmlNodePtr countryNode=parser.getChildNode(serverNode,"country");
				while(countryNode)
				{
					DWORD countryid=0;
					parser.getNodePropNum(countryNode,"id",&countryid,sizeof(countryid));
					xmlNodePtr mapNode=parser.getChildNode(countryNode,"map");
					while(mapNode)
					{
						//TODO �ж��Ƿ���Լ��������ͼ
						//���ص�ͼ
						//sky ����id�Ѿ����ǵ�����������,���������ַ�������ʽ������
						char s[MAX_NAMESIZE];
						if (!parser.getNodePropStr(mapNode,"mapID",s,sizeof(s)))
						{
							Zebra::logger->error("�õ���ͼ���ʧ��");
							return inited;
						}

						//sky ��������ַ����Ƿ���һ����̬�����ı�
						Scene *loaded = NULL;

						//sky ������Ƕ�̬�����ı���ֱ������һ����ͼ
						if(!strstr(s, ":"))
						{
							WORD mapid = atoi(s);
							loaded = loadScene(Scene::STATIC,countryid,mapid);
						}
						else
						{
							//sky �����ַ��������ɶ�̬��ͼ������ŵ�ȫ��������
							stRangMap * RangMap = new stRangMap;
							WORD Index = 0, minID, maxID;
							sscanf(s, "%d:%d-%d", &Index, &minID, &maxID);
							RangMap->init(countryid, Index, minID, maxID);

							if(RangMap)
							{
								RangMapData[Index] = RangMap;
								//sky ������һ�ŵ�ͼ��������
								loaded = loadBattleScene(Index);
							}
							else
							{
								printf("������̬��ͼ���� ����%d %d:%d-%d ʧ��",
								countryid, Index, minID, maxID);
							}
						}

						// ע���ͼ
						if (loaded)
						{
							printf("��session����ע����Ϣ(%s-%d-%d)\n",loaded->name,loaded->id,loaded->tempid);
							Zebra::logger->info("����%s(%d,%d)�ɹ�",loaded->name,loaded->id,loaded->tempid);
							t_regScene_SceneSession regscene;
							regscene.dwID=loaded->id;
							//Zebra::logger->info("[��ͼ��ʵID]:%d",loaded->id&0x0FFF);
							regscene.dwTempID=loaded->tempid;
							strncpy(regscene.byName,loaded->name,MAX_NAMESIZE);
							strncpy(regscene.fileName,loaded->getFileName(),MAX_NAMESIZE);
							regscene.dwCountryID = countryid;
							regscene.byLevel = loaded->getLevel();
							sessionClient->sendCmd(&regscene,sizeof(regscene));
							mapCount++;
						}
						else
						{
							return inited;
						}
						mapNode=parser.getNextNode(mapNode,"map");
					}
					countryNode=parser.getNextNode(countryNode,"country");
				}
				Zebra::logger->info("ScenesServer id=%d����%d�ŵ�ͼ.",id,mapCount);

				//sky ����ս��������Լ�� id ����BATTLEFIELD_SCENE_ID�����϶�Ϊս��������
				if(ScenesService::getInstance().getServerID() > BATTLEFIELD_SCENE_ID)
				{
					//sky ֪ͨsession���Լ���Ϊ�ɷ���ս����ͼ
					t_Scene_MeIsBattlefield cmd;
					sessionClient->sendCmd(&cmd, sizeof(t_Scene_MeIsBattlefield));
					printf("ScenesServer %d Ϊս��������,�����ϱ�session��\n", ScenesService::getInstance().getServerID());
				}

				//sky ����ó���ˢ�µ�NPC������
				printf("ScenesServer id=%d ���� %d �ŵ�ͼ ˢ��NPC����:%u\n", id, mapCount, MapdwNpcNum);
			}
			else{
				Zebra::logger->info("skip id=%d != %d.",id,ScenesService::getInstance().getServerID());
			}
			serverNode=parser.getNextNode(serverNode,"server");
		}
		inited=true;
	}
	else
		Zebra::logger->warn("SceneManager ���������ļ�ʧ��.");
	//ˢ����������Ҫ�ĵ�ͼ��Ϣ
	freshEverySceneField(); 
	return inited;
}

/**
 * \brief �������ֵĵ�����ָ��Ļص�
 * �����Ǳ�������scene���Ƚ�����
 * 
 */
class GetSceneByFileName:public SceneCallBack
{
  public:
    ///�ҵ���sceneָ��
    Scene *ret;
    ///Ҫ�ҵĳ�������
    const char *name;

    /**
     * \brief ���캯��
     *
     * 
     * \param name Ҫ���ҵĳ�������
     * \return 
     */
    GetSceneByFileName(const char *name) : ret(NULL),name(name) {};

    /**
     * \brief  ִ�в��ҵķ���
     *
     *
     * \param scene ����ָ��
     * \return �Ƿ��������
     */
    bool exec(Scene *scene)
    {
      if (strncmp(scene->getFileName(),name,MAX_NAMESIZE)==0)
      {
        ret=scene;
        return false;
      }
      else
        return true;
    }
};

/**
 * \brief �����ļ������ҵ�����ָ��
 *
 * \param name Ҫ�ҵĳ�������
 * \return Ҫ�ҵĳ���ָ��,ʧ�ܷ���0
 */
Scene * SceneManager::getSceneByFileName( const char * name)
{
  GetSceneByFileName gsfn(name);
  execEveryScene(gsfn);
  return gsfn.ret;
}

/**
 * \brief ���������ҵ�����ָ��
 *
 * \param name Ҫ�ҵĳ�������
 * \return Ҫ�ҵĳ���ָ��,ʧ�ܷ���0
 */
Scene * SceneManager::getSceneByName( const char * name)
{
  rwlock.rdlock();
  Scene *ret =(Scene *)getEntryByName(name);
  rwlock.unlock();
  return ret;
}

/**
 * \brief ��������ID�ҵ�����ָ��
 *
 * \param tempid Ҫ�ҵĳ�������ʱid
 * \return Ҫ�ҵĳ���ָ��,ʧ�ܷ���0
 */
Scene * SceneManager::getSceneByTempID( DWORD tempid)
{
  rwlock.rdlock();
  Scene * ret = (Scene *)getEntryByTempID(tempid);
  rwlock.unlock();
  return ret;
}

/**
 * \brief ����id�ҵ�����ָ��
 *
 * \param id Ҫ�ҵĳ���id
 * \return Ҫ�ҵĳ���ָ��,ʧ�ܷ���0
 */
Scene * SceneManager::getSceneByID( DWORD id)
{
  rwlock.rdlock();
  Scene * ret = (Scene *)getEntryByID(id);
  rwlock.unlock();
  return ret;
}

DWORD SceneManager::getMapId(DWORD countryid,DWORD mapid)
{
  MapMap_iter map_iter = map_info.find(mapid);
  if (map_iter == map_info.end())
  {
    return 0;
  }
  
  CountryMap_iter country_iter = country_info.find(countryid);
  
  if (country_iter == country_info.end())
  {
    return 0;
  }
  
  return (country_iter->second.id << 16) + map_iter->second.id;


}

/**
 * \brief ����һ����ͼ
 *
 * \param type ��ͼ����,��̬/��̬
 * \param countryid ����id
 * \param mapid ��ͼid
 * \return �¼��صĳ���ָ��
 */
Scene * SceneManager::loadScene(int type/*Scene::SceneType type*/,DWORD countryid,DWORD mapid)
{
  Zebra::logger->info("SceneManager::loadScene type=%d,countryid=%d,mapid=%d",type,countryid,mapid);
  zEntry *s=NULL;
  switch(type)
  {
	  case 0://Scene::STATIC:
        s=new StaticScene();
      break;
	  case 1://Scene::GANG:
		  s=new GangScene();
      break;
    default:
        Zebra::logger->error("δ֪��������");
        return false;
  }
  rwlock.wrlock();
  bool ret=((Scene *)s)->init(countryid,mapid);
  if (ret)
  {
    ret=addEntry(s);
    if (!ret)
      Zebra::logger->error("SceneManager::loadScene addEntry[%s]ʧ��.",s->name);
    else Zebra::logger->info("SceneManager::loadScene[%s]�ɹ�",s->name);
  }
  else
    Zebra::logger->error("SceneManager::loadScene init[%s]ʧ��.",s->name);
  rwlock.unlock();
  if (!ret)
  {
    SAFE_DELETE(s);
  }

  return (Scene *)s;
}

/**
* \brief ��̬����һ����ͼ
*
* \param type ��ͼ����,��̬/��̬
* \param countryid ����id
* \param baseid ��ͼԴid
* \return �¼��صĳ���ָ��
*/
Scene * SceneManager::loadBattleScene(DWORD baseid)
{
	if(baseid == 0)
		return NULL;

	zEntry *s=NULL;
	s=new GangScene();

	std::map<WORD, stRangMap*>::iterator iter;
	iter = RangMapData.find(baseid);

	if(iter == RangMapData.end())
		return NULL;

	DWORD countryid = iter->second->GetCountryid();
	DWORD mapid = 0;
	if(!iter->second->getUniqeID(mapid))
	{
		printf("%d ����ս�������ѵ��������!�޷��ٷ���", baseid);
		return NULL;
	}

	rwlock.wrlock();
	bool ret = ((GangScene*)s)->GangSceneInit(countryid, baseid, mapid);

	//sky ����ս����������
	//((GangScene*)s)->InitData();

	if (ret)
	{
		ret=addEntry(s);
		if (!ret)
			Zebra::logger->error("SceneManager::loadScene addEntry[%s]ʧ��.",s->name);
		else 
			Zebra::logger->info("SceneManager::loadScene[%s]�ɹ�",s->name);
	}
	else
		Zebra::logger->error("SceneManager::loadScene init[%s]ʧ��.",s->name);
	rwlock.unlock();
	if (!ret)
	{
		SAFE_DELETE(s);
	}

	return (Scene *)s;
}

/**
 * \brief ��������ж��һ�ŵ�ͼ
 *
 * \param name Ҫж�صĵ�ͼ����
 */
void SceneManager::unloadScene(std::string &name)
{
  zEntry * ret=NULL;
  rwlock.wrlock();
  if (zEntryName::find(name.c_str(),ret))
    removeEntry(ret);
  SAFE_DELETE(ret);
  rwlock.unlock();
  Zebra::logger->debug("SceneManager::unloadScene");
}

/**
 * \brief ���ݳ���ָ��ж��һ�ŵ�ͼ
 *
 * \param scene Ҫж�صĵ�ͼָ��
 */
void SceneManager::unloadScene(Scene * &scene)
{
  if (scene==NULL) return;
  rwlock.wrlock();
  removeEntry((zEntry *)scene);
  SAFE_DELETE(scene);
  rwlock.unlock();
  Zebra::logger->debug("SceneManager::unloadScene");
}

/**
 * \brief ж��ȫ����ͼ
 *
 */
void SceneManager::unloadAllScene()
{
  zEntry * ret=NULL;
  rwlock.wrlock();
  while(zEntryName::findOne(ret))
  {
    removeEntry(ret);
    SAFE_DELETE(ret);
  }
  rwlock.unlock();
  Zebra::logger->debug("SceneManager::unloadAllScene");
}

/**
 * \brief ��鲢ɾ������Ϊremove�ĳ���
 * �����Ǳ������г���,���������remove��־,���������npc�����,Ȼ��ɾ������
 * �÷����ڳ�����ѭ����ִ��
 *
 */
void SceneManager::checkUnloadOneScene()
{
  for(zEntryTempID::hashmap::iterator it=zEntryTempID::ets.begin();it!=zEntryTempID::ets.end();it++)
  {
    Scene *scene = (Scene *)it->second;
    if (scene->getRunningState() == SCENE_RUNNINGSTATE_REMOVE)
    {
      Zebra::logger->debug("ж�س���%s",scene->name);
      SceneNpcManager::getMe().removeNpcInOneScene(scene);
      scene->removeSceneObjectInOneScene();
      unloadScene(scene);
      return ;
    }
  }
}

struct EveryMapExec : public execEntry<SceneManager::MapInfo>
{
  Scene *_scene;
  EveryMapExec(Scene *s):_scene(s)
  {
  }
  bool exec(SceneManager::MapInfo *info)
  {
    if (info->function & 0x2)
    {
      char buf[MAX_NAMESIZE];
      bzero(buf,sizeof(buf));
      if (SceneManager::getInstance().buildMapName(_scene->getCountryID(),info->name,buf))
      {
        _scene->addMainMapName(buf);
      }
    }
    if (info->function & 0x20)
    {
      char buf[MAX_NAMESIZE];
      bzero(buf,sizeof(buf));
      if (SceneManager::getInstance().buildMapName(_scene->getCountryID(),info->name,buf))
      {
        _scene->addIncMapName(buf);
      }
    }
    return true;
  }
};
void SceneManager::freshEverySceneField()
{
  struct getAllMapExec :public SceneCallBack
  {
    Scene *_old_scene;
    getAllMapExec(Scene *s):_old_scene(s)
    {
    }
    bool exec(Scene *scene)
    {
      //Zebra::logger->debug("%d,%d,%d,%s",scene->getCountryID(),_old_scene->getCountryID(),scene->isMainCity(),scene->getFileName());
      if (/*scene != _old_scene && */_old_scene->isMainCity() && scene->getCountryID() == _old_scene->getCountryID() && scene->isField() && _old_scene->getWayPoint(scene->getFileName()))
      {
        _old_scene->addFieldMapName(scene->name);
      }
      return true;
    }
  };
  struct EverySceneExec :public SceneCallBack
  {
    bool exec(Scene *scene)
    {
      scene->clearMainMapName();
      EveryMapExec exec1(scene); 
      SceneManager::getInstance().execEveryMap(exec1);
      scene->clearFieldMapName();
      getAllMapExec exec(scene);
      SceneManager::getInstance().execEveryScene(exec);
      return true;
    }
  };
  EverySceneExec exec; 
  SceneManager::getInstance().execEveryScene(exec);
}
/**
 * \brief ��ÿ������ִ�лص�����
 *
 * \param callback Ҫִ�еĻص�����
 */
void SceneManager::execEveryScene(SceneCallBack &callback)
{
  for(zEntryTempID::hashmap::iterator it=zEntryTempID::ets.begin();it!=zEntryTempID::ets.end();it++)
  {
    if (!callback.exec((Scene *)it->second))
    {
      //mlock.unlock();
      return;
    }
  }
}


/**
 * \brief ���ݹ������ֵõ�����id
 *
 * \param name Ҫ�õ�id�Ĺ�������
 * \return �ҵ���id,ʧ�ܷ���0
 */
DWORD SceneManager::getCountryIDByCountryName(const char *name)
{
  SceneManager::CountryMap_iter country_iter = SceneManager::getInstance().country_info.begin();
  for(; country_iter != SceneManager::getInstance().country_info.end() ; country_iter ++)
  {
    if (strcmp(name,country_iter->second.name) == 0)
    {
      return country_iter->first;
    }
  }
  return 0;
}

/**
 * \brief ��ÿ����������ִ�лص�����
 *
 * \param callback Ҫִ�еĻص�����
 */
/*
void SceneManager::execEveryMap(MapCallBack &callback)
{
  SceneManager::MapMap_iter map_iter = SceneManager::getInstance().map_info.begin();
  for(; map_iter != SceneManager::getInstance().map_info.end() ; map_iter ++)
  {
    callback.exec(map_iter->second);
  }
}
// */
/**
 * \brief ���ݹ���id�õ���������
 *
 * \param id Ҫ�ҵĹ���id
 * \return �ҵ��Ĺ�������,ʧ�ܷ���0
 */
const char * SceneManager::getCountryNameByCountryID(DWORD id)
{
  SceneManager::CountryMap_iter country_iter = SceneManager::getInstance().country_info.begin();
  for(; country_iter != SceneManager::getInstance().country_info.end() ; country_iter ++)
  {
    if (country_iter->first == id)
    {
      return country_iter->second.name;
    }
  }
  return 0;
}

/**
 * \brief ���ݵ�ͼ���ֵõ���ͼid
 *
 * \param name Ҫ�ҵĵ�ͼ����
 * \return �ҵ���id,ʧ�ܷ���0
 */
DWORD SceneManager::getMapIDByMapName(const char *name)
{
  const char *p = strstr(name,"��");
  if (p)
    p += strlen("��");
  else
    p = name;

  SceneManager::MapMap_iter map_iter = SceneManager::getInstance().map_info.begin();
  for(; map_iter != SceneManager::getInstance().map_info.end() ; map_iter ++)
  {
    if (strcmp(p,map_iter->second.name) == 0)
    {
      return map_iter->first;
    }
  }
  return 0;
}

/**
 * \brief ���ݹ���id�͵�ͼid��ɳ���name
 *
 */
bool SceneManager::buildMapName(DWORD countryid,DWORD mapid,char *out)
{
  const char *c = getCountryNameByCountryID(countryid);
  const char *m = map_info[mapid].name;
  if (c && m)
  {
    sprintf(out,"%s��%s",c,m);
    return true;
  }
  return false;
}
 

/**
 * \brief ���ݹ���id�͵�ͼname��ɳ���name
 *
 */
bool SceneManager::buildMapName(DWORD countryid,const char *in,char *out)
{
  const char *c = getCountryNameByCountryID(countryid);
  if (c && in)
  {
    sprintf(out,"%s��%s",c,in);
    return true;
  }
  return false;
}
/**
 * \brief ���ݹ���id�͵�ͼid��ɳ���id
 *
 */
DWORD SceneManager::buildMapID(DWORD countryid,DWORD mapid)
{
  return (countryid << 16) + mapid;
}
bool SceneManager::isNewZoneConfig()
{
  return newzone && (!newzon_vec.empty());
}
void SceneManager::setNewZoneConfig(bool type)
{
  newzone=type;
  if (newzone==false)
  {
    newzon_vec.clear();
  }
}
void SceneManager::addNewZonePos(DWORD x,DWORD y)
{
  newzon_vec.push_back(std::make_pair(x,y));
}
SceneManager::NewZoneVec &SceneManager::queryNewZonePos()
{
  return newzon_vec;
}
bool SceneManager::randzPosNewZone(Scene *intoScene,zPos &findedPos)
{
  bool founded=false;
  int i=0;
  while(!founded && i < (int)newzon_vec.size())
  {
    int which = randBetween(0,newzon_vec.size() - 1);
    zPos initPos;
    int x = randBetween(0,10);
    int y = randBetween(0,10);
    initPos.x =newzon_vec[which].first + 5 - x;
    initPos.y =newzon_vec[which].second + 5 - y;
    founded = intoScene->findPosForUser(initPos,findedPos);
  }
  return founded;
}


TeamManager* SceneManager::GetMapTeam(DWORD TeamID)
{
	if(ScenTeamMap.empty() || ScenTeamMap[TeamID] == 0)
		return NULL;
	else
		return ScenTeamMap[TeamID];
}

bool SceneManager::SceneNewTeam(SceneUser *pUser)
{
	if (pUser->TeamThisID == 0)
	{
		//sky ���������ΨһID
		DWORD teamid;
		if(!getTeamID(teamid))
		{
			Channel::sendSys(pUser,Cmd::INFO_TYPE_GAME,"�������ΨһIDʧ�ܣ����齨��ʧ�ܣ�");
			return false;
		}

		TeamManager * team = new TeamManager();
		if(team == NULL)
		{
			Zebra::logger->debug("�ڴ治��!���ʧ��");
			putTeamID(teamid);
			return false;
		}

 		if (team->addMemberByTempID(pUser,pUser->tempid))
		{
			team->setTeamtempId(teamid);
			team->setLeader(pUser->tempid);

			ScenTeamMap[teamid] = team;

			pUser->TeamThisID = teamid;

			Cmd::stAddTeamMemberUserCmd ret;
			ret.dwTeamID = teamid;
			ret.dwHeadID = pUser->tempid;
			ret.data.dwTempID = pUser->tempid;
			ret.data.dwMaxHealth = pUser->charstate.maxhp;
			ret.data.dwHealth = pUser->charbase.hp;
			ret.data.dwMaxMp = pUser->charstate.maxmp;
			ret.data.dwMp = pUser->charbase.mp;
			ret.data.wdFace = pUser->charbase.face;
			strncpy(ret.data.pstrName,pUser->name,MAX_NAMESIZE);
			ret.data.byHead = true;
			pUser->sendCmdToMe(&ret,sizeof(ret));

			pUser->team_mode = Cmd::TEAM_HONOR;
			pUser->reSendMyMapData();
			//session����
			ScenTeamMap[teamid]->addMemberToSession(pUser->name);

			return true;
		}
	}

	return false;
}

bool SceneManager::SceneNewTeam(Cmd::Session::t_Team_Data* send)
{
	std::map<DWORD, TeamManager*>::iterator iter;
	iter = ScenTeamMap.find(send->dwTeamThisID);
	
	if(iter != ScenTeamMap.end())
		return true;

	TeamManager * team = new TeamManager();
	if(team == NULL)
	{
		Zebra::logger->debug("�ڴ治��!�糡�������¶���ʧ��");
		return false;
	}

	team->setTeamtempId(send->dwTeamThisID);

	SceneUser * leader = SceneUserManager::getMe().getUserByID(send->LeaderID);

	if(leader)
		team->setLeader(leader->tempid);
	else
		team->setLeader( 0 );

	for(int i=0; i<send->dwSize; i++)
	{
		bool bleaber;
		if(send->Member->dwID == send->LeaderID)
			bleaber = true;
		else
			bleaber = false;

		team->addWAwayMember(&(send->Member[i]));
	}

	ScenTeamMap[send->dwTeamThisID] = team;

	return true;
}

//sky ֪ͨSessionҲɾ������������Ķ���
bool SceneManager::SceneDelTeam(DWORD TeamID)
{
	std::map<DWORD, TeamManager*>::iterator iter;
	iter = ScenTeamMap.find(TeamID);

	if(iter != ScenTeamMap.end())
	{
		/*if(iter->second)
		{
			iter->second->deleteTeam();
			delete iter->second;
		}
		
		ScenTeamMap.erase(iter);

		putTeamID(TeamID);*/

		//sky ����Sessionɾ������
		Cmd::Session::t_Team_DelTeam rev;
		rev.TeamThisID = TeamID;
		sessionClient->sendCmd(&rev, sizeof(Cmd::Session::t_Team_DelTeam));
	}

	return true;
}

//sky ɾ����ǰ�����Ķ����������Ķ���
bool SceneManager::DelMapTeam(DWORD TeamID)
{
	std::map<DWORD, TeamManager*>::iterator iter;
	iter = ScenTeamMap.find(TeamID);

	if(iter != ScenTeamMap.end())
	{
		if(iter->second)
		{
			iter->second->deleteTeam();
			delete iter->second;
		}

		ScenTeamMap.erase(iter);

		putTeamID(TeamID);
	}

	return true;
}

void SceneManager::TeamRollItme()
{
	std::map<DWORD, TeamManager*>::iterator iter;
	for(iter=ScenTeamMap.begin(); iter!=ScenTeamMap.end(); iter++)
	{
		TeamManager * team = iter->second;

		if(team)
		{
			if(team->bRoll)
				team->RollItem_A();
		}
	}
}