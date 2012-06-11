#include <zebra/SessionServer.h>

const dbCol cartoon_load_define[] = {
  { "CARTOONID",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "NAME",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE]) },
  { "NPCID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MASTERID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MASTERNAME",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE]) },
  { "LEVEL",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "EXP",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MAXEXP",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "ADDEXP",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "STATE",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "ADOPTER",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE]) },
  { "TIME",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "SP",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MAXSP",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MASTERLEVEL",  zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "REPAIR",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { NULL,0,0}           
};

const dbCol cartoon_update_define[] = {
  { "NAME",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE]) },
  { "NPCID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MASTERID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MASTERNAME",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE]) },
  { "LEVEL",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "EXP",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MAXEXP",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "ADDEXP",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "STATE",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "ADOPTER",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE]) },
  { "TIME",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "SP",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MAXSP",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MASTERLEVEL",  zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "REPAIR",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { NULL,0,0}           
};

CartoonPetService*CartoonPetService::cs = 0;

CartoonPetService::CartoonPetService()
{
  repairData();
  if (!loadAllFromDB())
    Zebra::logger->error("[����]�����������ʧ��");

  group = 0;
}
CartoonPetService::~CartoonPetService()
{
}

CartoonPetService& CartoonPetService::getMe()
{
  if (!cs)
    cs = new CartoonPetService();
  return *cs;
}

void CartoonPetService::delMe()
{
  if (cs)
  {
    cs->writeAllToDB(false);
    SAFE_DELETE(cs);
    cs = 0;
  }
}

bool CartoonPetService::doCartoonCmd(const Cmd::Session::t_CartoonCmd *cmd,const DWORD cmdLen)
{
#ifdef _DEBUG
  //zThread::msleep(3000);
#endif
  using namespace Cmd;
  using namespace Cmd::Session;

  switch (cmd->cartoonPara)
  {
    case PARA_CARTOON_SET_REPAIR:
      {
        t_setRepairCartoon_SceneSession * rev = (t_setRepairCartoon_SceneSession *)cmd;
        if (cartoonPetMap[rev->userID].find(rev->cartoonID)==cartoonPetMap[rev->userID].end())
          return false;
        cartoonPetList[rev->cartoonID].repair = rev->repair;

        /*
        if (cartoonPetList[rev->cartoonID].state==CARTOON_STATE_WAITING
            || cartoonPetList[rev->cartoonID].state==CARTOON_STATE_ADOPTED)
        {
          UserSession * pUser = UserSessionManager::getInstance()->getUserByID(rev->userID);
          if (pUser)
          {
            stAddCartoonCmd send;
            send.isMine = true;
            send.cartoonID = rev->cartoonID;
            send.data = cartoonPetList[rev->cartoonID];
            pUser->sendCmdToMe(&send,sizeof(send));
          }
        }
        */
        Zebra::logger->info("[�Զ�����]%s(%u) %s�Զ�����",cartoonPetList[rev->cartoonID].masterName,cartoonPetList[rev->cartoonID].masterID,rev->repair?"��":"�ر�");
        return true;
      }
      break;
    case PARA_CARTOON_DRAW:
      {
        t_drawCartoon_SceneSession * rev = (t_drawCartoon_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByID(rev->userID);
        if (!pUser) return false;

        if (cartoonPetList.find(rev->cartoonID)==cartoonPetList.end()
            || cartoonPetList[rev->cartoonID].masterID!=pUser->id
            || 0==cartoonPetList[rev->cartoonID].addExp
            || Cmd::CARTOON_STATE_WAITING==cartoonPetList[rev->cartoonID].state
            || Cmd::CARTOON_STATE_ADOPTED==cartoonPetList[rev->cartoonID].state)
        {
          Zebra::logger->error("[����]%s(%u) ��ȡ��������ʧ�� cartoondID=%u addExp=%u state=%u"
              ,pUser->name,pUser->id,rev->cartoonID
              ,cartoonPetList[rev->cartoonID].addExp,cartoonPetList[rev->cartoonID].state);
          return false;
        }

        rev->num = cartoonPetList[rev->cartoonID].addExp;
        cartoonPetList[rev->cartoonID].addExp = 0;
        if (!writeDB(rev->cartoonID,cartoonPetList[rev->cartoonID]))
        {
          Zebra::logger->error("[����]%s(%u) ��ȡ���ﾭ��ʧ�� cartoonID=%u addExp=%u",pUser->name,pUser->id,rev->cartoonID,rev->num);
          cartoonPetList[rev->cartoonID].addExp = rev->num;
          return false;
        }
        pUser->scene->sendCmd(rev,cmdLen);
        return true;
      }
      break;
    case PARA_CARTOON_SALE:
      {
        t_saleCartoon_SceneSession * rev = (t_saleCartoon_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByID(rev->userID);
        if (!pUser) return false;

        if (cartoonPetList.find(rev->cartoonID)==cartoonPetList.end()) return false;

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle) 
        {                       
          Zebra::logger->error("[����]PARA_CARTOON_SALE:�õ����ݿ���ʧ��");
          return false;
        }

        char where[128];
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where)-1,"CARTOONID=%u",rev->cartoonID);
        DWORD ret = SessionService::dbConnPool->exeDelete(handle,"`CARTOONPET`",where);
        SessionService::dbConnPool->putHandle(handle);
        if (1!=ret) return false;

        cartoonPetList.erase(rev->cartoonID);
        modifyList.erase(rev->cartoonID);
        //cartoonPetMap[pUser->id].erase(rev->cartoonID);
        cartoonPetMap[pUser->id].erase(rev->cartoonID);

        pUser->scene->sendCmd(cmd,cmdLen);
        return true;
      }
      break;
    case PARA_CARTOON_BUY:
      {
        t_buyCartoon_SceneSession * rev = (t_buyCartoon_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByID(rev->data.masterID);
        if (!pUser) return false;

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle) 
        {                       
          Zebra::logger->error("[����]PARA_CARTOON_BUY:�õ����ݿ���ʧ��");
          return false;
        }

        char where[128];
        bzero(where,sizeof(where));
        _snprintf(where,sizeof(where)-1,"MASTERID=%u",pUser->id);

        /*
        cartoon_load_struct * dataList;
        DWORD retcode=SessionService::dbConnPool->exeSelect(handle,"`CARTOONPET`",cartoon_load_define,where,NULL,(BYTE**)&dataList);
        SAFE_DELETE_VEC(dataList);
        if (retcode>=1)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ĳ���̫����");
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }
        */
        if (cartoonPetMap[pUser->id].size()>=2)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��ĳ���̫����");
          SessionService::dbConnPool->putHandle(handle);
          Zebra::logger->info("[����]��⵽ %s �ظ��������,��ʧ���� %u ��",pUser->name,rev->data.npcID==9005?5000:1000);
          return true;
        }

        DWORD retcode = SessionService::dbConnPool->exeInsert(handle,"`CARTOONPET`",cartoon_update_define,(const BYTE *)&rev->data);
        SessionService::dbConnPool->putHandle(handle);

        if ((DWORD)-1 == retcode)
        {                       
          Zebra::logger->error("[����]PARA_CARTOON_BUY: ������ݿ�ʧ�� retcode=%d",retcode);
          return false;
        }

        cartoonPetList[retcode] = rev->data;
        modifyList.insert(retcode);
        cartoonPetMap[pUser->id].insert(retcode);

        //֪ͨ����
        t_addCartoon_SceneSession ret;
        ret.userID = pUser->id;
        ret.cartoonID = retcode;
        ret.data = rev->data;
        pUser->scene->sendCmd(&ret,sizeof(t_addCartoon_SceneSession));

        pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�㹺����һֻ������,�����԰������ø���ľ���");
        return true;
      }
      break;
    case PARA_CARTOON_GET_LIST:
      {
        t_getListCartoon_SceneSession *rev = (t_getListCartoon_SceneSession *)cmd;
        checkAdoptable(rev->userID);
        return true;
      }
      break;
    case PARA_CARTOON_SAVE:
      {
        t_saveCartoon_SceneSession *rev = (t_saveCartoon_SceneSession *)cmd;

        if (cartoonPetList.find(rev->cartoonID)==cartoonPetList.end())
          return true;

        switch (rev->type)
        {
          case SAVE_TYPE_LETOUT:
          case SAVE_TYPE_PUTAWAY:
          case SAVE_TYPE_TIMETICK:
            if (cartoonPetList[rev->cartoonID].state!=CARTOON_STATE_FOLLOW
                && cartoonPetList[rev->cartoonID].state!=CARTOON_STATE_PUTAWAY)
              return false;

            cartoonPetList[rev->cartoonID] = rev->data;
            break;
          case SAVE_TYPE_RETURN:
          case SAVE_TYPE_TIMEOVER:
          case SAVE_TYPE_SYN:
            if (cartoonPetList[rev->cartoonID].state!=CARTOON_STATE_ADOPTED
                || strncmp(cartoonPetList[rev->cartoonID].adopter,rev->userName,MAX_NAMESIZE))
            {
              Zebra::logger->error("[����]����ָ����� cartoonID=%u state=%u saveType=%u",rev->cartoonID,cartoonPetList[rev->cartoonID].state,rev->type);
              return false;
            }

            if (rev->type==SAVE_TYPE_RETURN || rev->type==SAVE_TYPE_TIMEOVER)
              adoptedPetMap[rev->userName].erase(rev->cartoonID);

            cartoonPetList[rev->cartoonID] << rev->data;

            break;
          default:
            break;
        }

        modifyList.insert(rev->cartoonID);
        if (rev->type!=SAVE_TYPE_SYN && rev->type!=SAVE_TYPE_TIMETICK)
          Zebra::logger->debug("[����]������� %s(%u) addExp=%u time=%u saveType=%u state=%u level=%u userName=%s",rev->data.masterName,rev->cartoonID,cartoonPetList[rev->cartoonID].addExp,rev->data.time,rev->type,cartoonPetList[rev->cartoonID].state,cartoonPetList[rev->cartoonID].lv,rev->userName);

        UserSession * pMaster = UserSessionManager::getInstance()->getUserByID(rev->data.masterID);
        switch (rev->type)
        {
          case SAVE_TYPE_LETOUT:
            {
              waitingList.insert(rev->cartoonID);

              stAddCartoonCmd send;
              send.isMine = true;
              send.cartoonID = rev->cartoonID;
              send.data = rev->data;
              if (pMaster) pMaster->sendCmdToMe(&send,sizeof(send));

              Cmd::stAddWaitingCartoonCmd add;
              add.cartoonID = rev->cartoonID;
              add = rev->data;
              sendCmdToItsFriendAndFamily(rev->data.masterID,rev->data.masterName,&add,sizeof(add));
            }
            break;
          case SAVE_TYPE_RETURN:
            {
              waitingList.insert(rev->cartoonID);

              Cmd::stAddWaitingCartoonCmd add;
              add.cartoonID = rev->cartoonID;
              add = rev->data;
              sendCmdToItsFriendAndFamily(rev->data.masterID,rev->data.masterName,&add,sizeof(add));

              if (pMaster)
              {
                t_addCartoon_SceneSession ac;
                ac.userID = rev->data.masterID;
                ac.cartoonID = rev->cartoonID;
                ac.data = cartoonPetList[rev->cartoonID];
                pMaster->scene->sendCmd(&ac,sizeof(ac));
              }
            }
            break;
            /*
          case SAVE_TYPE_CHARGE:
            if (pMaster)
            {
              t_addCartoon_SceneSession ac;
              ac.userID = rev->data.masterID;
              ac.cartoonID = rev->cartoonID;
              bcopy(&rev->data,&ac.data,sizeof(Cmd::t_CartoonData));
              pMaster->scene->sendCmd(&ac,sizeof(ac));
            }
            break;
            */
          case SAVE_TYPE_TIMEOVER:
            {
              waitingList.erase(rev->cartoonID);
              if (pMaster)
              {
                t_addCartoon_SceneSession ac;
                ac.userID = rev->data.masterID;
                ac.cartoonID = rev->cartoonID;
                ac.data = cartoonPetList[rev->cartoonID];
                pMaster->scene->sendCmd(&ac,sizeof(ac));
              }
              stRemoveWaitingCartoonCmd send;
              send.cartoonID = rev->cartoonID;
              sendCmdToItsFriendAndFamily(rev->data.masterID,rev->data.masterName,&send,sizeof(send));
            }
            break;
          case SAVE_TYPE_PUTAWAY:
            waitingList.erase(rev->cartoonID);
          case SAVE_TYPE_SYN:
            {
              if (pMaster)
              {
                t_addCartoon_SceneSession ac;
                ac.userID = rev->data.masterID;
                ac.cartoonID = rev->cartoonID;
                ac.data = cartoonPetList[rev->cartoonID];
                pMaster->scene->sendCmd(&ac,sizeof(ac));
              }
            }
            break;
          default:
            break;
        }
        return true;
      }
      break;
    case PARA_CARTOON_ADOPT:
      {
        t_adoptCartoon_SceneSession *rev = (t_adoptCartoon_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByID(rev->userID);
        if (!pUser) return false;

        if (cartoonPetList.find(rev->cartoonID)==cartoonPetList.end())
          return true;

        Cmd::t_CartoonData cd = cartoonPetList[rev->cartoonID];
        if (cd.state==Cmd::CARTOON_STATE_ADOPTED)//����,���Ϊ������ȴ�Ҳ���������ʱ���Լ���
        {
          UserSession * adopter = UserSessionManager::getInstance()->getUserSessionByName(cd.adopter);
          if (adopter && adopter!=pUser) return false;
        }
        if (cd.state!=Cmd::CARTOON_STATE_WAITING)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�������Ŀǰ���ڵȴ�״̬,�����Ѿ�����������");
          return true;
        }

        UserSession * pMaster = UserSessionManager::getInstance()->getUserByID(cd.masterID);
        //�������
        if (adoptedPetMap[pUser->name].end()!=adoptedPetMap[pUser->name].find(rev->cartoonID)) return true;
        if (adoptedPetMap[pUser->name].size()>=5)
        {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���������������,���ܼ���");
            return true;
        }
        //����ǲ����Ѿ��г��ﱻ������
        if (pMaster)
        {
            for (hash_set<DWORD>::iterator it=cartoonPetMap[pMaster->id].begin(); it!=cartoonPetMap[pMaster->id].end(); it++)
            {
                if (*it!=rev->cartoonID
                        && cartoonPetList[*it].state!=Cmd::CARTOON_STATE_PUTAWAY
                        && cartoonPetList[*it].state!=Cmd::CARTOON_STATE_FOLLOW)
                    return true;
            }
        }

        /*
        DWORD adopterCount = 0;
        for (std::map<DWORD,Cmd::t_CartoonData>::iterator it=cartoonPetList.begin(); it!=cartoonPetList.end(); it++)
        {
          if (it->second.masterID==cd.masterID
              && it->first!=rev->cartoonID
              && it->second.state!=Cmd::CARTOON_STATE_PUTAWAY
              && it->second.state!=Cmd::CARTOON_STATE_FOLLOW)
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����������ڲ�������");
            return true;
          }

          if (it->second.state==Cmd::CARTOON_STATE_ADOPTED
              && 0==strncmp(it->second.adopter,pUser->name,MAX_NAMESIZE))
          {
            adopterCount++;
            if (adopterCount>=5)
            {
              pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"���������������,���ܼ���");
              return true;
            }
          }
        }
        */

        bool isMyFriendOrFamily = false;
        //�Ƿ����
        CRelation * pRelation = pUser->relationManager.getRelationByID(cd.masterID);
        if (pRelation && pRelation->type!=Cmd::RELATION_TYPE_BAD
            && pRelation->type!=Cmd::RELATION_TYPE_ENEMY)
          isMyFriendOrFamily = true;
        //�Ƿ�����Ա
        if (!isMyFriendOrFamily)
        {
          CSeptMember * pMember = CSeptM::getMe().getMemberByName(cd.masterName);
          if (pMember && pMember->mySept->getID()==pUser->septid)
            isMyFriendOrFamily = true;
        }

        if (!isMyFriendOrFamily)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ʧ��,�㲻�Ǽ����Ա�����");
          return true;
        }

        //����
        cartoonPetList[rev->cartoonID].state = Cmd::CARTOON_STATE_ADOPTED;
        strncpy(cartoonPetList[rev->cartoonID].adopter,pUser->name,MAX_NAMESIZE);
        modifyList.insert(rev->cartoonID);
        waitingList.erase(rev->cartoonID);

        adoptedPetMap[pUser->name].insert(rev->cartoonID);

        if (pMaster)
        {
          t_addCartoon_SceneSession send;
          send.userID = cd.masterID;
          send.cartoonID = rev->cartoonID;
          send.data = cartoonPetList[rev->cartoonID];
          pMaster->scene->sendCmd(&send,sizeof(send));

          rev->masterState = 1;//��������,�����ͷ�3������
        }

        //֪ͨ���Ѻͼ���
        Cmd::stRemoveWaitingCartoonCmd rc;
        rc.cartoonID = rev->cartoonID;
        sendCmdToItsFriendAndFamily(cd.masterID,cd.masterName,&rc,sizeof(rc),pUser->name);

        //���
        rev->data = cartoonPetList[rev->cartoonID];
        rev->data.addExp = 0;
        pUser->scene->sendCmd(rev,cmdLen);

        Zebra::logger->debug("[����]%s ������ %s ��������� %s(%u)",pUser->name,cd.masterName,cd.name,rev->cartoonID);

        return true;
      }
      break;
    case PARA_CARTOON_GET_BACK:
      {
        t_getBackCartoon_SceneSession *rev = (t_getBackCartoon_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByID(rev->userID);
        if (!pUser) return false;

        cartoon_load_struct ld;
        for (hash_set<DWORD>::iterator it=cartoonPetMap[pUser->id].begin(); it!=cartoonPetMap[pUser->id].end(); it++)
        {
          if ((cartoonPetList[*it].state==Cmd::CARTOON_STATE_WAITING 
                || cartoonPetList[*it].state==Cmd::CARTOON_STATE_ADOPTED))
          {
            ld.cartoonID = *it;
            ld.data = cartoonPetList[*it];
            break;
          }
        }
        if (0==ld.cartoonID) return true;

        if (ld.data.state==Cmd::CARTOON_STATE_ADOPTED)
        {
          UserSession *pAdopter = UserSessionManager::getInstance()->getUserSessionByName(ld.data.adopter);
#ifdef _DEBUG
          //pAdopter = 0;
#endif
          if (pAdopter)
          {
            rev->userID = pAdopter->id;
            rev->cartoonID = ld.cartoonID;
            pAdopter->scene->sendCmd(rev,cmdLen);
          }
          else
          {
            //����,�����������Ҳ���������ʱ���Լ���
            cartoonPetList[ld.cartoonID].state=Cmd::CARTOON_STATE_WAITING;
            bzero(cartoonPetList[ld.cartoonID].adopter,MAX_NAMESIZE);
            adoptedPetMap[ld.data.adopter].erase(ld.cartoonID);
          }
        }

        if (cartoonPetList[ld.cartoonID].state==Cmd::CARTOON_STATE_WAITING)
        {
          cartoonPetList[ld.cartoonID].state = Cmd::CARTOON_STATE_PUTAWAY;

          waitingList.erase(ld.cartoonID);

          t_addCartoon_SceneSession ac;
          ac.userID = rev->userID;
          ac.cartoonID = ld.cartoonID;
          ac.data = cartoonPetList[ld.cartoonID];
          pUser->scene->sendCmd(&ac,sizeof(ac));

          stRemoveWaitingCartoonCmd send;
          send.cartoonID = ld.cartoonID;
          sendCmdToItsFriendAndFamily(pUser->id,pUser->name,&send,sizeof(send));
        }

        modifyList.insert(ld.cartoonID);
        return true;

      }
      break;
    case PARA_CARTOON_NOTIFY:
      {
        t_notifyCartoon_SceneSession * rev = (t_notifyCartoon_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName(rev->adopter);
        if (!pUser) return false;
        pUser->scene->sendCmd(rev,cmdLen);
        return true;
      }
      break;
    case PARA_CARTOON_LOAD:
      {
        t_loadCartoon_SceneSession * rev = (t_loadCartoon_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByID(rev->userID);
        if (!pUser) return false;
        loadFromDB(rev->userID);
        return true;
      }
      break;
    case PARA_CARTOON_CORRECT:
      {
        t_correctCartoon_SceneSession * rev = (t_correctCartoon_SceneSession *)cmd;

        if (cartoonPetList.find(rev->cartoonID)==cartoonPetList.end())
          return true;

        cartoonPetList[rev->cartoonID].state = Cmd::CARTOON_STATE_WAITING;
        bzero(cartoonPetList[rev->cartoonID].adopter,MAX_NAMESIZE);
        adoptedPetMap[cartoonPetList[rev->cartoonID].adopter].erase(rev->cartoonID);
        modifyList.insert(rev->cartoonID);

        UserSession * pMaster = UserSessionManager::getInstance()->getUserByID(cartoonPetList[rev->cartoonID].masterID);
        if (pMaster)
        {
          t_addCartoon_SceneSession ac;
          ac.userID = cartoonPetList[rev->cartoonID].masterID;
          ac.cartoonID = rev->cartoonID;
          ac.data = cartoonPetList[rev->cartoonID];
          pMaster->scene->sendCmd(&ac,sizeof(ac));
        }

        Cmd::stAddWaitingCartoonCmd add;
        add.cartoonID = rev->cartoonID;
        add = cartoonPetList[rev->cartoonID];
        sendCmdToItsFriendAndFamily(cartoonPetList[rev->cartoonID].masterID,add.masterName,&add,sizeof(add));
        return true;
      }
      break;
    case PARA_CARTOON_CHARGE:
      {
        t_chargeCartoon_SceneSession * rev = (t_chargeCartoon_SceneSession *)cmd;

        if (cartoonPetList.find(rev->cartoonID)==cartoonPetList.end())
        {
          Zebra::logger->error("[����]�������ֵʱ�Ҳ������� cartoonID=%u time=%u",rev->cartoonID,rev->time);
          return false;
        }

#ifdef _DEBUG
        //zThread::msleep(5000);
#endif
        cartoonPetList[rev->cartoonID].time += rev->time;

        if (cartoonPetList[rev->cartoonID].state==CARTOON_STATE_ADOPTED)
        {
          UserSession *pAdopter = UserSessionManager::getInstance()->getUserSessionByName(cartoonPetList[rev->cartoonID].adopter);
          if (pAdopter)
          {
            t_chargeNotifyCartoon_SceneSession send;
            strncpy(send.adopter,pAdopter->name,MAX_NAMESIZE);
            send.cartoonID = rev->cartoonID;
            send.time = rev->time;
            pAdopter->scene->sendCmd(&send,sizeof(send));

            return true;
          }
          else
          {
            cartoonPetList[rev->cartoonID].state = Cmd::CARTOON_STATE_WAITING;
            bzero(cartoonPetList[rev->cartoonID].adopter,MAX_NAMESIZE);
            adoptedPetMap[cartoonPetList[rev->cartoonID].adopter].erase(rev->cartoonID);
            modifyList.insert(rev->cartoonID);
          }
        }

        if (!writeDB(rev->cartoonID,cartoonPetList[rev->cartoonID]))
        {
          Zebra::logger->error("[����]%s �������ֵд���ݿ�ʧ�� cartoonID=%u time=%u",cartoonPetList[rev->cartoonID].masterName,rev->cartoonID,rev->time);
          return false;
        }

        UserSession *pMaster = UserSessionManager::getInstance()->getUserByID(rev->masterID);
        if (pMaster)
        {
          /*
          t_addCartoon_SceneSession ac;
          ac.userID = rev->masterID;
          ac.cartoonID = rev->cartoonID;
          ac.data = cartoonPetList[rev->cartoonID];
          pMaster->scene->sendCmd(&ac,sizeof(ac));
          */

          pMaster->sendSysChat(Cmd::INFO_TYPE_GAME,"��Ϊ %s ��ֵ %uСʱ%u��%u�� �ɹ�",cartoonPetList[rev->cartoonID].name,rev->time/3600,(rev->time%3600)/60,rev->time%60);
        }

        if (cartoonPetList[rev->cartoonID].state==CARTOON_STATE_WAITING)
        {
          Cmd::stAddWaitingCartoonCmd add;
          add.cartoonID = rev->cartoonID;
          add = cartoonPetList[rev->cartoonID];
          sendCmdToItsFriendAndFamily(rev->masterID,add.masterName,&add,sizeof(add));
        }

        Zebra::logger->info("[����]%s ������ %s(%u) ��ֵ %u ��",cartoonPetList[rev->cartoonID].masterName,cartoonPetList[rev->cartoonID].name,rev->cartoonID,rev->time);
        return true;
      }
      break;
      /*
    case PARA_CARTOON_CONSIGN:
      {
        t_consignCartoon_SceneSession * rev = (t_consignCartoon_SceneSession *)cmd;

        if (cartoonPetList.find(rev->cartoonID)==cartoonPetList.end())
        {
          Zebra::logger->error("[����]ί����������ʱ�Ҳ������� cartoonID=%u",rev->cartoonID);
          return false;
        }

        UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName(rev->name);
        UserSession * pMaster = UserSessionManager::getInstance()->getUserByID(rev->userID);

        if (!pUser)
        {
          if (pMaster)
            pMaster->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Է�������");
          return true;
        }

        if (cartoonPetList[rev->cartoonID].state!=CARTOON_STATE_PUTAWAY
            && cartoonPetList[rev->cartoonID].state!=CARTOON_STATE_WAITING)
        {
          if (pMaster)
            pMaster->sendSysChat(Cmd::INFO_TYPE_FAIL,"��������������ȴ�״̬�ſ���ί��");
          return true;
        }

        rev->userID = pUser->id;
        if (pMaster)//�Ҳ�������Ҳ����ί��
          strncpy(rev->name,pMaster->name,MAX_NAMESIZE);
        else
          bzero(rev->name,MAX_NAMESIZE);

        if (pMaster)
          pMaster->reqAdopter = pUser->id;
        pUser->scene->sendCmd(rev,sizeof(t_consignCartoon_SceneSession));

        return true;
      }
      break;
    case PARA_CARTOON_CONSIGN_RET:
      {
        t_consignRetCartoon_SceneSession * rev = (t_consignRetCartoon_SceneSession *)cmd;

        if (cartoonPetList.find(rev->cartoonID)==cartoonPetList.end())
          return true;

        UserSession * pUser = UserSessionManager::getInstance()->getUserByID(rev->userID);
        if (!pUser) return false;

        Cmd::t_CartoonData cd = cartoonPetList[rev->cartoonID];
        UserSession * pMaster = UserSessionManager::getInstance()->getUserByID(cd.masterID);

        if (0==rev->ret)
        {
          if (pMaster)
            pMaster->sendSysChat(Cmd::INFO_TYPE_FAIL,"%s �ܾ�������ĳ���",pUser->name);
          return true;
        }

        if (2==rev->ret)
        {
          if (pMaster)
            pMaster->sendSysChat(Cmd::INFO_TYPE_FAIL,"%s ������������",pUser->name);
          return true;
        }

        if (pMaster && pMaster->reqAdopter!=pUser->id)
        {
          if (pMaster->reqAdopter==pUser->id)
            pMaster->reqAdopter = 0;
          else
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����������Ѿ�ȡ��������������");
            return true;
          }
        }

        if (cd.state==Cmd::CARTOON_STATE_ADOPTED)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"��������Ѿ���������");
          return true;
        }

        //����ǲ����Ѿ��г��ﱻ������
        if (pMaster)
        {
          for (std::set<DWORD>::iterator it=cartoonPetMap[pUser->id].begin(); it!=cartoonPetMap[pUser->id].end(); it++)
          {
            if (*it!=rev->cartoonID
                && cartoonPetList[*it].state!=Cmd::CARTOON_STATE_PUTAWAY
                && cartoonPetList[*it].state!=Cmd::CARTOON_STATE_FOLLOW)
              return true;
          }
        }
        else
        {
          DWORD count = 0;
          for (std::map<DWORD,Cmd::t_CartoonData>::iterator it=cartoonPetList.begin(); it!=cartoonPetList.end(); it++)
          {
            if (count>=5) break;
            if (it->second.masterID==cd.masterID)
            {
              if (it->first!=rev->cartoonID
                  && it->second.state!=Cmd::CARTOON_STATE_PUTAWAY
                  && it->second.state!=Cmd::CARTOON_STATE_FOLLOW)
                return true;
              count++;
            }
          }
        }

        bool isMyFriendOrFamily = false;
        //�Ƿ����
        CRelation * pRelation = pUser->relationManager.getRelationByID(cd.masterID);
        if (pRelation && pRelation->type!=Cmd::RELATION_TYPE_BAD
            && pRelation->type!=Cmd::RELATION_TYPE_ENEMY)
          isMyFriendOrFamily = true;
        //�Ƿ�����Ա
        if (!isMyFriendOrFamily)
        {
          CSeptMember * pMember = CSeptM::getMe().getMemberByName(cd.masterName);
          if (pMember && pMember->mySept->getID()==pUser->septid)
            isMyFriendOrFamily = true;
        }

        if (!isMyFriendOrFamily)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ʧ��,�㲻�Ǽ����Ա�����");
          return true;
        }

        //����
        cartoonPetList[rev->cartoonID].state = Cmd::CARTOON_STATE_ADOPTED;
        strncpy(cartoonPetList[rev->cartoonID].adopter,pUser->name,MAX_NAMESIZE);
        modifyList.insert(rev->cartoonID);
        //waitingList.erase(rev->cartoonID);

        if (pMaster)
        {
          t_addCartoon_SceneSession send;
          send.userID = cd.masterID;
          send.cartoonID = rev->cartoonID;
          send.data = cartoonPetList[rev->cartoonID];
          pMaster->scene->sendCmd(&send,sizeof(send));

          pMaster->sendSysChat(Cmd::INFO_TYPE_GAME,"%s ��������ĳ��� %s",pUser->name,cd.name);
          //rev->masterState = 1;//��������,�����ͷ�3������
        }

        //���
        t_adoptCartoon_SceneSession ac;
        ac.userID = pUser->id;
        ac.cartoonID = rev->cartoonID;
        ac.data = cartoonPetList[rev->cartoonID];
        ac.data.addExp = 0;
        pUser->scene->sendCmd(&ac,sizeof(ac));

        return true;
      }
      break;
      */
    default:
      break;
  }
  return false;
}

bool CartoonPetService::writeDB(DWORD cartoonID,Cmd::t_CartoonData& data)
{
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {               
    Zebra::logger->error("[����]writeDB: �õ����ݿ���ʧ��");
    return false;
  }
  char where[128];
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where)-1,"CARTOONID=%u",cartoonID);
  DWORD retcode = SessionService::dbConnPool->exeUpdate(handle,"`CARTOONPET`",cartoon_update_define,(BYTE*)&data,where);
  SessionService::dbConnPool->putHandle(handle);

  if (1==retcode)
    modifyList.erase(cartoonID);
  return 1==retcode;
}

