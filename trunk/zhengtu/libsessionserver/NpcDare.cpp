/**
 * \brief ����NPC����ս������
 *
 */

#include <ctime>

#include <zebra/SessionServer.h>

using namespace NpcDareDef;

CNpcDareM *CNpcDareM::um(NULL);

/**
 * \brief ���캯��
 */
CNpcDareM::CNpcDareM()
{
  _notDare = true;
  _notifyDareMessage = true;
}

/**
 * \brief NPC����ս��������ʼ��
 * \return true ��ʼ���ɹ� false��ʼ��ʧ��
 */
bool CNpcDareM::init()
{
  return load();
}

/**
 * \brief ��ù�������Ψһʵ��
 * \return ��������Ψһʵ��
 */
CNpcDareM &CNpcDareM::getMe()
{
  if (um==NULL)
  {
    um=new CNpcDareM();
  }
  return *um;
}

/**
 * \brief ����������
 */
void CNpcDareM::destroyMe()
{
}

/**
 * \brief �����ݿ��м�������Ŀ���¼
 * \return true ���سɹ�
 */
bool CNpcDareM::load()
{
  const dbCol npcdare_read_define[] = {
    { "`COUNTRY`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`MAPID`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`NPCID`",    zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`POSX`",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`POSY`",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`HOLDSEPTID`",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`DARESEPTID`",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`GOLD`",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { NULL,0,0}
  };

  struct NpcDareRecord *recordList,*tempPoint,info;

  recordList = NULL;
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return false;
  }

  DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`NPCDARE`",npcdare_read_define,NULL,NULL,(BYTE **)&recordList);
  SessionService::dbConnPool->putHandle(handle);
  if ((DWORD)-1 == retcode
      || 0 == retcode)
  {
    return true;
  }
  if (recordList)
  {
    tempPoint = &recordList[0];
    for (DWORD i=0; i< retcode; i++)
    {
      bcopy(tempPoint,&info,sizeof(info),sizeof(info));
      CNpcDareObj *pObject = new CNpcDareObj();
      if (pObject)
      {
        pObject->create(info);
        _objList.push_back(pObject);
      }
#ifdef _DEBUG
      Zebra::logger->debug("dwCountry=%u dwMapID=%u dwNpcID=%u dwHoldSeptID=%u dwDareSeptID=%u dwGold=%u",
    info.dwCountry,
    info.dwMapID,
    info.dwNpcID,
    info.dwHoldSeptID,
    info.dwDareSeptID,
    info.dwGold);
#endif
      tempPoint++;
    }
    SAFE_DELETE_VEC(recordList);
    return true;
  }
  else
  {
    Zebra::logger->error("NPC����սĿ�����ݼ���ʧ��,exeSelect ������Чbufָ��");
  }
  return false;
}

/**
 * \brief ���������е�����ˢ�µ����ݿ���ȥ
 * \return true ˢ�³ɹ�
 */
bool CNpcDareM::refreshDB()
{
  return true;
}

/**
 * \brief ����ʼ��ս
 */
void CNpcDareM::doDare()
{
  std::vector<CNpcDareObj*>::iterator vIterator;

  for(vIterator = _objList.begin(); vIterator!=_objList.end(); vIterator++)
  {
    (*vIterator)->doDare();
  }
}

/**
 * \brief ����ʼ��սǰ��֪ͨ��Ϣ
 */
void CNpcDareM::notifyDareReady()
{
  std::vector<CNpcDareObj*>::iterator vIterator;

  for(vIterator = _objList.begin(); vIterator!=_objList.end(); vIterator++)
  {
    (*vIterator)->notifyDareReady();
  }
}

/**
 * \brief �����ս���
 */
void CNpcDareM::doResult()
{
  std::vector<CNpcDareObj*>::iterator vIterator;

  for(vIterator = _objList.begin(); vIterator!=_objList.end(); vIterator++)
  {
    (*vIterator)->doResult();
  }
}

/**
* \brief ǿ�ƽ��ж�ս�������
*/
void CNpcDareM::forceProcessResult()
{
  std::vector<CNpcDareObj*>::iterator vIterator;

  for(vIterator = _objList.begin(); vIterator!=_objList.end(); vIterator++)
  {
    (*vIterator)->forceProcessResult();
  }
}


