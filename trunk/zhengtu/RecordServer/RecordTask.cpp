/**
 * \brief ʵ�ֶ���������
 *
 * 
 */

#include "RecordServer.h"

/**
 * \brief ��֤��½����������������ָ��
 *
 * �����֤��ͨ��ֱ�ӶϿ�����
 *
 * \param ptCmd ��½ָ��
 * \return ��֤�Ƿ�ɹ�
 */
bool RecordTask::verifyLogin(const Cmd::Record::t_LoginRecord *ptCmd)
{
  Zebra::logger->info("RecordTask::verifyLogin(%s:%d)%u",getIP(),ptCmd->wdServerID,ptCmd->wdServerType);
  using namespace Cmd::Record;

  if (CMD_LOGIN == ptCmd->cmd  && PARA_LOGIN == ptCmd->para)
  {
    const Cmd::Super::ServerEntry *entry = RecordService::getInstance().getServerEntryById(ptCmd->wdServerID);
    char strIP[32];
    strncpy(strIP,getIP(),sizeof(strIP));    
    if (entry
        && ptCmd->wdServerType == entry->wdServerType
        && 0 == strcmp(strIP,entry->pstrIP))
    {
      wdServerID = ptCmd->wdServerID;
      wdServerType = ptCmd->wdServerType;
      return true;
    }
  }

  return false;
}

/**
 * \brief �ȴ�������ָ֤�������֤
 *
 * ʵ���麯��<code>zTCPTask::verifyConn</code>
 *
 * \return ��֤�Ƿ�ɹ�,���߳�ʱ
 */
int RecordTask::verifyConn()
{
  Zebra::logger->debug("RecordTask::verifyConn");
  int retcode = mSocket->recvToBuf_NoPoll();
  if (retcode > 0)
  {
    BYTE pstrCmd[zSocket::MAX_DATASIZE];
    int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
    if (nCmdLen <= 0)
      //����ֻ�Ǵӻ���ȡ���ݰ�,���Բ������,û������ֱ�ӷ���
      return 0;
    else
    {
      using namespace Cmd::Record;
      if (verifyLogin((t_LoginRecord *)pstrCmd))
      {
        Zebra::logger->debug("�ͻ�������ͨ����֤");
        return 1;
      }
      else
      {
        Zebra::logger->error("�ͻ���������֤ʧ��");
        return -1;
      }
    }
  }
  else
    return retcode;
}

/**
 * \brief ȷ��һ�����������ӵ�״̬�ǿ��Ի��յ�
 *
 * ��һ������״̬�ǿ��Ի��յ�״̬,��ô��ζ��������ӵ������������ڽ���,���Դ��ڴ��а�ȫ��ɾ���ˣ���<br>
 * ʵ�����麯��<code>zTCPTask::recycleConn</code>
 *
 * \return �Ƿ���Ի���
 */