void CartoonPetService::loadFromDB(DWORD masterID)
{
  if (0==cartoonPetMap[masterID].size()) return;

  UserSession * pUser = UserSessionManager::getInstance()->getUserByID(masterID);
  if (!pUser) return;

  Cmd::Session::t_addCartoon_SceneSession ret;
  ret.userID = masterID;
  for (hash_set<DWORD>::iterator it=cartoonPetMap[masterID].begin(); it!=cartoonPetMap[masterID].end(); it++)
  {
      ret.cartoonID = *it;
      ret.data = cartoonPetList[*it];
      pUser->scene->sendCmd(&ret,sizeof(Cmd::Session::t_addCartoon_SceneSession));
      //cartoonPetMap[pUser->id].insert(it->first);
  }

  /*
  Cmd::Session::t_addCartoon_SceneSession ret;
  ret.userID = masterID;
  DWORD count = 0;
  for (__gnu_cxx::hash_map<DWORD,Cmd::t_CartoonData>::iterator it=cartoonPetList.begin(); it!=cartoonPetList.end(); it++)
  {
    if (count>=5) break;
    if (it->second.masterID==masterID)
    {
      ret.cartoonID = it->first;
      ret.data = it->second;
      pUser->scene->sendCmd(&ret,sizeof(Cmd::Session::t_addCartoon_SceneSession));
      cartoonPetMap[pUser->id].insert(it->first);
      count++;
    }
  }
  */
}