/**
 * \brief ��ù�������Ψһʵ��
 * \return ��������Ψһʵ��
 */
void CNpcDareM::timer()
{
  struct tm tv1;
  time_t timValue = time(NULL);
  zRTime::getLocalTime(tv1,timValue);

#ifdef _DEBUG
  Zebra::logger->debug("ʱ�䣺%u:%u",tv1.tm_hour,tv1.tm_min);
#endif

  if (_notifyDareMessage && (18 == tv1.tm_hour) && (tv1.tm_min>=55))
  {
    _notifyDareMessage = false;
    notifyDareReady();
  }

  if (_notDare && (19 == tv1.tm_hour) && (tv1.tm_min <6))
  {
    _notDare = false;
    doDare();
  }

  if ((19 == tv1.tm_hour) && (tv1.tm_min >=18))//&&(!_notifyDareMessage)) 
  {
    _notDare= true;
    _notifyDareMessage = true;
    forceProcessResult();
    doResult();
  }
  if (!_notDare)
  {
    doResult();
  }
}

/**
 * \brief ����NPC��Ϣ
 * \param country ����
 * \param mapid ��ͼid
 * \param npcid npc���
 */
CNpcDareObj* CNpcDareM::findObject(DWORD country,DWORD mapid,DWORD npcid)
{
  std::vector<CNpcDareObj*>::iterator vIterator;

  for(vIterator = _objList.begin(); vIterator!=_objList.end(); vIterator++)
  {
    if ((*vIterator)->isMe(country,mapid,npcid))
    {
      return (*vIterator);
    }
  }
  return NULL;
}

/**
 * \brief �����ս������
 * \param rev ��ս������Ϣ
 */
void CNpcDareM::processRequest(Cmd::Session::t_NpcDare_Dare_SceneSession * rev)
{
  CNpcDareObj * npcObj = findObject(rev->dwCountryID,rev->dwMapID,rev->dwNpcID);
  if (npcObj)
  {
    npcObj->dareRequest(rev->dwUserID);
  }
  else
  {
    UserSession *pUser = UserSessionManager::getInstance()->getUserByID(rev->dwUserID);
    if (pUser)
    {
//      CNpcDareObj::itemBack(pUser);
      pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"���˵�˼��������,�޷�����������");
    }
  }

}

/**
 * \brief �����ȡ������
 * \param pUser ��Ǯ�Ľ�ɫ
 * \param rev ��ǰ����Ϣ
 */
void CNpcDareM::processGetGold(Cmd::Session::t_NpcDare_GetGold_SceneSession *rev)
{
  DWORD septid=0;

  UserSession* pUser = UserSessionManager::getInstance()->getUserByID(rev->dwUserID);
  if (!pUser) return;
  if ((septid=CSeptM::getMe().findUserSept(rev->dwUserID)) == 0)
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�޷���ȡ˰���㲻���峤��");
  }
  else
  {
    CNpcDareObj * Obj = getNpcDareObjBySept(septid);
    if (Obj)
    {
      Obj->processGetGold(pUser,septid,rev->dwNpcID,rev->dwMapID,rev->dwCountryID);
    }
  }
}

