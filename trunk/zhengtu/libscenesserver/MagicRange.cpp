/**
 * \brief Ⱥ�����ܷ�Χ����
 *
 * 
 */

#include <zebra/ScenesServer.h>

/**
 * \brief �����������õ���������
 *
 *
 * \param center: ����������ĵ�
 * \param dir:������귽��
 * \return ��������
 */
zPos RelativePos::getAbsolutePos(const zPos &center,DWORD dir)
{
  zPos pos;

  switch(dir)
  {
    case 0:
    case 1:
      {
        pos.x = x;
        pos.y = y;
      }
      break;
    case 2:
    case 3:
      {
        pos.x = -y;
        pos.y = x;
      }
      break;
    case 4:
    case 5:
      {
        pos.x = -x;
        pos.y = -y;
      }
      break;
    case 6:
    case 7:
      {
        pos.x = y;
        pos.y = -x;
      }
      break;
    default:
      {
        pos.x = x;
        pos.y = y;
      }
      break;
  }
  pos.x += center.x;
  pos.y += center.y;
  return pos;
}

/**
 * \brief Ψһʵ��
 *
 */
MagicRangeInit *MagicRangeInit::instance = NULL;

/**
 * \brief ��ʼ�����ܷ�Χ
 *
 *
 * \return ��ʼ���Ƿ�ɹ�
 */
bool MagicRangeInit::init()
{
  final();

  rwlock.wrlock();
  zXMLParser xml;
  if (!xml.initFile(Zebra::global["confdir"] + "magicrangefile.xml"))
  {
    Zebra::logger->error("����magicrangefile.xmlʧ��");
    return false;
  }

  xmlNodePtr root = xml.getRootNode("MagicRange");
  if (root)
  {
    xmlNodePtr node = xml.getChildNode(root,"range");
    while(node)
    {
      if (strcmp((char *)node->name,"range") == 0)
      {
        DWORD type;
        DWORD dir;
        DWORD num;
        SMagicRange range_vector;
        range_vector.num = 0;
        xml.getNodePropNum(node,"type",&type,sizeof(type));
        xml.getNodePropNum(node,"dir",&dir,sizeof(dir));
        xml.getNodePropNum(node,"num",&num,sizeof(num));
        range_vector.num = num;
        xmlNodePtr subnode = xml.getChildNode(node,"pos");
        while(subnode)
        {
          if (strcmp((char*)subnode->name,"pos") == 0)
          {
            RelativePos pos;
            xml.getNodePropNum(subnode,"x",&pos.x,sizeof(pos.x));
            xml.getNodePropNum(subnode,"y",&pos.y,sizeof(pos.y));
            xml.getNodePropNum(subnode,"w",&pos.w,sizeof(pos.w));
            range_vector.lib.push_back(pos);
          }
          subnode = xml.getNextNode(subnode,NULL);
        }
        ranges.insert(MagicRangeContainer::value_type(magicrange_hash(type,dir),range_vector));
      }

      node = xml.getNextNode(node,NULL);
    }
    Zebra::logger->info("��ʼ��������Χ�ɹ�");
    rwlock.unlock();
    return true;
  }
  rwlock.unlock();

  Zebra::logger->error("���ؽǹ�����Χ�����ļ�ʧ��");
  return false;
}

/**
 * \brief ����Ⱥ�����ͺͷ���õ����������
 *
 *
 * \param type: Ⱥ������
 * \param dir: ����
 * \param range:��Χ����(���)
 */
void MagicRangeInit::get(const DWORD type,const DWORD dir,SMagicRange &range)
{
  rwlock.rdlock();
  MagicRangeContainer::const_iterator it = ranges.find(magicrange_hash(type,dir));
  if (it == ranges.end())
  {
    it = ranges.begin();
  }
  range = it->second;
  rwlock.unlock();
}

/**
 * \brief ж��
 *
 */
void MagicRangeInit::final()
{
  rwlock.wrlock();
  ranges.clear();
  rwlock.unlock();
}

/**
 * \brief ���캯��,����Ⱥ�����ͺͷ���õ�������Χ
 *
 *
 * \param type: Ⱥ������
 * \param dir: ����
 */
MagicPos::MagicPos(const DWORD type,const DWORD dir)
{
  MagicRangeInit::getInstance().get(type,dir % 2,range);
}
/**
 * \brief ִ��ÿһ��Ⱥ�����õ�
 *
 *
 * \param pos: �ص�����
 */
void MagicPos::execEvery(MagicPosExec &pos)
{
  for(MagicRange::iterator iter = range.lib.begin(); iter != range.lib.end() ; iter ++)
  {
    pos.exec(*iter);
  }
}
