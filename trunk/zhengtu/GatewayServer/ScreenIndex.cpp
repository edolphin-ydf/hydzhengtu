/**
 * \brief ������
 *
 * 
 */
#include "GatewayServer.h"

ScreenIndex::ScreenIndex(const DWORD x,const DWORD y):screenx(x),screeny(y),screenMax(x*y)
{
  const int adjust[9][2] = { {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,0} };
  //Ԥ�Ƚ�����ͼ��������
  for(DWORD j=0; j < screenMax ; j ++)
  {
    index[j];
    int nScreenX = j % screenx;
    int nScreenY = j / screenx;
    //������Χ����
    {
      zPosIVector pv;
      for(int i = 0; i < 9; i++) {
        int x = nScreenX + adjust[i][0];
        int y = nScreenY + adjust[i][1];
        if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny) {
          pv.push_back(y * screenx + x);
        }
      }
      ninescreen.insert(NineScreen_map_value_type(j,pv));
    }
    //��������仯������������
    for(int dir = 0; dir < 8; dir++)
    {
      int start,end;
      zPosIVector pv;

      if (1 == dir % 2) {
        //б����
        start = 6;
        end = 10;
      }
      else {
        //������
        start = 7;
        end = 9;
      }
      for(int i = start; i <= end; i++) {
        int x = nScreenX + adjust[(i + dir) % 8][0];
        int y = nScreenY + adjust[(i + dir) % 8][1];
        if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny) {
          pv.push_back(y * screenx + x);
        }
      }
      direct_screen[dir].insert(NineScreen_map_value_type(j,pv));
    }
    //���㷴��仯������������
    for(int dir = 0; dir < 8; dir++)
    {
      int start,end;
      zPosIVector pv;

      if (1 == dir % 2) {
        //б����
        start = 2;
        end = 6;
      }
      else {
        //������
        start = 3;
        end = 5;
      }
      for(int i = start; i <= end; i++) {
        int x = nScreenX + adjust[(i + dir) % 8][0];
        int y = nScreenY + adjust[(i + dir) % 8][1];
        if (x >= 0 && y >= 0 && x < (int)screenx && y < (int)screeny) {
          pv.push_back(y * screenx + x);
        }
      }
      reversedirect_screen[dir].insert(NineScreen_map_value_type(j,pv));
    }
  }
} 

template <class YourNpcEntry>
void ScreenIndex::execAllOfScreen(const DWORD screen,execEntry<YourNpcEntry> &callback)
{
  SceneEntry_SET &set = index[screen];
  wrlock.rdlock();
  SceneEntry_SET::iterator it = set.begin(),ed = set.end();
  for(; it != ed; ++it)
  {
    callback.exec(*it);
  }
  wrlock.unlock();
}
struct SendNineExec : public execEntry<GateUser>
{
  const void  *_cmd;
  DWORD _cmdLen;
  DWORD _type;
  char _name[MAX_NAMESIZE];
  int _sendLen;
  bool zip;
  t_StackCmdQueue cmd_queue;
  unsigned short targetDupIndex;
  SendNineExec(unsigned short _targetDupIndex,const void *cmd,const int cmdLen):_cmd(cmd),_cmdLen(cmdLen),_type(0),zip(false),
	  targetDupIndex(_targetDupIndex)
  {
  }
  bool exec(GateUser *pUser)
  {

    if(pUser->dupIndex == targetDupIndex)
	{
		if (!zip)
		{
			zip=true;
			bzero(_name,sizeof(_name));
			GateUser::cmdFilter((Cmd::stNullUserCmd *)_cmd,_type,_name,_cmdLen);
			_sendLen = zSocket::packetPackZip(_cmd,_cmdLen,cmd_queue); 
		}
		pUser->sendCmd(cmd_queue.rd_buf(),_sendLen,_type,_name,true);

	}
		return true;
  }
};

void ScreenIndex::sendCmdToNine(const DWORD posi,const void *pstrCmd,const int nCmdLen,unsigned short dupIndex)
{
  SendNineExec exec(dupIndex,pstrCmd,nCmdLen);
  const zPosIVector &pv = getNineScreen(posi);
  zPosIVector::const_iterator it = pv.begin(),ed = pv.end();
  for(; it != ed; ++it)
  {
    /*
    Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;
    if (pNullCmd->cmd != 6 && pNullCmd->para !=20)
    Zebra::logger->debug("ScreenIndex::sendCmdToNine������%d,��Ϣ:(%d,%d)",*it,pNullCmd->cmd,pNullCmd->para);
    // */
    execAllOfScreen(*it,exec);
  }
}