/**
* \brief ���� Gateway ת�������Ŀͻ�����Ϣ
* \param pUser ��Ϣ������
* \param pNullCmd ��Ϣ����
* \param cmdLen ��Ϣ����
* \return true ����ɹ� false ��Ϣ���ڴ���Χ֮��
*/
bool CNpcDareM::processUserMessage(UserSession *pUser,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen)
{
  switch(pNullCmd->byParam)
  {
    case Cmd::NPCDARE_GETGOLD_PARA:
      {
        //Cmd::stDareNpcGetGold *rev = (Cmd::stDareNpcGetGold *)pNullCmd;
        //processGetGold(pUser,rev);
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

/**
* \brief ����ӳ�����������Ϣ
* \param cmd ��Ϣ��
* \param cmdLen ��Ϣ����
* \return true ����ɹ� false ��Ϣ���ڴ���Χ֮��
*/
bool CNpcDareM::processSceneMessage(const Cmd::t_NullCmd *pNullCmd,const DWORD cmdLen)
{
  switch(pNullCmd->para)
  {
    case Cmd::Session::PARA_SEPT_NPCDARE_GETGOLD:
      {
        Cmd::Session::t_NpcDare_GetGold_SceneSession *rev = (Cmd::Session::t_NpcDare_GetGold_SceneSession *)pNullCmd;
        processGetGold(rev);
        return true;
      }
      break;
    case Cmd::Session::PARA_QUESTION_NPCDARE:
      {
        Cmd::Session::t_questionNpcDare_SceneSession* ptCmd = 
          (Cmd::Session::t_questionNpcDare_SceneSession*)pNullCmd;

        UserSession* pUser = UserSessionManager::getInstance()->getUserByID(ptCmd->dwUserID);
        if (!pUser) return true;
        CNpcDareObj *Obj = this->searchRecord(ptCmd->dwCountryID,ptCmd->dwMapID,ptCmd->dwNpcID);
        if (Obj)
        {
          switch(ptCmd->byType)
          {
            case Cmd::QUESTION_NPCDARE_HOLD:
              {
                DWORD septid = Obj->get_holdseptid();
                if (0 == septid)
                {
                  pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"Ŀǰû�м�����Ƹ�����");
                }
                else
                {
                  CSept *pSept = CSeptM::getMe().getSeptByID(septid);
                  if (pSept)
                  {
                    pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"Ŀǰ��%s�������",  pSept->name);
                  }
                  else
                  {
                    pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"���Ƹ����˵ļ�������Ѿ���ɢ");
                  }
                }
              }
              break;
            case Cmd::QUESTION_NPCDARE_DARE:
              {
                DWORD septid = Obj->get_dareseptid();
                if (0 == septid)
                {
                  pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"Ŀǰû�м�����ս���˵Ŀ���Ȩ");
                }
                else
                {
                  CSept *pSept = CSeptM::getMe().getSeptByID(septid);
                  if (pSept)
                  {
                    pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"Ŀǰ%s��������ս���˵Ŀ���Ȩ",  pSept->name);
                  }
                  else
                  {
                    pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"��ս�ļ�������Ѿ���ɢ");
                  }
                }
              }
            default:
              break;
          }
        }
        else
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_MSG,"���˵�˼��������,�޷�����������");
        }
        return true;
      }
      break;
    case Cmd::Session::PARA_SEPT_NPCDARE_DARE:
      {
        Cmd::Session::t_NpcDare_Dare_SceneSession *rev = (Cmd::Session::t_NpcDare_Dare_SceneSession *)pNullCmd;
        processRequest(rev);
        return true;
      }
      break;
    case Cmd::Session::PARA_SEPT_NPCDARE_RESULT:
      {
        Cmd::Session::t_NpcDare_Result_SceneSession *rev = (Cmd::Session::t_NpcDare_Result_SceneSession *)pNullCmd;
        CNpcDareObj * Obj = getNpcDareObjBySept(rev->dwSeptID);
        if (Obj)
        {
          Obj->processResult(rev->dwSeptID);
        }
        return true;
      }
      break;
    case Cmd::Session::PARA_SEPT_NPCDARE_GMCMD:
      {
        doDare();
        _notDare = false;
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}


/**
* \brief �����ü����з�ռ�û���������ս��ҵNPC
* \param septid ����id
* \return true �ɹ��ҵ��ü������Ϣ false �ļ����������Ϣ����ҵNPCȺ����
*/
bool CNpcDareM::searchSept(DWORD septid)
{
  std::vector<CNpcDareObj*>::iterator vIterator;

  for(vIterator = _objList.begin(); vIterator!=_objList.end(); vIterator++)
  {
    if ((*vIterator)->get_dareseptid() == septid ||
      (*vIterator)->get_holdseptid() == septid )
    {
      return true;
    }
  }
  return false;
}

/**
* \brief �����ü����з�ռ����ҵNPC
* \param septid ����id
* \return true �ɹ��ҵ��ü������Ϣ false �ļ�����ռ����Ϣ����ҵNPCȺ����
*/
CNpcDareObj* CNpcDareM::searchSeptHold(DWORD septid)
{
  std::vector<CNpcDareObj*>::iterator vIterator;

  for(vIterator = _objList.begin(); vIterator!=_objList.end(); vIterator++)
  {
    if ((*vIterator)->get_holdseptid() == septid )
    {
      return *vIterator;
    }
  }
  return NULL;
}