void CartoonPetService::checkAdoptable(DWORD id)
{
  UserSession * pUser = UserSessionManager::getInstance()->getUserByID(id);
  if (!pUser) return;

  //������
  for (hash_set<DWORD>::iterator it=waitingList.begin(); it!=waitingList.end(); it++)
  {
    if (cartoonPetList[*it].state==Cmd::CARTOON_STATE_WAITING)
    {
      CRelation * r = pUser->relationManager.getRelationByID(cartoonPetList[*it].masterID);
      if (r && (r->type==Cmd::RELATION_TYPE_FRIEND || r->type==Cmd::RELATION_TYPE_LOVE))
      {
        Cmd::stAddWaitingCartoonCmd add;
        add.cartoonID = *it;
        add = cartoonPetList[*it];
        pUser->sendCmdToMe(&add,sizeof(add));
      }

      if (!pUser->septid) continue;
      CSeptMember * pMember = CSeptM::getMe().getMemberByName(cartoonPetList[*it].masterName);
      if (pMember && pMember->mySept && pMember->mySept->id==pUser->septid)
      {
        Cmd::stAddWaitingCartoonCmd add;
        add.cartoonID = *it;
        add = cartoonPetList[*it];
        pUser->sendCmdToMe(&add,sizeof(add));
      }
    }
    else
      Zebra::logger->error("[����]�ȴ��б��г���״̬���� cartoonID=%u master=%s",*it,cartoonPetList[*it].masterName);
  }
}

