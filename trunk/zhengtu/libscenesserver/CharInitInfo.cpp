/**
 * \brief ��ʼ��ɫ��Ϣ
 *
 * 
 */
#include <zebra/ScenesServer.h>

CharInitInfo *CharInitInfo::instance = NULL;

/**
 * \brief ��ʼ��������Ϣ(��ȡ�����ļ� initcharinfo.xml)
 *
 *
 * \return ��ʼ���Ƿ�ɹ�
 */
bool CharInitInfo::init()
{
  final();

  rwlock.wrlock();
  zXMLParser xml;
  if (!xml.initFile(Zebra::global["confdir"] + "initcharinfo.xml"))
  {
    Zebra::logger->error("����initcharinfo.xmlʧ��");
    return false;
  }

  xmlNodePtr root = xml.getRootNode("CharInfo");
  if (root)
  {
    xmlNodePtr node = xml.getChildNode(root,"object");
    while(node)
    {
      if (strcmp((char *)node->name,"object") == 0)
      {
        InitObject object;
        xml.getNodePropNum(node,"id",&object.id,sizeof(object.id));
        xml.getNodePropStr(node,"name",object.name,sizeof(object.name));
        xml.getNodePropNum(node,"localeID",&object.localeID,sizeof(object.localeID));
        xml.getNodePropNum(node,"number",&object.num,sizeof(object.num));
        xml.getNodePropNum(node,"profession",&object.profession,sizeof(object.profession));
        //Zebra::logger->debug("%u,%s,%u,%u,%u",object.id,object.name,object.localeID,object.num,object.profession);
        objects.insert(ObjectsContainer::value_type(object.profession,object));
      }

      node = xml.getNextNode(node,NULL);
    }
    Zebra::logger->info("��ʼ����ʼ��Ʒϵͳ�ɹ�");
    rwlock.unlock();
    return true;
  }
  rwlock.unlock();

  Zebra::logger->error("���ؽ�ɫ��ʼ��Ϣ�����ļ�ʧ��");
  return false;
}

/**
 * \brief ����ְҵ�õ�������Ʒ
 *
 *
 * \param profession: ְҵ
 * \param objs: ��ְҵ��������Ʒ(���)
 */
void CharInitInfo::get(const WORD profession,InitObjectVector &objs)
{
  rwlock.rdlock();
  //__gnu_cxx::pair<ObjectsContainer::const_iterator,ObjectsContainer::const_iterator> os = objects.equal_range(profession);
  for(ObjectsContainer::const_iterator it = objects.begin();it!=objects.end();it++)
  {
    if (profession & it->second.profession)
      objs.push_back((*it).second);

  }
  /*
  for(ObjectsContainer::const_iterator it = os.first; it != os.second; it++)
  {
    objs.push_back((*it).second);
  }
  // */
  rwlock.unlock();

  //Zebra::logger->debug("%u,%u",profession,objs.size());
}

/**
 * \brief ���������Ʒ�б�
 *
 *
 */
void CharInitInfo::final()
{
  rwlock.wrlock();
  objects.clear();
  rwlock.unlock();
}