void ScreenIndex::sendCmdToDirect(const zPosI posi,const int direct,const void *pstrCmd,const int nCmdLen,unsigned short dupIndex)
{
  SendNineExec exec(dupIndex,pstrCmd,nCmdLen);
  const zPosIVector &pv = getDirectScreen(posi,direct);
  zPosIVector::const_iterator it = pv.begin(),ed = pv.end();
  for(; it != ed; ++it)
  {
    /*
    Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;
    if (pNullCmd->cmd != 6 && pNullCmd->para !=20)
    Zebra::logger->debug("ScreenIndex::sendCmdToDirect������%d,��Ϣ:(%d,%d)",*it,pNullCmd->cmd,pNullCmd->para);
    // */
    execAllOfScreen(*it,exec);
  }
}

void ScreenIndex::sendCmdToReverseDirect(const zPosI posi,const int direct,const void *pstrCmd,const int nCmdLen,unsigned short dupIndex)
{
  SendNineExec exec(dupIndex,pstrCmd,nCmdLen);
  const zPosIVector &pv = getReverseDirectScreen(posi,direct);
  zPosIVector::const_iterator it = pv.begin(),ed = pv.end();
  for(; it != ed; ++it)
  {
    /*
    Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;
    if (pNullCmd->cmd != 6 && pNullCmd->para !=20)
    Zebra::logger->debug("ScreenIndex::sendCmdToReverseDirect������%d,��Ϣ:(%d,%d)",*it,pNullCmd->cmd,pNullCmd->para);
    // */
    execAllOfScreen(*it,exec);
  }
}

void ScreenIndex::sendCmdToAll(const void *pstrCmd,const int nCmdLen)
{
  int _sendLen;
  t_StackCmdQueue cmd_queue;
  DWORD _type=0;
  char _name[MAX_NAMESIZE];
  bzero(_name,sizeof(_name));
  DWORD cmdLen=nCmdLen;
  GateUser::cmdFilter((Cmd::stNullUserCmd *)pstrCmd,_type,_name,cmdLen);
  _sendLen = zSocket::packetPackZip(pstrCmd,nCmdLen,cmd_queue);
  wrlock.rdlock();
  SceneEntry_SET::iterator it=all.begin(),ed = all.end();
  for(; it != ed; ++it)
  {
    (*it)->sendCmd(cmd_queue.rd_buf(),_sendLen,_type,_name,true);
  }
  wrlock.unlock();
}

struct SendNineExecExceptMe : public execEntry<GateUser>
{
  const DWORD _exceptme_id;
  const void  *_cmd;
  DWORD _cmdLen;
  DWORD _type;
  char _name[MAX_NAMESIZE];
  int _sendLen;
  t_StackCmdQueue cmd_queue;
  SendNineExecExceptMe(const DWORD exceptme_id,const void *cmd,const int cmdLen):_exceptme_id(exceptme_id),_cmd(cmd),_cmdLen(cmdLen),_type(0)
  {
    bzero(_name,sizeof(_name));
    GateUser::cmdFilter((Cmd::stNullUserCmd *)_cmd,_type,_name,_cmdLen);
    _sendLen = zSocket::packetPackZip(_cmd,_cmdLen,cmd_queue); 
  }
  bool exec(GateUser *pUser)
  {
    if (_exceptme_id != pUser->id)
      pUser->sendCmd(cmd_queue.rd_buf(),_sendLen,_type,_name,true);
    return true;
  }
};

void ScreenIndex::sendCmdToNineExceptMe(const DWORD posi,const DWORD exceptme_id,const void *pstrCmd,const int nCmdLen)
{
  SendNineExecExceptMe exec(exceptme_id,pstrCmd,nCmdLen);
  const zPosIVector &pv = getNineScreen(posi);
  zPosIVector::const_iterator it = pv.begin(),ed = pv.end();
  for(; it != ed; ++it)
  {
    /*
    Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;
    if (pNullCmd->cmd != 6 && pNullCmd->para !=20)
    Zebra::logger->debug("ScreenIndex::sendCmdToNineExceptMe������%d,��Ϣ:(%d,%d)",*it,pNullCmd->cmd,pNullCmd->para);
    // */
    execAllOfScreen(*it,exec);
  }
}