void CartoonPetService::sendCmdToItsFriendAndFamily(DWORD id,const char * name,const void *cmd,DWORD len,const char * except)
{
  UserSession * pMaster = UserSessionManager::getInstance()->getUserByID(id);
  if (pMaster)
    pMaster->relationManager.sendCmdToMyFriendExcept(cmd,len,true,except);
  else
  {
    connHandleID handle = SessionService::dbConnPool->getHandle();
    if ((connHandleID)-1 == handle)
    {               
      Zebra::logger->error("[����]sendCmdToItsFriendAndFamily: �õ����ݿ���ʧ��");
      return;
    }
    char where[128];
    bzero(where,sizeof(where));
    _snprintf(where,sizeof(where)-1,"CHARID=%u",id);

    const dbCol relation_define[] = {
      { "RELATIONID",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
      { "TYPE",      zDBConnPool::DB_BYTE,sizeof(BYTE) },
      { NULL,0,0}           
    };
    cartoon_relation_struct * dataList;
    DWORD retcode=SessionService::dbConnPool->exeSelect(handle,"`SAMPLERELATION`",relation_define,where,NULL,(BYTE**)&dataList);
    SessionService::dbConnPool->putHandle(handle);
    if (retcode==(DWORD)-1)
      return;

    if (dataList)
    {
      UserSession * pUser = 0;

      for (DWORD i=0; i< retcode; i++) 
      {
        if (dataList[i].type==Cmd::RELATION_TYPE_BAD
              || dataList[i].type==Cmd::RELATION_TYPE_ENEMY)
          continue;

        pUser = UserSessionManager::getInstance()->getUserByID(dataList[i].relationID);
        if (!pUser) continue;
        if (0==strncmp(pUser->name,except,MAX_NAMESIZE)) continue;//������������

        pUser->sendCmdToMe(cmd,len);
      }                               
      SAFE_DELETE_VEC(dataList);
    }
  }
  CSeptMember * pMember = CSeptM::getMe().getMemberByName(name);
  if (!pMember) return;
  pMember->mySept->sendCmdToAllMemberExcept((const Cmd::stNullUserCmd *)cmd,len,except);
}

void CartoonPetService::userLevelUp(DWORD userID,DWORD level)
{
  UserSession * pMaster = UserSessionManager::getInstance()->getUserByID(userID);
  if (pMaster)
  {
    for (hash_set<DWORD>::iterator it=cartoonPetMap[pMaster->id].begin(); it!=cartoonPetMap[pMaster->id].end(); it++)
    {
      cartoonPetList[*it].masterLevel = level;
      if (cartoonPetList[*it].state==Cmd::CARTOON_STATE_ADOPTED)
      {
        UserSession *pAdopter = UserSessionManager::getInstance()->getUserSessionByName(cartoonPetList[*it].adopter);
        if (pAdopter)
        {
          Cmd::Session::t_levelNotifyCartoon_SceneSession send;
          send.userID = pAdopter->id;
          send.cartoonID = *it;
          send.level = level;
          pAdopter->scene->sendCmd(&send,sizeof(send));
        }
      }
    }
  }
  /*
  else
  {
    DWORD count = 0;
    for (std::map<DWORD,Cmd::t_CartoonData>::iterator it=cartoonPetList.begin(); it!=cartoonPetList.end(); it++)
    {
      if (count>=5) break;
      if (it->second.masterID==userID)
      {
        it->second.masterLevel = level;
        if (it->second.state==Cmd::CARTOON_STATE_ADOPTED)
        {
          UserSession *pAdopter = UserSessionManager::getInstance()->getUserSessionByName(it->second.adopter);
          if (pAdopter)
          {
            Cmd::Session::t_levelNotifyCartoon_SceneSession send;
            send.userID = pAdopter->id;
            send.cartoonID = it->first;
            send.level = level;
            pAdopter->scene->sendCmd(&send,sizeof(send));
          }
        }
        modifyList.insert(it->first);
        count++;
      }
    }
  }
  */
}

/*
 * \brief ����ʱ,�޸�������Ϊ����������������ݴ���
 *
 */
void CartoonPetService::repairData()
{
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {               
    Zebra::logger->error("[����]repairData: �õ����ݿ���ʧ��");
    return;
  }
  char where[128];
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where)-1,"STATE=%u",Cmd::CARTOON_STATE_ADOPTED);

  const dbCol cartoon_state_define[] = 
  {
    { "STATE",  zDBConnPool::DB_BYTE,sizeof(BYTE) },
    { NULL,0,0}
  };
  BYTE state = Cmd::CARTOON_STATE_WAITING;
  SessionService::dbConnPool->exeUpdate(handle,"`CARTOONPET`",cartoon_state_define,(BYTE*)&state,where);
  SessionService::dbConnPool->putHandle(handle);
}