int RecordTask::recycleConn()
{
  Zebra::logger->debug("RecordTask::recycleConn");
  RecordSessionManager::getInstance().removeAllByServerID(getID());
  //TODO ��Ҫ��֤�浵ָ��������
  return 1;
}
bool RecordTask::msgParse_Session(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  Zebra::logger->debug("RecordTask::msgParse_Session");

  using namespace Cmd::Record;
  
  switch(pNullCmd->para)
  {
    case PARA_CHK_USER_EXIST:
      {
        t_chkUserExist_SessionRecord *rev = (t_chkUserExist_SessionRecord *)pNullCmd;
        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          Zebra::logger->error("���ܻ�ȡ���ݿ���");
          return false;
        }
#pragma pack(1)
        struct exist_struct
        {
          DWORD id;
          DWORD level;
          char name[MAX_NAMESIZE+1];
        };// __attribute__ ((packed));
#pragma pack()
        static const dbCol exist_define[] = {
          { "`CHARID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { "`LEVEL`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { "`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
          { NULL,0,0}
        };
        exist_struct * es;
        char where[128];
        bzero(where,sizeof(where));

        std::string escapeName;
        _snprintf(where,sizeof(where) - 1,"NAME='%s'",
            RecordService::dbConnPool->escapeString(handle,rev->name,escapeName).c_str());

        DWORD retcode = RecordService::dbConnPool->exeSelect(handle,"`CHARBASE`",exist_define,where,"CHARID DESC",(BYTE **)&es);
        RecordService::dbConnPool->putHandle(handle);

        if (es)
        {
          for (DWORD i=0; i< retcode; i++)
          {
            if (strcmp(es[i].name,rev->name) == 0)
            {
              rev->user_id = es[i].id;
              rev->user_level = es[i].level;
              break;
            }
          }
          SAFE_DELETE_VEC(es);
          sendCmd(rev,sizeof(t_userExist_SceneRecord));
        }
        return true;
      }
    default:
      break;
  }

  Zebra::logger->error("RecordTask::msgParse_Session(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

bool RecordTask::msgParse_Gateway(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  Zebra::logger->debug("RecordTask::msgParse_Gateway");
  using namespace Cmd::Record;

  switch(pNullCmd->para)
  {
    case PARA_GATE_DELCHAR:
      {
        t_DelChar_GateRecord *rev = (t_DelChar_GateRecord *)pNullCmd;
        t_DelChar_Return_GateRecord cmd;
        /*
        static const dbCol delchar_define[] = {
          { "`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
          { "`AVAILABLE`",zDBConnPool::DB_BYTE,sizeof(BYTE) },
          { NULL,0,0}
        };
        struct {
          char name[MAX_NAMESIZE+1];
          BYTE available;
        } __attribute__ ((packed))
        delchar_data = { "",0}; //������,����
        // */
        char where[128];

        cmd.accid = rev->accid;
        cmd.id = rev->id;
        strncpy(cmd.name,rev->name,sizeof(cmd.name));
        cmd.retcode = 0;

        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          Zebra::logger->error("���ܻ�ȡ���ݿ���");
          sendCmd(&cmd,sizeof(cmd));
          return false;
        }

        //����ɾ��ԭ���Ѿ����ϵĽ�ɫ
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where) - 1,"ACCID = %u AND CHARID = %u",rev->accid,rev->id);
        if ((DWORD)-1 == RecordService::dbConnPool->exeDelete(handle,"`CHARBASE`",where))
        {
          RecordService::dbConnPool->putHandle(handle);
          sendCmd(&cmd,sizeof(cmd));
          Zebra::logger->warn("ɾ����ɫʱʧ��:%u,%u",rev->accid,rev->id);
          return false;
        }
       
        RecordService::dbConnPool->putHandle(handle);
        Zebra::logger->info("ɾ����ɫ:%u,%u",rev->accid,rev->id);

        cmd.retcode = 1;
        sendCmd(&cmd,sizeof(cmd));
        //ɾ����ɫ�����µõ���ɫ�б�
        return getSelectInfo(rev->accid,rev->countryid); 

        return true;
      }
      break;
      //������ҵ�������
    case REQUEST_GATE_COUNTRY_ORDER:
      {
        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          Zebra::logger->error("���ܻ�ȡ���ݿ���");
          return false;
        }

        static const dbCol countryid_define[] = {
          { "`COUNTRY`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { "`A`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { NULL,0,0}
        };
        char Buf[200];
        bzero(Buf,sizeof(Buf));
        t_order_Country_GateRecord *ret_gate = 
          (t_order_Country_GateRecord*)Buf;
        constructInPlace(ret_gate);
        DWORD retcode=0; 
        char where[128];
        bzero(where,sizeof(where));
       
        strncpy(where,"SELECT `COUNTRY` `A`,count(`COUNTRY`)  AS `A` FROM `CHARBASE` GROUP BY `COUNTRY` ORDER BY `A`",sizeof(where));
        retcode = RecordService::dbConnPool->execSelectSql(handle,
            where,strlen(where),countryid_define,(DWORD)10,(BYTE*)(ret_gate->order.order));

        RecordService::dbConnPool->putHandle(handle);
        if (retcode != (DWORD)-1)
        {
          ret_gate->order.size = retcode;
        }
        for(int i = 0 ; i < (int)ret_gate->order.size ; i ++)
        {
          Zebra::logger->debug("����:%d,ע������:%d",ret_gate->order.order[i].country,ret_gate->order.order[i].count);
        }
        sendCmd(ret_gate,sizeof(t_order_Country_GateRecord) 
            + sizeof(ret_gate->order.order[0]) * ret_gate->order.size); 
        return true;
      }
      break;
    case PARA_GATE_CHECKNAME:
      {
        t_CheckName_GateRecord * rev = (t_CheckName_GateRecord *)pNullCmd;
        t_CheckName_Return_GateRecord ret;
        ret.accid = rev->accid;
        strncpy(ret.name,rev->name,MAX_NAMESIZE-1);
        //������֤�����Ƿ��ظ�

        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          Zebra::logger->error("���ܻ�ȡ���ݿ���");
          return false;
        }

        static const dbCol verifyname_define[] = {
          { "`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
          { NULL,0,0}
        };
        char strName[MAX_NAMESIZE+1];
        char where[128];
        bzero(where,sizeof(where));
        
        std::string upName;
        RecordService::dbConnPool->escapeString(handle,rev->name,upName);

        _snprintf(where,sizeof(where) - 1,"NAME = '%s'",upName.c_str());
        DWORD retcode = RecordService::dbConnPool->exeSelectLimit(handle,"`CHARBASE`",verifyname_define,where,"CHARID DESC",1,(BYTE*)(strName));

        RecordService::dbConnPool->putHandle(handle);
        Zebra::logger->debug("��ɫ�����:%s have %d",upName.c_str(),retcode);

        ret.err_code = retcode;
        sendCmd(&ret,sizeof(ret));
        return true;
      }
      break;
    case PARA_GATE_CREATECHAR:
      {
        t_CreateChar_GateRecord *rev = (t_CreateChar_GateRecord *)pNullCmd;
        t_CreateChar_Return_GateRecord ret;
        static const dbCol createchar_define[] = {
          { "`ACCID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { "`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
          { "`TYPE`",zDBConnPool::DB_WORD,sizeof(WORD) },
          { "`LEVEL`",zDBConnPool::DB_WORD,sizeof(WORD) },
          { "`HAIR`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { "`MAPID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { "`MAPNAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
          { "`COUNTRY`",zDBConnPool::DB_WORD,sizeof(WORD) },
          { "`ACCPRIV`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { "`CREATEIP`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
		  { "`GRACE`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
		  { "`FACE`",zDBConnPool::DB_WORD,sizeof(WORD) },
          { NULL,0,0}
        };
#pragma pack(1)
        struct {
          DWORD accid;
          char name[MAX_NAMESIZE+1];
          WORD type;
          WORD level;
          DWORD hair;
          DWORD mapid;
          char  mapName[MAX_NAMESIZE+1];
          WORD country;
          DWORD accPriv;
          DWORD createip;
		  DWORD useJob; //sky ��ɫְҵ
		  WORD  face;  // ranqd ��ɫͷ��
        }// __attribute__ ((packed))

        createchar_data;
#pragma pack()
        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          Zebra::logger->error("���ܻ�ȡ���ݿ���");
          return false;
        }

        //����ʺ�Ȩ��
        static const dbCol priv_define[] = {
          { "`PRIV`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { NULL,0,0}
        };

        //�������ݿ��ɫ��Ϣ
        bzero(&createchar_data,sizeof(createchar_data));
        createchar_data.accid = rev->accid;
        strncpy(createchar_data.name,rev->name,MAX_NAMESIZE);
        createchar_data.type = rev->type;
        createchar_data.country = rev->country;
        createchar_data.level = 1;
        createchar_data.hair = rev->hair;
        createchar_data.mapid = 0;
		// [ranqd] ����ְҵ����
		createchar_data.useJob = rev->JobType;
		// [ranqd] ����ͷ�񱣴�
		createchar_data.face   = rev->Face;
		if( createchar_data.useJob <= JOB_NULL || createchar_data.useJob > JOB_PASTOR )
		{
			createchar_data.useJob = JOB_FIGHTER;
		}
        strncpy(createchar_data.mapName,rev->mapName,MAX_NAMESIZE);
        createchar_data.createip = rev->createip;
        //Zebra::logger->debug("������ɫIP %s(%u)",inet_ntoa(*(struct in_addr*)&createchar_data.createip),createchar_data.createip);
        char where[64];
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where)-1,"ACCID=%u",rev->accid);
        DWORD retcode = RecordService::dbConnPool->exeSelectLimit(handle,"`ACCPRIV`",priv_define,where,NULL,1,(BYTE*)(&createchar_data.accPriv));
        retcode = RecordService::dbConnPool->exeInsert(handle,"`CHARBASE`",createchar_define,(const BYTE *)(&createchar_data));
        RecordService::dbConnPool->putHandle(handle);
        if ((DWORD)-1 == retcode)
        {
          Zebra::logger->error("������ɫ�������ݿ���� %u,%s",rev->accid,rev->name);

          ret.accid = rev->accid;
          ret.retcode = 0;
          bzero(&ret.charinfo,sizeof(ret.charinfo));
          sendCmd(&ret,sizeof(ret));

          return false;
        }

        //�����´�����ɫ��Ϣ������
        ret.accid = rev->accid;
        ret.retcode = 1;
        bzero(&ret.charinfo,sizeof(ret.charinfo));
        ret.charinfo.id = retcode;
        strncpy(ret.charinfo.name,createchar_data.name,MAX_NAMESIZE);
        ret.charinfo.type = createchar_data.type;
        ret.charinfo.level = createchar_data.level;
        ret.charinfo.mapid = createchar_data.mapid;
        ret.charinfo.country = createchar_data.country;
		ret.charinfo.JobType = createchar_data.useJob;
		ret.charinfo.face    = createchar_data.face;
        strncpy(ret.charinfo.mapName,createchar_data.mapName,MAX_NAMESIZE);

        return sendCmd(&ret,sizeof(ret));
      }
      break;
    case PARA_GATE_GET_SELECTINFO:
      {
        t_Get_SelectInfo_GateRecord *rev= (t_Get_SelectInfo_GateRecord *)pNullCmd;
        return getSelectInfo(rev->accid,rev->countryid); 
      }
      break;
  }

  Zebra::logger->error("RecordTask::msgParse_Gateway(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

const dbCol RecordTask::charbase_define[] = {
  { "`ACCID`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`CHARID`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`NAME`",        zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
  { "`TYPE`",        zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`LEVEL`",      zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`FACE`",        zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`HAIR`",        zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`BODYCOLOR`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`GOODNESS`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`MAPID`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`MAPNAME`",      zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
  { "`X`",        zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`Y`",        zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`UNIONID`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`SCHOOLID`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`SEPTID`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`HP`",        zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`MP`",        zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`SP`",        zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`EXP`",        zDBConnPool::DB_QWORD,sizeof(QWORD) },
  { "`OLDMAP`", zDBConnPool::DB_STR, sizeof(char[MAX_PATH]) },
  {"`SKILLPOINTS`",    zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`POINTS`",      zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`COUNTRY`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`CONSORT`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`FORBIDTALK`",    zDBConnPool::DB_QWORD,sizeof(QWORD) },
  { "`BITMASK`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`ONLINETIME`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`CON`",        zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`STR`",        zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`DEX`",        zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`INT`",        zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`MEN`",        zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`RELIVEWEAKTIME`",  zDBConnPool::DB_WORD,sizeof(WORD) },
  { "`GRACE`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`EXPLOIT`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`TIRETIME`",      zDBConnPool::DB_BIN,sizeof(char[36+1])},
  { "`OFFLINETIME`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`FIVETYPE`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`FIVELEVEL`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`PKADDITION`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`MONEY`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`ANSWERCOUNT`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`HONOR`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`MAXHONOR`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`GOMAPTYPE`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`MSGTIME`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`ACCPRIV`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`GOLD`",        zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`TICKET`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`CREATETIME`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`GOLDGIVE`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`PETPACK`",      zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "`PETPOINT`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`LEVELSEPT`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`PUNISHTIME`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`TRAINTIME`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`ZS`",			  zDBConnPool::DB_DWORD,	sizeof(DWORD)},
  { "`DOUBLETIME`",	  zDBConnPool::DB_DWORD,	sizeof(DWORD)},
  { "`ALLBINARY`",    zDBConnPool::DB_BIN2,0},
  { NULL,0,0}
};

bool RecordTask::readCharBase(const Cmd::Record::t_ReadUser_SceneRecord *rev)
{
  Zebra::logger->debug("RecordTask::readCharBase");
  char readBuf[zSocket::MAX_DATASIZE];
  using namespace Cmd::Record;
  t_UserInfo_SceneRecord *ret;
  ret = (t_UserInfo_SceneRecord *)readBuf;
  constructInPlace(ret);

  char where[128];

  ret->id=rev->id;
  ret->dwMapTempID = rev->dwMapTempID;
  ret->RegMapType = rev->RegMapType;

  Zebra::logger->fatal("readCharBase dwMapTempID:%d RegMapType:%d",ret->dwMapTempID, ret->RegMapType);

  connHandleID handle = RecordService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"CHARID = %u AND AVAILABLE = 1",rev->id);
  DWORD retcode = RecordService::dbConnPool->exeSelectLimit(
      handle,"`CHARBASE`",charbase_define,where,"CHARID DESC",1,(BYTE*)(&ret->charbase));//TODO �ȴ��޸�?
  RecordService::dbConnPool->putHandle(handle);
  if (1 == retcode)
  {
    Zebra::logger->info("�ҵ��ϸ�Ľ�ɫ��¼��%u,%s)",ret->charbase.id,ret->charbase.name);
  }
  else
  {
    Zebra::logger->error("��ȡ����ʧ��,û���ҵ���¼");
    bzero(&ret->charbase,sizeof(ret->charbase));
    ret->dataSize = (DWORD)PARA_SCENE_USER_READ_ERROR;
    sendCmd(ret,sizeof(t_UserInfo_SceneRecord));
    return false;
  }
  Zebra::logger->debug("��ȡ��������������,ѹ�����ݴ�С(size = %ld)",ret->dataSize);
  return sendCmd(ret,sizeof(t_UserInfo_SceneRecord) + ret->dataSize);
}

bool RecordTask::saveCharBase(const Cmd::Record::t_WriteUser_SceneRecord *rev)
{
  Zebra::logger->debug("RecordTask::saveCharBase");
  char where[128];

  connHandleID handle = RecordService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"`CHARID` = %u AND `AVAILABLE` = 1",rev->id);
  DWORD retcode = RecordService::dbConnPool->exeUpdate(handle,"`CHARBASE`",charbase_define,(BYTE*)(&rev->charbase),where);
  RecordService::dbConnPool->putHandle(handle);

  //Zebra::logger->debug(where);
  if (1 == retcode)
  {
    Zebra::logger->info("���浵���ɹ���%u,%u",rev->id,retcode);
  }
  else
  {
    Zebra::logger->error("���浵��ʧ�ܣ�%u,%u",rev->id,retcode);
  }

  return true;
}

bool RecordTask::msgParse_Scene(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
  Zebra::logger->debug("RecordTask::msgParse_Scene");
  using namespace Cmd::Record;

  switch(pNullCmd->para)
  {
    case PARA_SCENE_USER_WRITE:
      {
        t_WriteUser_SceneRecord *rev = (t_WriteUser_SceneRecord *)pNullCmd;
        if (((TIMETICK_WRITEBACK == rev->writeback_type || OPERATION_WRITEBACK == rev->writeback_type)
              && RecordSessionManager::getInstance().verify(rev->accid,rev->id,getID()))
            || (LOGOUT_WRITEBACK == rev->writeback_type
              && RecordSessionManager::getInstance().remove(rev->accid,rev->id,getID()))
            || (CHANGE_SCENE_WRITEBACK == rev->writeback_type
              && RecordSessionManager::getInstance().remove(rev->accid,rev->id,getID())))
        {
          if (saveCharBase(rev))
          {
            if (CHANGE_SCENE_WRITEBACK == rev->writeback_type || LOGOUT_WRITEBACK == rev->writeback_type)
            {
              using namespace Cmd::Record;
              t_WriteUser_SceneRecord_Ok ok; 
              ok.type=rev->writeback_type;
              ok.id=rev->id;
              ok.accid=rev->accid;
              sendCmd(&ok,sizeof(ok));
            }
            return true;
          }
        }
        else
        {
          Zebra::logger->error("��д������֤ʧ��,���ܻ�д������%lu,%lu",rev->accid,rev->id);
        }
      }
      break;
    case PARA_SCENE_USER_READ:
      {
        t_ReadUser_SceneRecord *rev = (t_ReadUser_SceneRecord *)pNullCmd;
        if (RecordSessionManager::getInstance().add(rev->accid,rev->id,getID()))
        {
          if (readCharBase(rev)) 
          {
            return true;
          }
          else
          {
            RecordSessionManager::getInstance().remove(rev->accid,rev->id,getID());
            return true;
          }
        }
        else
        {
          using namespace Cmd::Record;
          t_UserInfo_SceneRecord ret;
          ret.id=rev->id;
          ret.dwMapTempID=rev->dwMapTempID;
          ret.dataSize = (DWORD)PARA_SCENE_USER_READ_ERROR;
          sendCmd(&ret,sizeof(t_UserInfo_SceneRecord));
          Zebra::logger->error("��Ӷ�ȡ��¼ʧ��,���ܶ�ȡ������Ϣ��%lu,%lu",rev->accid,rev->id);
          return true;
        }
      }
      break;
    case PARA_SCENE_USER_REMOVE:
      {
        t_RemoveUser_SceneRecord *rev = (t_RemoveUser_SceneRecord *)pNullCmd;
        RecordSessionManager::getInstance().remove(rev->accid,rev->id,getID());
        Zebra::logger->warn("�û��ڶ�ȡ�����������˳�(accid=%u,id=%u",rev->accid,rev->id);
        return true;
      }
      break;
    case PARA_SCENE_USER_EXIST:
      {
        t_userExist_SceneRecord *rev = (t_userExist_SceneRecord *)pNullCmd;

        connHandleID handle = RecordService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          Zebra::logger->error("���ܻ�ȡ���ݿ���");
          return false;
        }
#pragma pack(1)
        struct exist_struct
        {
          DWORD id;
          char name[MAX_NAMESIZE+1];
        };// __attribute__ ((packed));
#pragma pack()
        static const dbCol exist_define[] = {
          { "`CHARID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { "`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
          { NULL,0,0}
        };
        exist_struct * es;
        char where[128];
        bzero(where,sizeof(where));

		//DWORD len = exist_define[1].size*2 + 1;

        //char *strData = new char[len];
		//bzero(strData,len);
        std::string escapeName;

		//std::ostringstream strSql;

		//RecordService::dbConnPool->escapeString(handle,rev->sm.toName,strData,33);
        
		//strSql << strData;
		RecordService::dbConnPool->escapeString(handle,rev->sm.toName,escapeName);
        _snprintf(where,sizeof(where) - 1,"NAME='%s'",escapeName.c_str()/*strSql.str().c_str()*//*,strData,exist_define[1].size)*/);
        DWORD retcode = RecordService::dbConnPool->exeSelect(handle,"`CHARBASE`",exist_define,where,"CHARID DESC",(BYTE **)&es);
        RecordService::dbConnPool->putHandle(handle);
//		delete[] strData;

        if (es)
        {
          for (DWORD i=0; i< retcode; i++)
          {
            if (strcmp(es[i].name,rev->sm.toName))
              continue;

            rev->toID = es[i].id;
          }
          SAFE_DELETE_VEC(es);
        }
        sendCmd(rev,sizeof(t_userExist_SceneRecord));
        return true;
      }
      break;
#ifdef _TEST_DATA_LOG
    case PARA_SCENE_INSERT_CHARTEST:
      {
        insertCharTest((t_Insert_CharTest_SceneRecord *)pNullCmd);
        return true;
      }
      break;
    case PARA_SCENE_UPDATE_CHARTEST:
      {
        updateCharTest((t_Update_CharTest_SceneRecord *)pNullCmd);
        return true;
      }
      break;
    case PARA_SCENE_DELETE_CHARTEST:
      {
        deleteCharTest((t_Delete_CharTest_SceneRecord *)pNullCmd);
        return true;
      }
      break;
    case PARA_SCENE_READ_CHARTEST:
      {
        readCharTest((t_Read_CharTest_SceneRecord *)pNullCmd);
        return true;
      }
      break;
#endif
    default:
      break;
  }

  Zebra::logger->error("RecordTask::msgParse_Scene(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

/**
 * \brief �������Ը������������ӵ�ָ��
 *
 * \param pNullCmd �������ָ��
 * \param nCmdLen ָ���
 * \return �����Ƿ�ɹ�
 */
bool RecordTask::msgParse(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen)
{
#ifdef _MSGPARSE_
  Zebra::logger->error("?? RecordTask::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
#endif

  using namespace Cmd::Record;

  switch(pNullCmd->cmd)
  {
    case CMD_GATE:
      if (msgParse_Gateway(pNullCmd,nCmdLen))
      {
        return true;
      }
      break;
    case CMD_SCENE:
      if (msgParse_Scene(pNullCmd,nCmdLen))
      {
        return true;
      }
      break;
    case CMD_SESSION:
      if (msgParse_Session(pNullCmd,nCmdLen))
      {
        return true;
      }
      break;
  }

  Zebra::logger->error("RecordTask::msgParse(%u,%u,%u)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
  return false;
}

bool RecordTask::getSelectInfo(DWORD accid, DWORD countryid)
{
  Zebra::logger->debug("RecordTask::getSelectInfo");
  static const dbCol charinfo_define[] = {
    { "`CHARID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`NAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
	{ "`GRACE`",zDBConnPool::DB_WORD,sizeof(WORD)},
    { "`TYPE`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { "`LEVEL`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { "`MAPID`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { "`MAPNAME`",zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
    { "`COUNTRY`",zDBConnPool::DB_WORD,sizeof(WORD) },
    { "`FACE`",zDBConnPool::DB_WORD,sizeof(WORD) },
	{ "`HAIR`",zDBConnPool::DB_WORD,sizeof(WORD) },
	{ "`OLDMAP`", zDBConnPool::DB_STR, sizeof(char[MAX_PATH]) },
	{ "", zDBConnPool::DB_STR, sizeof(char[MAX_NAMESIZE+1]) },
    { "`BITMASK`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
    { NULL,0,0}
  };
  Cmd::Record::t_Ret_SelectInfo_GateRecord ret;
  char where[128];

  connHandleID handle = RecordService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }
  bzero(where,sizeof(where));
  if( countryid == -1 )
  {
	  _snprintf( where,sizeof(where) - 1,"ACCID = %u AND AVAILABLE = 1",accid );
  }
  else
  {
	  _snprintf( where,sizeof(where) - 1,"ACCID = %u AND COUNTRY = %u AND AVAILABLE = 1",accid,countryid );
  }

  DWORD retcode = RecordService::dbConnPool->exeSelectLimit(handle,"`CHARBASE`",charinfo_define,where,"LEVEL DESC"/*,CHARID DESC" */,Cmd::MAX_CHARINFO,(BYTE*)(ret.info));

  RecordService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode
      || 0 == retcode)
  {
    //Zebra::logger->debug("û���ҵ���¼");
    ret.num = 0;
    bzero(ret.info,sizeof(ret.info));
  }
  else
  {
    //Zebra::logger->debug("�ҵ� %u ���ϸ�Ľ�ɫ��¼",retcode);
    ret.num = retcode;
  }
  ret.accid=accid;
//#if 0
//  ret.num=1;
//  std::string s;
//  ret.info[0].id=rev->accid*2;
//  char name[32];
//  _snprintf(name,32,"�����û�%d",ret.info[0].id);
//  s=name;
//  strcpy((char *)ret.info[0].name,s.c_str());
//  ret.info[0].level=1;
//  ret.info[0].mapid=1;
//  s="���Ե�ͼ";
//  strcpy((char *)ret.info[0].mapName,s.c_str());
//#endif

  return sendCmd(&ret,sizeof(ret));
}
#ifdef _TEST_DATA_LOG
const dbCol RecordTask::chartest_define[] = {
  { "`NAME`",        zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE]) },
  { "`LEVEL`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`UPDATETIME`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`UPDATEUSETIME`",  zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`DEATHTIMES`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`HPLEECHDOM`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`MPLEECHDOM`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`SPLEECHDOM`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`GETMONEY`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`GETHEIGH`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`GETSOCKET`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`GETMATERIAL`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`GETSTONE`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`GETSCROLL`",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "`MONEY`",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { NULL,0,0}
};
/**
 * \brief ��ȡ�������ݵ���
 *
 *
 * \param rev: �յ��Ķ�ȡָ��
 * \return ��ȡ�Ƿ�ɹ�
 */
bool RecordTask::readCharTest(Cmd::Record::t_Read_CharTest_SceneRecord *rev)
{
  Zebra::logger->debug("RecordTask::readCharTest");
  
  using namespace Cmd::Record;
  t_Read_CharTest_SceneRecord ret;
  strncpy(ret.name,rev->name,MAX_NAMESIZE);
  ret.level = rev->level;

  char where[128];

  connHandleID handle = RecordService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }
  bzero(where,sizeof(where));
  std::string escapeName;
  _snprintf(where,sizeof(where) - 1,"`NAME` = '%s' AND `LEVEL` = %u",RecordService::dbConnPool->escapeString(handle,ret.name,escapeName).c_str(),ret.level);
  DWORD retcode = RecordService::dbConnPool->exeSelectLimit(
      handle,"`CHARTEST`",chartest_define,where,"`NAME` DESC",1,(BYTE*)(ret.name));//TODO �ȴ��޸�?
  RecordService::dbConnPool->putHandle(handle);
  if (1 == retcode)
  {
    Zebra::logger->debug("��ȡ����ͳ�����ݳɹ�(%s)",ret.name);
  }
  else
  {
    Zebra::logger->debug("δ�ҵ�ͳ������(%s)",ret.name);
    return false;
  }
  return sendCmd(&ret,sizeof(ret));
}
/**
 * \brief �����������
 *
 *
 * \param rev: �յ��Ĳ���ָ��
 * \return �����Ƿ�ɹ�
 */
bool RecordTask::insertCharTest(Cmd::Record::t_Insert_CharTest_SceneRecord *rev)
{
  Zebra::logger->debug("RecordTask::insertCharTest");
  DWORD retcode = 0;
  connHandleID handle = RecordService::dbConnPool->getHandle();
  retcode=RecordService::dbConnPool->exeInsert(handle,"`CHARTEST`",chartest_define,(const BYTE *)(rev->name));
  RecordService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode)
  {
    Zebra::logger->error("��ɫ����������ݿ���� %s",rev->name);
    return false;
  }
  return true;
}
/**
 * \brief ���²�������
 *
 *
 * \param rev: �յ��ĸ�������ָ��
 * \return �����Ƿ�ɹ�
 */
bool RecordTask::updateCharTest(Cmd::Record::t_Update_CharTest_SceneRecord *rev)
{
  Zebra::logger->debug("RecordTask::updateCharTest");
  char where[128];

  connHandleID handle = RecordService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }
  bzero(where,sizeof(where));
  std::string escapeName;
  _snprintf(where,sizeof(where) - 1,"`NAME` = '%s' AND `LEVEL` = %u",RecordService::dbConnPool->escapeString(handle,rev->name,escapeName).c_str(),rev->level);
  DWORD retcode=RecordService::dbConnPool->exeUpdate(handle,"`CHARTEST`",chartest_define,(BYTE*)(rev->name),where);
  RecordService::dbConnPool->putHandle(handle);

  Zebra::logger->debug(where);
  if (1 == retcode)
  {
    Zebra::logger->debug("����������ݳɹ�%s,%u",rev->name,retcode);
  }
  else
  {
    Zebra::logger->error("�����������ʧ�ܣ�%s,%u",rev->name,retcode);
  }
  return true;
}
/**
 * \brief ɾ����������
 *
 *
 * \param rev: �յ���ɾ������ָ��
 * \return ɾ���Ƿ�ɹ�
 */
bool RecordTask::deleteCharTest(Cmd::Record::t_Delete_CharTest_SceneRecord *rev)
{
  Zebra::logger->debug("RecordTask::deleteCharTest");
  char where[128];
  bzero(where,sizeof(where));
  connHandleID handle = RecordService::dbConnPool->getHandle();
  std::string escapeName;
  _snprintf(where,sizeof(where) - 1,"`NAME` = '%s' AND `LEVEL` = %u",RecordService::dbConnPool->escapeString(handle,rev->name,escapeName).c_str(),rev->level);
  RecordService::dbConnPool->exeDelete(handle,"`CHARBASE`",where);
  RecordService::dbConnPool->putHandle(handle);
  return true;
}
#endif
