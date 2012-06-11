/**
 * \brief ����Ʒѷ��������ӿͻ���
 *
 */
#include <zebra/ScenesServer.h>

/**
 * \brief �Ʒѷ��������ӿͻ���
 *
 * һ������ֻ��һ���Ʒѷ���������������ֻ��Ҫ����һ��ָ�룬����Ҫ���ӹ�����֮��Ķ���
 *
 */
MiniClient *miniClient = NULL;

/**
 * \brief ������Mini������������
 *
 * \return �����Ƿ�ɹ�
 */
bool MiniClient::connectToMiniServer()
{
  if (!connect())
  {
    Zebra::logger->error("����Mini������ʧ��");
    return false;
  }

  Cmd::Mini::t_LoginMini tCmd;
  tCmd.wdServerID   = ScenesService::getInstance().getServerID();
  tCmd.wdServerType = ScenesService::getInstance().getServerType();

  return sendCmd(&tCmd,sizeof(tCmd));
}

/**
 * \brief ����zThread�еĴ��麯�������̵߳����ص����������ڴ�����յ���ָ��
 *
 */
void MiniClient::run()
{
  zTCPBufferClient::run();

  while(!ScenesService::getInstance().isTerminate())
  {
    while(!connect())
    {
      Zebra::logger->error("����С��Ϸ������ʧ��");
      zThread::msleep(1000);
    }
    Cmd::Super::t_restart_ServerEntry_NotifyOther notify;
    notify.srcID=ScenesService::getInstance().getServerID();
    notify.dstID=this->getServerID();
    ScenesService::getInstance().sendCmdToSuperServer(&notify,sizeof(notify));
    zThread::msleep(2000);
    connect();

    Cmd::Mini::t_LoginMini tCmd;
    tCmd.wdServerID   = ScenesService::getInstance().getServerID();
    tCmd.wdServerType = ScenesService::getInstance().getServerType();

    if (sendCmd(&tCmd,sizeof(tCmd)))
    {
      zTCPBufferClient::run();
    }
    zThread::msleep(1000);
  }
  //��Mini֮������ӶϿ�������Ҫ�رշ�����
  //ScenesService::getInstance().Terminate();
}

struct Capacity : public PackageCallback
{       
  public: 
    Capacity(SceneUser* user,DWORD id,DWORD level) : _user(user),_id(id),_level(level),_num(0)
    { }

    bool exec(zObject* o)
    {
      if (o && o->data.dwObjectID==_id && o->data.upgrade==_level && o->data.dwNum<o->base->maxnum)
        _num += o->base->maxnum-o->data.dwNum;
      return true;
    }

    DWORD num() const
    {
      return _num;
    }

  private:
    SceneUser* _user;
    DWORD _id;
    DWORD _level;
    DWORD _num;
};

struct DrawCombin : public PackageCallback
{       
  public: 
    DrawCombin(SceneUser* user,DWORD id,DWORD level,DWORD &num) : _user(user),_id(id),_level(level),_num(num)
    { }

    bool exec(zObject* o)
    {
      if (o && o->data.dwObjectID==_id && o->data.upgrade==_level && o->data.dwNum<o->base->maxnum)
      {
        DWORD n = (_num>o->base->maxnum-o->data.dwNum)?(o->base->maxnum-o->data.dwNum):_num;

        o->data.dwNum += n;
        Cmd::stRefCountObjectPropertyUserCmd status;
        status.qwThisID = o->data.qwThisID;
        status.dwNum = o->data.dwNum;
        _user->sendCmdToMe(&status,sizeof(status));

        _num -= n;
      }
      return _num>0;
    }

    DWORD num() const
    {
      return _num;
    }

  private:
    SceneUser* _user;
    DWORD _id;
    DWORD _level;
    DWORD &_num;
};
/**
 * \brief ��������Mini������������ָ��
 *
 * \param pNullCmd ��������ָ��
 * \param nCmdLen ��������ָ���
 * \return �����Ƿ�ɹ�
 */
bool MiniClient::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
#ifdef _MSGPARSE_
  Zebra::logger->error("?? MiniClient::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
#endif 

  using namespace Cmd::Mini;
  using namespace Cmd;

  switch (pNullCmd->para)
  {
    case PARA_SCENE_CHECK_DRAW:
      {
        t_Scene_Check_Draw *rev = (t_Scene_Check_Draw *)pNullCmd;
        SceneUser *u = SceneUserManager::getMe().getUserByID(rev->userID);
        if (!u) return false;

        zObjectB *base = objectbm.get(584);
        if (!base) return false;

        Capacity c(u,584,0);
        u->packs.main.execEvery(c);
        if ((c.num()+u->packs.main.space()*base->maxnum)<rev->num)
        {
          stDrawRetCommonMiniGameCmd send;
          send.ret = 3;//������
          u->sendCmdToMe(&send,sizeof(send));
          return true;
        }
        return sendCmd(rev,nCmdLen);
      }
      break;
    case PARA_SCENE_DRAW_RET:
      {
        t_Scene_Draw_Ret *rev = (t_Scene_Draw_Ret *)pNullCmd;
        SceneUser *u = SceneUserManager::getMe().getUserByID(rev->userID);
        if (!u)
        {
          rev->ret = 2;//������
          sendCmd(rev,nCmdLen);
          return true;
        }

        zObjectB *base = objectbm.get(584);
        if (!base)
        {
          rev->ret = 0;//ʧ��
          sendCmd(rev,nCmdLen);
          return false;
        }

        //�ϲ�
        DrawCombin dc(u,584,0,rev->num);
        u->packs.main.execEvery(dc);

        //��Ӱ���
        for (DWORD i=rev->num; i!=0;)
        {
          DWORD num = (i>base->maxnum)?base->maxnum:i;

          zObject* o = zObject::create(base,num,0);
          if (o && u->packs.addObject(o,true,AUTO_PACK))
          {
            rev->num -= num;

            Cmd::stAddObjectPropertyUserCmd send;
            bcopy(&o->data,&send.object,sizeof(t_Object),sizeof(send.object)); 
            u->sendCmdToMe(&send,sizeof(send));
            Channel::sendSys(u,Cmd::INFO_TYPE_GAME,"�õ�%s%ld��",o->name,o->data.dwNum);
          }

          i -= num;
        }

        rev->ret = rev->num?3:1;//��Ӱ���ʧ��
        sendCmd(rev,nCmdLen);
        return true;
      }
      break;
    case PARA_SCENE_DEPOSIT:
      {
        t_Scene_Deposit *rev = (t_Scene_Deposit *)pNullCmd;
        SceneUser *u = SceneUserManager::getMe().getUserByID(rev->userID);
        if (!u) return false;

        if (!u->packs.checkMoney(rev->num*100) || !u->packs.removeMoney(rev->num*100,"С��Ϸ��ֵ"))
        {
          Channel::sendSys(u,Cmd::INFO_TYPE_FAIL,"��Ľ�Ǯ����");
          return true;
        }
        return sendCmd(rev,nCmdLen);
      }
      break;
    default:
      break;
  }

  Zebra::logger->error("MiniClient::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