/*
 * \brief ���ɾ��ʱɾ�����г���
 *
 */
void CartoonPetService::delPetRecordByID(DWORD masterID)
{
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {               
    Zebra::logger->error("[����]delPetRecordByID: �õ����ݿ���ʧ��");
    return;
  }
  char where[128];
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where)-1,"MASTERID=%u",masterID);
  DWORD ret = SessionService::dbConnPool->exeDelete(handle,"`CARTOONPET`",where);
  SessionService::dbConnPool->putHandle(handle);

  if (0==ret) return;

  DWORD count = 0;
  for (hash_map<DWORD,Cmd::t_CartoonData>::iterator it=cartoonPetList.begin(); it!=cartoonPetList.end();)
  {
    hash_map<DWORD,Cmd::t_CartoonData>::iterator temp = it++;
    if (count>=5) break;
    if (temp->second.masterID==masterID)
    {
      if (temp->second.state==Cmd::CARTOON_STATE_WAITING)
      {
        waitingList.erase(temp->first);

        Cmd::stRemoveWaitingCartoonCmd send;
        send.cartoonID = temp->first;
        sendCmdToItsFriendAndFamily(masterID,temp->second.masterName,&send,sizeof(send));
      }
      else
        if (temp->second.state==Cmd::CARTOON_STATE_ADOPTED)
        {
          UserSession *pAdopter = UserSessionManager::getInstance()->getUserSessionByName(temp->second.adopter);
          if (pAdopter)
          {
            Cmd::Session::t_getBackCartoon_SceneSession send;
            send.userID = pAdopter->id;
            send.cartoonID = temp->first;
            pAdopter->scene->sendCmd(&send,sizeof(send));

            pAdopter->sendSysChat(Cmd::INFO_TYPE_FAIL,"�Բ���,������� %s ɾ����",temp->second.masterName);

            adoptedPetMap[temp->second.adopter].erase(temp->first);
          }
        }

      modifyList.erase(temp->first);
      cartoonPetList.erase(temp->first);
      count++;
    }
  }                               
  Zebra::logger->info("[����]���ɾ��,ɾ�����г��� masterID=%u",masterID);
}
/*
 * \brief �����ݿ��������������������
 *
 */