/**
* \brief �����ü����з�ռ�û���������ս��ҵNPC
* \param septid ����id
* \return true �ɹ��ҵ��ü������Ϣ false �ļ����������Ϣ����ҵNPCȺ����
*/
CNpcDareObj* CNpcDareM::searchRecord(DWORD dwCountryID,DWORD dwMapID,DWORD dwNpcID)
{
  std::vector<CNpcDareObj*>::iterator vIterator;

  for(vIterator = _objList.begin(); vIterator!=_objList.end(); vIterator++)
  {
    if ((*vIterator)->get_country() == dwCountryID &&
      (*vIterator)->get_mapid() == dwMapID &&
      (*vIterator)->get_npcid() == dwNpcID)
    {
      return (*vIterator);
    }
  }
  return NULL;
}

/**
* \brief �����иü�����Ϣ����ս��¼
* \param septid ����id
* \return true ��ս����,�Ҳ�������NULL
*/
CNpcDareObj* CNpcDareM::getNpcDareObjBySept(DWORD septid)
{
  std::vector<CNpcDareObj*>::iterator vIterator;

  for(vIterator = _objList.begin(); vIterator!=_objList.end(); vIterator++)
  {
    if ((*vIterator)->get_dareseptid() == septid ||
      (*vIterator)->get_holdseptid() == septid )
    {
      return (*vIterator);
    }
  }
  return NULL;
}

void CNpcDareM::sendUserData(UserSession *pUser)
{
  DWORD septid = CSeptM::getMe().getSeptIDByUserName(pUser->name);
  if (0 != septid) 
  {
    CNpcDareObj * cnd = this->searchSeptHold(septid);
    if (cnd)
    {
      Cmd::Session::t_notifyNpcHoldData send;
      send.dwUserID = pUser->id;
      send.dwMapID = cnd->get_mapid();
      send.dwPosX = cnd->get_posx();
      send.dwPosY = cnd->get_posy();
      pUser->scene->sendCmd(&send,sizeof(send));
      return ;
    }  
  }

  Cmd::Session::t_notifyNpcHoldData send;
  send.dwUserID = pUser->id;
  send.dwMapID = 0;
  send.dwPosX = 0;
  send.dwPosY = 0;
  pUser->scene->sendCmd(&send,sizeof(send));  
}

//------------------------------------------------------------------------------------------------------------

/**
* \brief �������ݿ��¼
*/
void CNpcDareObj::writeDatabase()
{
  const dbCol npcdare_update_define[] = {
    { "`HOLDSEPTID`",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`DARESEPTID`",  zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { "`GOLD`",      zDBConnPool::DB_DWORD,  sizeof(DWORD) },
    { NULL,0,0}
  };

  struct {
    DWORD dwHoldSeptID;
    DWORD dwDareSeptID;
    DWORD dwGold;
  }updatenpcdare_data;
  char where[128];

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    Zebra::logger->error("���ܻ�ȡ���ݿ���");
    return;
  }

  bzero(&updatenpcdare_data,sizeof(updatenpcdare_data));

  updatenpcdare_data.dwHoldSeptID = _dwHoldSeptID;
  updatenpcdare_data.dwDareSeptID = _dwDareSeptID;
  updatenpcdare_data.dwGold = _dwGold;

  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"COUNTRY = '%u' AND MAPID = '%u' AND NPCID = '%u'",_dwCountry,_dwMapID,_dwNpcID);

  DWORD retcode = SessionService::dbConnPool->exeUpdate(handle,"`NPCDARE`",npcdare_update_define,(BYTE*)(&updatenpcdare_data),where);
  SessionService::dbConnPool->putHandle(handle);

  if (1 != retcode)
  {
    Zebra::logger->error("�޸�NPC��ս����ʧ�ܣ�country=%u,mapid=%u npcid=%u",_dwCountry,_dwMapID,_dwNpcID);
  }
}


/**
* \brief ��ʼ������һ��NPCDare����
*/
void CNpcDareObj::create(NpcDareDef::NpcDareRecord &record)
{
  _dwCountry  = record.dwCountry;
  _dwMapID  = record.dwMapID;
  _dwNpcID  = record.dwNpcID;
  _dwHoldSeptID = record.dwHoldSeptID;
  _dwDareSeptID = record.dwDareSeptID;
  _dwGold    = record.dwGold;
  _dwPosX    = record.dwPosX;
  _dwPosY    = record.dwPosY;
  _dwResultHold = 0;
  _dwResultDare = 0;
  _dareStep  =  0;
}

