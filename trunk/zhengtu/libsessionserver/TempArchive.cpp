/**
 * \brief ��ʱ�����洢
 *
 * 
 */

#include <zebra/SessionServer.h>

GlobalTempArchiveIndex *GlobalTempArchiveIndex::_instance = NULL;
GlobalTempArchiveIndex::GlobalTempArchiveIndex()
{
}
GlobalTempArchiveIndex::~GlobalTempArchiveIndex()
{
}
/**
 * \brief �õ�Ψһʵ��
 *
 *
 * \return 
 */
GlobalTempArchiveIndex *GlobalTempArchiveIndex::getInstance()
{
  if (_instance == NULL)
  {
    _instance = new GlobalTempArchiveIndex; 
  }
  return _instance;
}
/**
 * \brief ɾ��Ψһʵ��
 *
 *
 * \return 
 */
void GlobalTempArchiveIndex::delInstance()
{
  SAFE_DELETE(_instance);
}
/**
 * \brief ��ȡһ���û�����ʱ����
 *
 *
 * \param id: �û�id
 * \param out: ���buf
 * \param outSize: ������ݴ�С
 * \return ��ȡ�Ƿ�ɹ�
 */
bool GlobalTempArchiveIndex::readTempArchive(DWORD id,char *out,DWORD &outSize)
{
  checkOverdue();
  mlock.lock();
  TempArchive_iterator iter = tempArchive.find(id);
  if (iter == tempArchive.end())
  {
    outSize = 0;
    mlock.unlock();
    return false;
  }
  if (outSize < iter->second->dwSize || !out)
  {
    outSize = 0 ;
    mlock.unlock();
    return false;
  }
  outSize = iter->second->dwSize;
  //Zebra::logger->debug("��ʱ�����������ݴ�С%u",outSize);
  bcopy(iter->second->data,out,outSize,MAX_TEMPARCHIVE_SIZE);
  remove(iter);
  mlock.unlock();
  return true;
}
/**
 * \brief ����Ƿ��й��ڵĵ���
 *
 *
 * \return 
 */
void GlobalTempArchiveIndex::checkOverdue()
{
  TempArchive_iterator del_iter;
  zRTime ctv;
  mlock.lock();
  for(TempArchive_iterator iter = tempArchive.begin() ; iter != tempArchive.end() ;)
  {
    //Zebra::logger->debug("��ʱ�浵������ʧЧʱ�仹ʣ%u��",ctv.sec() - iter->second->createtime.sec());
    if (ctv.sec() - iter->second->createtime.sec() > 120)
    {
      //Zebra::logger->info("��ʱ�浵���ݹ���ʧЧ%u",iter->second->id);
      del_iter = iter;
      iter ++;
      remove(del_iter);
      continue;
    }
    iter ++;
  }
  mlock.unlock();
}
/**
 * \brief ����һ���û�����ʱ����
 *
 *
 * \param id: �û�id
 * \param data: ���buf
 * \param dwSize: ������ݴ�С
 * \return ��ȡ�Ƿ�ɹ�
 */
bool GlobalTempArchiveIndex::writeTempArchive(DWORD id,char *data,DWORD  dwSize)
{
  //Zebra::logger->debug("��ʱ�浵�������ݴ�С%u",dwSize);
  checkOverdue();
  
  mlock.lock();
  TempArchive_iterator iter = tempArchive.find(id);
  if (iter != tempArchive.end())
  {
    remove(iter);
  }
  char *buf = new char[sizeof(TempArchive) + dwSize];
  if (!buf)
  {
    mlock.unlock();
    return false;
  }
  TempArchive *ta = (TempArchive *)buf;
  ta->id = id ;
  zRTime ctv;
  ta->createtime = ctv;
  ta->dwSize = dwSize;
  bcopy(data,ta->data,dwSize,sizeof(TempArchive) + dwSize - sizeof(TempArchive) );
  bool inserted = tempArchive.insert(TempArchive_value_type(ta->id,ta)).second;
  mlock.unlock();
  if (!inserted)
  {
    SAFE_DELETE_VEC(buf);
  }

  return inserted;
}
/**
 * \brief ɾ��һ���û�������ռ�õĿռ�
 *
 *
 * \param del_iter: ��Ҫɾ���ĵ���
 * \return 
 */
void GlobalTempArchiveIndex::remove(TempArchive_iterator del_iter)
{
  char *tmp = (char*)del_iter->second;
  SAFE_DELETE_VEC(tmp);
  tempArchive.erase(del_iter);
}