bool CartoonPetService::loadAllFromDB()
{
  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {               
    Zebra::logger->error("[����]loadAllFromDB: �õ����ݿ���ʧ��");
    return false;
  }
  char where[128];
  bzero(where,sizeof(where));

  cartoon_load_struct * dataList,* tempPoint;
  DWORD retcode=SessionService::dbConnPool->exeSelect(handle,"`CARTOONPET`",cartoon_load_define,where,NULL,(BYTE**)&dataList);
  SessionService::dbConnPool->putHandle(handle);
  if (retcode==(DWORD)-1) return false;

  if (dataList)                          
  {
    for (DWORD i=0; i<retcode; i++) 
    {                               
      tempPoint = &dataList[i];

      cartoonPetList[tempPoint->cartoonID] = tempPoint->data;
      if (tempPoint->data.state==Cmd::CARTOON_STATE_WAITING)
        waitingList.insert(tempPoint->cartoonID);
      cartoonPetMap[tempPoint->data.masterID].insert(tempPoint->cartoonID);
    }                               
    SAFE_DELETE_VEC(dataList);

    Zebra::logger->info("[����]���� %u �����������Ϣ�ɹ�",retcode);
  }
  return true;
}

/*
 * \brief ��ʱ��������
 * \param group �Ƿ���鱣��,�������ȫ����
 *
 */