/**
* \brief ������Ʒ����
* \param pUser ��ɫ
*/
void CNpcDareObj::itemBack(UserSession *pUser)
{
  Cmd::Session::t_NpcDare_ItemBack_SceneSession send;
  send.dwUserID = pUser->id;
  pUser->scene->sendCmd(&send,sizeof(send));
}

/**
* \brief ��ս����
* \param userID ��ɫ
*/
void CNpcDareObj::dareRequest(DWORD userId)
{
  DWORD septid = 0;
  UserSession *pUser = NULL;
  pUser = UserSessionManager::getInstance()->getUserByID(userId);
  if (NULL == pUser) return;

  if ((septid=CSeptM::getMe().findUserSept(userId)) == 0)
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�޷�������ս�㲻���峤��");
//    itemBack(pUser);
  }
  else
  {
    if (!CNpcDareM::getMe().searchSept(septid)) //�����ļ����з���ս��ռ����ҵNPC
    {
      CSept *pSept = CSeptM::getMe().getSeptByID(_dwHoldSeptID);
      if (_dwHoldSeptID == 0 ||((_dwHoldSeptID !=0) && (pSept==NULL)))
      {
        _dwHoldSeptID = septid;
        itemBack(pUser);
        writeDatabase();
        pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"���Ѿ������˸�����,����ÿ��������ȡ����ѣ�");
        CSeptM::getMe().notifyNpcHoldData(septid);
      }
      else
      {
        if (_dwDareSeptID == 0)
        {
          // ������ս
          struct tm  tv1;
          time_t timValue = time(NULL);
          zRTime::getLocalTime(tv1,timValue);
          if (tv1.tm_hour <18 || tv1.tm_hour>20)
          {
            //������ս
            _dwDareSeptID = septid;
            writeDatabase();
            itemBack(pUser);
            pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�����ս�����ѱ����ܣ�");
            SceneSession *scene= SceneSessionManager::getInstance()->getSceneByID((this->_dwCountry<<16)+this->_dwMapID);
            if (scene)
            {
              CSeptM::getMe().sendSeptNotify(this->_dwHoldSeptID,"�м���׼�����������%s(%u,%u)���������˿���Ȩ����ս",scene->name,this->_dwPosX,this->_dwPosY);
              CSeptM::getMe().sendSeptNotify(this->_dwDareSeptID,"���彫׼��������������%s(%u,%u)���������˿���Ȩ����ս",scene->name,this->_dwPosX,this->_dwPosY);
            }
            else
            {
              CSeptM::getMe().sendSeptNotify(this->_dwHoldSeptID,"�м���׼��������巢�����˿���Ȩ����ս,��׼����");
              CSeptM::getMe().sendSeptNotify(this->_dwDareSeptID,"���彫׼�����������巢�����˿���Ȩ����ս,��׼����");
            }
          }
          else
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�����ݲ�������սҵ��");
//            itemBack(pUser);
          }
        }
        else
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"��Ǹ��������,�����Ѿ���һ����ս�ˣ�");
//          itemBack(pUser);
        }
      }
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�㲻�ܷ�����ս��");
//      itemBack(pUser);
    }
  }
}

/**
* \brief �жϼ���Ŀ���ǲ��Ǳ�����
* \param country ���� id
* \param mapid ��ͼ id
* \param npcid NPC id
*/
bool CNpcDareObj::isMe(DWORD country,DWORD mapid,DWORD npcid)
{
  if (_dwCountry == country &&
    _dwMapID == mapid &&
    _dwNpcID == npcid)
    return true;
  else
    return false;
}