bool ScreenIndex::refresh(GateUser *e,const DWORD newIndex)
{
  zRWLock_scope_wrlock scope_wrlock(wrlock);
  if (e==NULL) return false;
  //-2 ��ʾɾ��״̬,�����Ա��������
  //-1 ��ʾ�ȴ����״̬
  if (e->getIndexKey() == (DWORD)-2 && newIndex != (DWORD)-1) return false;

  if (e->inserted)
  {
    //�Ѿ������ͼ����,ֻ������֮�������л�
    bool ret=false;

    SceneEntry_SET &pimi = index[e->getIndexKey()];
    SceneEntry_SET::iterator it = pimi.find(e);
    if (it != pimi.end())
    {
      //Zebra::logger->debug("[��ͼ����]�û��л���%s,%u,%u",e->name,e->getIndexKey(),newIndex);
      ret=true;
      pimi.erase(it);
      index[newIndex].insert(e);
      e->setIndexKey(newIndex);
    }

    return ret;
  }
  else if (newIndex != (DWORD)-1)
  {
    //��ȫ�����������
    if (all.insert(e).second)
    {
      //Zebra::logger->debug("[��ͼ����]�û����룺%s,%u,%u",e->name,e->getIndexKey(),newIndex);
      //�¼����ͼ����
      index[newIndex].insert(e);
    }
    //else
    //  Zebra::logger->error("[��ͼ����]�û�����ʧ��,�Ѿ��������У�%s,%u,%u",e->name,e->getIndexKey(),newIndex);
    e->inserted=true;
  }
  //else
  //  Zebra::logger->debug("[��ͼ����]�û����ÿɼ���״̬��%s,%u,%u",e->name,e->getIndexKey(),newIndex);

  e->setIndexKey(newIndex);
  return e->inserted;
}

void ScreenIndex::removeGateUser(GateUser *e)
{
  zRWLock_scope_wrlock scope_wrlock(wrlock);
  if (e==NULL || !e->inserted) return;

  SceneEntry_SET::iterator it = all.find(e);
  if (it != all.end())
  {
    //Zebra::logger->debug("[��ͼ����]�û�ɾ����%s,%u",e->name,e->getIndexKey());
    SceneEntry_SET &pimi = index[e->getIndexKey()];
    e->setIndexKey((DWORD)-2);
    //��ȫ��������ɾ��
    all.erase(it);
    //����������ɾ��
    pimi.erase(e);
    e->inserted=false;
  }
}

/**
 * \brief ����������ȡ��Χ9������Ļ���
 * \param posi ������
 * \param pv ��������������
 */
const zPosIVector &ScreenIndex::getNineScreen(const zPosI &posi)
{
  NineScreen_map_iter iter = ninescreen.find((DWORD)posi);
  if (iter != ninescreen.end())
  {
    return iter->second;
  }
  //��������쳣����0��ŵ�������
  return ninescreen[(DWORD)-1];
}

/**
 * \brief ����������ȡ��ǰ��3������5������Ļ���
 * \param posi ������
 * \param direct ����
 * \return ��������������
 */
const zPosIVector &ScreenIndex::getDirectScreen(const zPosI &posi,const int dir)
{
  NineScreen_map_iter iter = direct_screen[dir].find((DWORD)posi);
  if (iter != direct_screen[dir].end())
  {
    return iter->second;
  }
  //��������쳣����0��ŵ�������
  return direct_screen[dir][(DWORD)-1];
}

/**
 * \brief ����������ȡ����3������5������Ļ���
 * \param posi ������
 * \param direct ����
 * \return ��������������
 */
const zPosIVector &ScreenIndex::getReverseDirectScreen(const zPosI &posi,const int dir)
{
  NineScreen_map_iter iter = reversedirect_screen[dir].find((DWORD)posi);
  if (iter != reversedirect_screen[dir].end())
  {
    return iter->second;
  }
  //��������쳣����0��ŵ�������
  return reversedirect_screen[dir][(DWORD)-1];
}