DWORD CartoonPetService::writeAllToDB(bool groupflag)
{
  DWORD ret = 0;
  for (hash_set<DWORD>::iterator it=modifyList.begin(); it!=modifyList.end();)
  {
    DWORD id = *(it++);
    if (!groupflag||id%10==group)
    {
      writeDB(id,cartoonPetList[id]);
      ret++;
    }
  }
  group = (group+1)%10;

#ifdef _DEBUG
  if (ret)
    if (groupflag)
      Zebra::logger->debug("[����]���� %u ����������¼ group=%u",ret,group);
    else
      Zebra::logger->debug("[����]���� %u ����������¼",ret);
#endif
  return ret;
}

void CartoonPetService::userOnline(UserSession * pUser)
{
    if (0==adoptedPetMap[pUser->name].size()) return;

    for (hash_set<DWORD>::iterator it=adoptedPetMap[pUser->name].begin(); it!=adoptedPetMap[pUser->name].end(); it++)
    {
        waitingList.insert(*it);

        cartoonPetList[*it].state = Cmd::CARTOON_STATE_WAITING;
        bzero(cartoonPetList[*it].adopter,MAX_NAMESIZE);

        Cmd::stAddWaitingCartoonCmd add;
        add.cartoonID = *it;
        add = cartoonPetList[*it];
        sendCmdToItsFriendAndFamily(cartoonPetList[*it].masterID,cartoonPetList[*it].masterName,&add,sizeof(add));

        UserSession * pMaster = UserSessionManager::getInstance()->getUserByID(cartoonPetList[*it].masterID);
        if (pMaster)
        {
            Cmd::Session::t_addCartoon_SceneSession ac;
            ac.userID = pMaster->id;
            ac.cartoonID = *it;
            ac.data = cartoonPetList[*it];
            pMaster->scene->sendCmd(&ac,sizeof(ac));
        }
    }
    adoptedPetMap[pUser->name].clear();

    Zebra::logger->error("[����]%s ����,�޸� %u ֻ�����״̬",pUser->name,adoptedPetMap[pUser->name].size());
}