/**
* \brief ִ�ж�ս��ʼ����
* \return true �ɹ���ս false δ�п�ս
*/
bool CNpcDareObj::doDare()
{
  if (this->_dwDareSeptID !=0 &&
    this->_dwHoldSeptID !=0)
  {
    _dwResultDare = 0;
    _dwResultHold = 0;
    _dareStep =0;

    std::vector<DWORD> dare_list;
    dare_list.push_back(this->_dwDareSeptID);

    Cmd::Session::t_NpcDare_NotifyScene_SceneSession send;
    send.dwCountryID = this->_dwCountry;
    send.dwMapID = this->_dwMapID;
    send.dwNpcID = this->_dwNpcID;
    send.dwPosX  = this->_dwPosX;
    send.dwPoxY  = this->_dwPosY;

    CSeptM::getMe().sendNpcDareCmdToScene(this->_dwDareSeptID,&send,sizeof(send));
    CSeptM::getMe().sendNpcDareCmdToScene(this->_dwHoldSeptID,&send,sizeof(send));
    CSeptM::getMe().sendSeptNotify(this->_dwDareSeptID,"��������ս��ʼ,��ս��Ա����֮ǰ��Ҫ�뿪ս��");
    CSeptM::getMe().sendSeptNotify(this->_dwHoldSeptID,"��������ս��ʼ,��ս��Ա����֮ǰ��Ҫ�뿪ս��");


    Cmd::Session::t_createDare_SceneSession pCmd;
#ifdef _DEBUG
    pCmd.active_time  =  60;
#else
    pCmd.active_time  =  600;
#endif
    pCmd.ready_time  =  1;

    pCmd.relationID2  =  this->_dwHoldSeptID;
    pCmd.type      =  Cmd::SEPT_NPC_DARE;

    CDareM::getMe().createDare_sceneSession(&pCmd,dare_list);
    return true;
  }
  return false;
}


/**
* \brief ִ�ж�ս��ʼ��֪ͨ
* \return true �ɹ���ս false δ�п�ս
*/
bool CNpcDareObj::notifyDareReady()
{
  if (_dwGold< 1000000)
  {
    if (this->_dwDareSeptID ==0 &&
      this->_dwHoldSeptID ==0)
    {
      _dwGold = 0;
    }
    _dwGold += 4000;
    this->writeDatabase();
  }
  if (this->_dwDareSeptID !=0 &&
    this->_dwHoldSeptID !=0)
  {
    SceneSession *scene= SceneSessionManager::getInstance()->getSceneByID((this->_dwCountry<<16)+this->_dwMapID);
    if (scene)
    {
      CSeptM::getMe().sendSeptNotify(this->_dwDareSeptID,"5�����Ժ�����ս��ʼ,�뵽%s��%u,%u�����꼯�ϣ�",scene->name,this->_dwPosX,this->_dwPosY);
      CSeptM::getMe().sendSeptNotify(this->_dwHoldSeptID,"5�����Ժ�����ս��ʼ,�뵽%s��%u,%u�����꼯�ϣ�",scene->name,this->_dwPosX,this->_dwPosY);
    }
    else
    {
      CSeptM::getMe().sendSeptNotify(this->_dwDareSeptID,"�뵽���˴�����,5�����Ժ����˿���Ȩ����ս��ʼ");
      CSeptM::getMe().sendSeptNotify(this->_dwHoldSeptID,"�뵽���˴�����,5�����Ժ����˿���Ȩ����ս��ʼ");
    }
    return true;
  }
  return false;
}

/**
* \brief �ռ���ս���
* \param septid ����
*/
void CNpcDareObj::processResult(DWORD septid)
{
  if (this->_dwDareSeptID == septid)
  {
    _dareStep = 1;
    _dwResultDare++;
#ifdef _DEBUG
  Zebra::logger->debug("�յ���ս�������id=%u _dwResultDare=%u",septid,_dwResultDare);
#endif
  }
  if (this->_dwHoldSeptID == septid)
  {
    _dareStep = 1;
    _dwResultHold++;
#ifdef _DEBUG
  Zebra::logger->debug("�յ���ս�������id=%u _dwResultHold=%u",septid,_dwResultHold);
#endif
  }
  resultTime = SessionTimeTick::currentTime;
}

/**
* \brief ǿ�ƽ��ж�ս�������
*/
void CNpcDareObj::forceProcessResult()
{
  if (0 == _dareStep)
  {
    _dareStep=1;
    resultTime = SessionTimeTick::currentTime;
  }
}


/**
* \brief ֪ͨ��ս���
* \param septid ����
*/
void CNpcDareObj::doResult()
{
  if ((1==_dareStep) && SessionTimeTick::currentTime.sec() - resultTime.sec() >120)
  {
    Zebra::logger->info("[����]����NPC����ս������[%u]����ʣ��[%d]����ս��[%u]����ʣ��[%d]��MAPID=[%u] NPCID=[%u]",this->_dwHoldSeptID,this->_dwResultHold,this->_dwDareSeptID,this->_dwResultDare,this->_dwMapID,this->_dwNpcID);
    if (this->_dwResultHold>= this->_dwResultDare)
    {
      CSeptM::getMe().changeRepute(this->_dwDareSeptID,-5);
        
      CSeptM::getMe().sendSeptNotify(this->_dwHoldSeptID,"��ϲ��,��ļ��屣ס�����˵Ŀ���Ȩ��");
      CSeptM::getMe().sendSeptNotify(this->_dwDareSeptID,"���ź�,��ļ�����սʧ�ܣ�ʧȥ5��������");
    }
    else
    {
      CSeptM::getMe().changeRepute(this->_dwHoldSeptID,-5);
      
      CSeptM::getMe().sendSeptNotify(this->_dwDareSeptID,"��ϲ��,��ļ���Ӯ�������˵Ŀ���Ȩ��");
      CSeptM::getMe().sendSeptNotify(this->_dwHoldSeptID,
        "���ź�,��ļ���ʧȥ�����˵Ŀ���Ȩ��ʧȥ5������");

      this->_dwHoldSeptID = this->_dwDareSeptID;
    }
    CSeptM::getMe().notifyNpcHoldData(this->_dwHoldSeptID);
    CSeptM::getMe().notifyNpcHoldData(this->_dwDareSeptID);
    this->_dwDareSeptID = 0;
    this->_dareStep  =  2;
    this->writeDatabase();
  }
}

/**
* \brief �����ɫ��ȡ������
*/
void CNpcDareObj::processGetGold(UserSession *pUser,DWORD septid,DWORD dwNpcID,DWORD dwMapID,DWORD dwCountryID)
{
  if (this->_dwHoldSeptID == septid &&
    this->_dwMapID == dwMapID &&
    this->_dwCountry == dwCountryID &&
    this->_dwNpcID == dwNpcID)
  {
    if (this->_dwGold >0)
    {
      Cmd::Session::t_NpcDare_GetGold_SceneSession send;
      send.dwUserID = pUser->id;
      send.dwGold = _dwGold;
      pUser->scene->sendCmd(&send,sizeof(send));
      Zebra::logger->info("[����]��ɫ%s��ȡ��NPC˰��%u",pUser->name,_dwGold);
      _dwGold = 0;
      this->writeDatabase();
    }
    else
    {
      pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�����Ѿ��չ�˰��,�����ٴ���ȡ��");
    }
  }
  else
  {
    pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"���޷�������ȡ˰��");
  }

}

/**
* \brief ��ȡnpc���ڹ���
*/
DWORD CNpcDareObj::get_country()
{
  return _dwCountry;
}

/**
* \brief ��ȡnpc���ڵ�ͼ
*/
DWORD CNpcDareObj::get_mapid()
{
  return _dwMapID;
}

/**
* \brief ��ȡnpc��id
*/
DWORD CNpcDareObj::get_npcid()
{
  return _dwNpcID;
}

/**
* \brief ��ȡ��ǰ����npc�ļ���id
*/
DWORD CNpcDareObj::get_holdseptid()
{
  return _dwHoldSeptID;
}

/**
* \brief ��ȡ��ǰ��սnpc�ļ���id
*/
DWORD CNpcDareObj::get_dareseptid()
{
  return _dwDareSeptID;
}

/**
* \brief ��ȡnpc���ڵ�˰��
*/
DWORD CNpcDareObj::get_gold()
{
  return _dwGold;
}

/**
* \brief ��ȡnpc��x����
*/
DWORD CNpcDareObj::get_posx()
{
  return _dwPosX;
}

/**
* \brief ��ȡnpc��y����
*/
DWORD CNpcDareObj::get_posy()
{
  return _dwPosY;
}

/**
* \brief �������NPC
*/
void CNpcDareObj::abandon_npc()
{
  CSeptM::getMe().sendSeptNotify(_dwHoldSeptID,"����������������Ȩ");
  _dwHoldSeptID = 0;  
  _dwGold = 0;
  this->writeDatabase();
}

