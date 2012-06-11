#include <zebra/SessionServer.h>
#include <stdarg.h>

const dbCol auction_define[] = {
  { "OWNERID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "OWNER",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
  { "STATE",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "CHARGE",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "ITEMID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "NAME",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
  { "TYPE",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "QUALITY",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "NEEDLEVEL",    zDBConnPool::DB_WORD,sizeof(WORD) },
  { "MINMONEY",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MAXMONEY",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MINGOLD",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MAXGOLD",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "STARTTIME",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "ENDTIME",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "BIDTYPE",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "ITEM",    zDBConnPool::DB_BIN,sizeof(Cmd::Session::SessionObject)},
  { NULL,0,0}           
};

const dbCol auction_bid_define[] = {
  { "ID",      zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "OWNERID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "OWNER",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
  { "STATE",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "CHARGE",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MINMONEY",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MAXMONEY",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MINGOLD",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "MAXGOLD",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "BIDDERID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "BIDDER2ID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "BIDDER",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
  { "BIDDER2",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
  { "ENDTIME",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "BIDTYPE",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { "ITEMID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "ITEM",    zDBConnPool::DB_BIN,sizeof(Cmd::Session::SessionObject)},
  { NULL,0,0}           
};

const dbCol auction_state_define[] = {
  { "STATE",    zDBConnPool::DB_BYTE,sizeof(BYTE) },
  { NULL,0,0}           
};

const dbCol auction_bidder_define[] = {
  { "BIDDERID",    zDBConnPool::DB_DWORD,sizeof(DWORD) },
  { "BIDDER",    zDBConnPool::DB_STR,sizeof(char[MAX_NAMESIZE+1]) },
  { NULL,0,0}           
};

AuctionService *AuctionService::as = 0;

AuctionService::AuctionService(){}
AuctionService::~AuctionService(){}

AuctionService& AuctionService::getMe()
{
  if (!as)
    as = new AuctionService();
  return *as;
}

void AuctionService::delMe()
{
  SAFE_DELETE(as);
}

/* \brief ����������Ϣ
 *
 * \param cmd ��Ϣ
 * \param cmdLen ��Ϣ����
 * 
 */
bool AuctionService::doAuctionCmd(const Cmd::Session::t_AuctionCmd *cmd,const DWORD cmdLen)
{
  using namespace Cmd;
  using namespace Cmd::Session;

  switch (cmd->auctionPara)
  {
    case PARA_AUCTION_SALE:
      {
        t_saleAuction_SceneSession * rev = (t_saleAuction_SceneSession *)cmd;

        UserSession * pUser = UserSessionManager::getInstance()->getUserSessionByName(rev->info.owner);
        /*
        if (!pUser)
        {
          Zebra::logger->error("[����]doAuctionCmd(PARA_AUCTION_SALE): ���������¼ʱδ�ҵ������� %s",rev->info.owner);
          Zebra::logger->error("[����]���������¼ʱδ�ҵ������� %s ��Ʒ: %s",rev->info.owner,rev->item.object.strName);
          return false;
        }
        */

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          error("[����]doAuctionCmd(PARA_AUCTION_SALE): �õ����ݿ���ʧ��");
          Zebra::logger->error("[����]���������¼���� %s ��Ʒ: %s �۸�%u-%u",rev->info.owner,rev->item.object.strName,rev->info.minMoney,rev->info.maxMoney);
          if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"������Ʒʧ��,����GM��ϵ");
          return true;
        }
        DWORD retcode = SessionService::dbConnPool->exeInsert(handle,"`AUCTION`",auction_define,(const BYTE *)&(rev->info));
        SessionService::dbConnPool->putHandle(handle);
        if ((DWORD)-1 == retcode)
        {
          error("[����]doAuctionCmd(PARA_AUCTION_SALE): ����������¼���ݿ����");
          Zebra::logger->error("[����]���������¼���� %s ��Ʒ: %s �۸�%u-%u",rev->info.owner,rev->item.object.strName,rev->info.minMoney,rev->info.maxMoney);
          if (pUser) pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ʧ��,����GM��ϵ");
        }       
        else    
        {       
          if (pUser)
          {
            stAddListAuction al;
            al.list = 3;//�����б�
            al.auctionID = retcode;//����ID
            al.minMoney = rev->info.minMoney;
            al.maxMoney = rev->info.maxMoney;
            al.minGold = rev->info.minGold;
            al.maxGold = rev->info.maxGold;
            al.endTime = rev->info.endTime - rev->info.startTime;
            al.bidType = rev->info.bidType;
            bcopy(&rev->item.object,&al.item,sizeof(t_Object),sizeof(al.item));
            strncpy(al.owner,rev->info.owner,MAX_NAMESIZE);
            al.mine = true;

            pUser->sendCmdToMe(&al,sizeof(al));
            pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"������һ����Ʒ");
          }

          Zebra::logger->error("[����]:%s ������Ʒ %s auctionID=%u",rev->info.owner,rev->item.object.strName,retcode);
        }

        return true;
      }
      break;
    case PARA_AUCTION_CHECK_BID:
      {
        t_checkBidAuction_SceneSession * rev = (t_checkBidAuction_SceneSession *)cmd;
        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (!pUser)
        {
          Zebra::logger->error("[����]doAuctionCmd(PARA_AUCTION_CHECK_BID): ����ʱδ�ҵ�������");
          return false;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          error("[����]doAuctionCmd(PARA_AUCTION_CHECK_BID): �õ����ݿ���ʧ��");
          return true;
        }

        char where[128];
        bzero(where,sizeof(where));
        zRTime ct;
        //_snprintf(where,sizeof(where)-1,"ID=%u AND STATE=%u AND ENDTIME>%lu",rev->auctionID,AUCTION_STATE_NEW,ct.sec());
        _snprintf(where,sizeof(where)-1,"ID=%u AND STATE=%u",rev->auctionID,AUCTION_STATE_NEW);

        auctionBidInfo bid;
        DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`AUCTION`",auction_bid_define,where,NULL,1,(BYTE*)&bid);
        SessionService::dbConnPool->putHandle(handle);
        if ((DWORD)-1 == retcode)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ʧ��");
          error("[����]%s ����ʱ��ѯ���� auctionID=%u retCode=%d",pUser->name,rev->auctionID,retcode);
          return false;
        }
        if (1 != retcode)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ʧ��,����Ʒ�Ѿ��ɽ�");
          //Zebra::logger->warn("[����]%s ����û���ҵ���¼ auctionID=%u retCode=%d",pUser->name,rev->auctionID,retcode);
          return true;
        }

        if (0==strncmp(bid.bidder,pUser->name,MAX_NAMESIZE))
        {
          if ((0==bid.bidType&&((bid.maxMoney&&rev->money<bid.maxMoney)||0==bid.maxMoney))
              || (1==bid.bidType&&(bid.maxGold&&rev->gold<bid.maxGold||0==bid.maxGold)))
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�����Ʒ����߼��Ѿ�����ġ���������Ʒ�趨��һ�ڼ�,�����һ�ڼ۰���������");
            return true;
          }
        }

        if ((!strncmp(bid.owner,pUser->name,MAX_NAMESIZE))
            || (0==bid.bidType && bid.minMoney>rev->money)
            || (0==bid.bidType && (bid.minMoney==rev->money && (bid.maxMoney!=bid.minMoney)))
            || (1==bid.bidType && bid.minGold>rev->gold)
            || (1==bid.bidType && (bid.minGold==rev->gold && (bid.maxGold!=bid.minGold))))
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ִ�иò���,�Ƿ�����");
          return false;
        }

        rev->bidType = bid.bidType;
        pUser->scene->sendCmd(rev,sizeof(t_checkBidAuction_SceneSession));
      }
      break;
    case PARA_AUCTION_BID:
      {
        t_bidAuction_SceneSession * rev = (t_bidAuction_SceneSession *)cmd;

        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (!pUser)
        {
          Zebra::logger->error("[����]doAuctionCmd(PARA_AUCTION_CHECK_BID): ����ʱδ�ҵ�������,��ʧ%u��",rev->money);
          return false;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          error("[����]doAuctionCmd(PARA_AUCTION_CHECK_BID): �õ����ݿ���ʧ��",pUser->name);
          Zebra::logger->error("[����]����ʧ�� %s ��ʧ money=%u gold=%u",rev->money,rev->gold);
          return false;
        }

        char where[128];
        bzero(where,sizeof(where));
        zRTime ct;
        _snprintf(where,sizeof(where) - 1,"ID=%u AND STATE=%u",rev->auctionID,AUCTION_STATE_NEW);

        auctionBidInfo bid;
        DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`AUCTION`",auction_bid_define,where,NULL,1,(BYTE*)&bid);

        if ((DWORD)-1 == retcode)
        {
          SessionService::dbConnPool->putHandle(handle);
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ʧ��");
          error("[����]%s ����ʱ��ѯ���� auctionID=%u retCode=%d ��ʧ money=%u gold=%u",pUser->name,rev->auctionID,retcode,rev->money,rev->gold);
          return false;
        }
        if (1 != retcode)
        {
          if (MailService::getMe().sendMoneyMail("��;������",0,pUser->name,pUser->id,rev->money,"����ʧ�ܷ���������",(DWORD)handle,MAIL_TYPE_AUCTION,bid.itemID))
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ʧ��,����Ʒ�Ѿ��ɽ�,�������˻ص�����");
          else
            Zebra::logger->error("[����]%s ����ʧ�ܷ�������ʧ�� auctionID=%u retCode=%d ��ʧ money=%u gold=%u",pUser->name,rev->auctionID,retcode,rev->money,rev->gold);
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }

        //��Ǯ����һ��������
        if (0!=strcmp(bid.bidder,"") || bid.bidderID!=0)
        {
          char buf[128];
          bzero(buf,sizeof(buf));
          _snprintf(buf,sizeof(buf),"�㾺�� %s �ļ۸�ѹ��,�����Ľ�Ǯ",bid.item.object.strName);
          MailService::getMe().sendMoneyMail("��;������",0,bid.bidder,bid.bidderID,bid.minMoney,buf,(DWORD)handle,MAIL_TYPE_AUCTION,bid.itemID);
        }

        char bidder3[MAX_NAMESIZE];
        strncpy(bidder3,bid.bidder2,MAX_NAMESIZE);

        //���¼۸�;�����
        bid.minMoney = 0==bid.bidType?rev->money:0;
        bid.minGold = 1==bid.bidType?rev->gold:0;
        strncpy(bid.bidder2,bid.bidder,MAX_NAMESIZE);
        bid.bidder2ID = bid.bidderID;
        strncpy(bid.bidder,pUser->name,MAX_NAMESIZE);
        bid.bidderID = pUser->id;
        retcode = SessionService::dbConnPool->exeUpdate(handle,"`AUCTION`",auction_bid_define,(BYTE*)&bid,where);
        if (1 != retcode)
        {
          if (MailService::getMe().sendMoneyMail("��;������",0,pUser->name,pUser->id,rev->money,"����ʧ�ܷ����Ľ�Ǯ",(DWORD)handle,MAIL_TYPE_AUCTION,bid.itemID))
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ʧ��,����Ʒ�Ѿ��ɽ�,�������˻ص�����");
          else
          {
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����ʧ��");
            Zebra::logger->error("[����]%s ����ʱû�ҵ���¼,�˻��ʼ�ʧ�� auctionID=%u retCode=%d,��ʧ money=%u gold=%u",pUser->name,rev->auctionID,retcode,rev->money,rev->gold);
            SessionService::dbConnPool->putHandle(handle);
            return false;
          }
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }

        //���۴���һ�ڼ�
        if ((0==bid.bidType && bid.maxMoney && rev->money>=bid.maxMoney)
            || (1==bid.bidType && bid.maxGold && rev->gold>=bid.maxGold))
        {
          if (!sendAuctionItem((DWORD)handle,rev->auctionID,AUCTION_STATE_DEAL,false))
          {
            Zebra::logger->error("[����]%s ���۳ɹ�������Ʒʧ�� auctionID=%u",pUser->name,rev->auctionID);
            SessionService::dbConnPool->putHandle(handle);
            return false;
          }
        }
        else
        {
          //֪ͨ������ˢ�¼�¼
          stAddListAuction al;
          al.auctionID = bid.auctionID;
          al.minMoney = bid.minMoney;
          al.maxMoney = bid.maxMoney;
          al.minGold = bid.minGold;
          al.maxGold = bid.maxGold;
          zRTime ct;
          al.endTime = bid.endTime>ct.sec()?bid.endTime-ct.sec():0;
          al.bidType = bid.bidType;
          bcopy(&bid.item.object,&al.item,sizeof(t_Object),sizeof(al.item));
          strncpy(al.owner,bid.owner,MAX_NAMESIZE);

          //al.list = 1;
          al.mine = true;
          //pUser->sendCmdToMe(&al,sizeof(al));
          al.list = 2;
          pUser->sendCmdToMe(&al,sizeof(al));

          al.list = 2;//�����б�
          al.mine = false;
          UserSession * u = 0;
          if (strcmp(bidder3,pUser->name))
          {
            u = UserSessionManager::getInstance()->getUserSessionByName(bidder3);
            if (u)
              u->sendCmdToMe(&al,sizeof(al));
          }
          if (0!=strcmp(bid.bidder2,""))
          {
            u = UserSessionManager::getInstance()->getUserSessionByName(bid.bidder2);
            if (u)
              u->sendCmdToMe(&al,sizeof(al));
          }

          al.list = 3;//�����ߵ������б�
          u = UserSessionManager::getInstance()->getUserSessionByName(bid.owner);
          if (u)
            u->sendCmdToMe(&al,sizeof(al));
        }

        SessionService::dbConnPool->putHandle(handle);

        return true;
      }
      break;
    case PARA_AUCTION_QUERY:
      {
        t_queryAuction_SceneSession * rev = (t_queryAuction_SceneSession *)cmd;

        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (!pUser)  return true;

        if (0==strcmp(rev->name,"") && 0==rev->type && 0==rev->quality)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"����̫��,������������������С������Χ");
          return true;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          error("[����]doAuctionCmd(PARA_AUCTION_QUERY): �õ����ݿ���ʧ��");
          return false;
        }

        std::string where;
        char tmp[128];
        bzero(tmp,sizeof(tmp));
        _snprintf(tmp,sizeof(tmp),"STATE = %d",AUCTION_STATE_NEW);
        where = tmp;
        bzero(tmp,sizeof(tmp));
        if (rev->type)
        {
          _snprintf(tmp,sizeof(tmp)," AND TYPE=%u ",rev->type);
          where += tmp;
          bzero(tmp,sizeof(tmp));
        }
        if (rev->quality)
        {
          _snprintf(tmp,sizeof(tmp)," AND QUALITY=%u ",rev->quality-1);
          where += tmp;
          bzero(tmp,sizeof(tmp));
        }
        if (rev->level!=(WORD)-1)
        {
          _snprintf(tmp,sizeof(tmp)," AND NEEDLEVEL=%u ",rev->level);
          where += tmp;
          bzero(tmp,sizeof(tmp));
        }
        if (strcmp(rev->name,""))
        {
          std::string escapeName;
          SessionService::dbConnPool->escapeString(handle,rev->name,escapeName);
          _snprintf(tmp,sizeof(tmp)," AND NAME LIKE '%%%s%%' ",escapeName.c_str());
          where += tmp;
          bzero(tmp,sizeof(tmp));
        }

        auctionBidInfo bid[10];
        DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`AUCTION`",auction_bid_define,where.c_str(),NULL,10,(BYTE*)&bid,rev->page*10);

        static const dbCol count_define[] = {
          { "`RET`",zDBConnPool::DB_DWORD,sizeof(DWORD) },
          { NULL,0,0}
        };
        char sql[1024];
        DWORD count;
        _snprintf(sql,1024,"SELECT COUNT(*) AS `RET` FROM `AUCTION` WHERE %s",where.c_str());
        SessionService::dbConnPool->execSelectSql(handle,sql,strlen(sql),count_define,1,(BYTE*)&count);
        SessionService::dbConnPool->putHandle(handle);

        if ((DWORD)-1 == retcode)
        {
          Zebra::logger->error("[����]%s ��ѯ������Ʒʱ���� retCode=%d",pUser->name,retcode);
          return false;
        }

        if (retcode)
        {
          stAddListAuction al;
          al.list = 1;
          al.max = count;
          zRTime ct;
          for (DWORD i=0; i<retcode && i<10; i++)
          {
            al.auctionID = bid[i].auctionID;
            al.minMoney = bid[i].minMoney;
            al.maxMoney = bid[i].maxMoney;
            al.minGold = bid[i].minGold;
            al.maxGold = bid[i].maxGold;
            al.endTime = bid[i].endTime>ct.sec()?bid[i].endTime-ct.sec():0;
            al.bidType = bid[i].bidType;
            bcopy(&bid[i].item.object,&al.item,sizeof(t_Object),sizeof(al.item));
            strncpy(al.owner,bid[i].owner,MAX_NAMESIZE);
            al.mine = (0==strcmp(bid[i].bidder,pUser->name));

            pUser->sendCmdToMe(&al,sizeof(al));
#ifdef _DEBUG
        Zebra::logger->debug("[����]��ѯ�� %u ����¼ %s",count,where.c_str());
#endif
          }
        }
      }
      break;
    case PARA_AUCTION_CHECK_CANCEL:
      {
        t_checkCancelAuction_SceneSession * rev = (t_checkCancelAuction_SceneSession *)cmd;

#ifdef _DEBUG
        //zThread::msleep(3000);
#endif

        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (!pUser)
        {
          Zebra::logger->error("[����]doAuctionCmd(PARA_AUCTION_CHECK_CANCEL): ȡ���������ʱδ�ҵ�������");
          return false;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          error("[����]doAuctionCmd(PARA_AUCTION_CHECK_CANCEL): �õ����ݿ���ʧ��");
          return false;
        }

        char where[128];
        bzero(where,sizeof(where));
        zRTime ct;
        std::string escapeName;
        _snprintf(where,sizeof(where) - 1,"ID=%u AND STATE=%u AND OWNER='%s'",rev->auctionID,AUCTION_STATE_NEW,SessionService::dbConnPool->escapeString(handle,pUser->name,escapeName).c_str());

        auctionBidInfo bid;
        DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`AUCTION`",auction_bid_define,where,NULL,1,(BYTE*)&bid);
        SessionService::dbConnPool->putHandle(handle);
        if ((DWORD)-1 == retcode)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"ȡ������ʧ��");
          error("[����]%s ȡ���������ʱ��ѯ���� auctionID=%u retCode=%d",pUser->name,rev->auctionID,retcode);
          return false;
        }
        if (1 != retcode)
        {
          pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�޷�ȡ������,��Ʒ�Ѿ��ɽ������");
          return true;
        }

        t_checkCancelAuction_SceneSession cca;
        cca.userID = rev->userID;
        cca.auctionID = rev->auctionID;
        cca.charge = bid.charge;

        pUser->scene->sendCmd(&cca,sizeof(cca));
      }
      break;
    case PARA_AUCTION_CANCEL:
      {
        t_cancelAuction_SceneSession * rev = (t_cancelAuction_SceneSession *)cmd;

        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (!pUser)
        {
          Zebra::logger->error("[����]doAuctionCmd(PARA_AUCTION_CANCEL): ȡ������ʱδ�ҵ�������");
          return false;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          error("doAuctionCmd(PARA_AUCTION_CANCEL): �õ����ݿ���ʧ��");
          return true;
        }

        char where[128];
        bzero(where,sizeof(where));
        zRTime ct;
        _snprintf(where,sizeof(where) - 1,"ID=%u",rev->auctionID);

        auctionBidInfo bid;
        DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`AUCTION`",auction_bid_define,where,NULL,1,(BYTE*)&bid);
        if (1 != retcode
            || bid.state!=AUCTION_STATE_NEW)
        {
          if (MailService::getMe().sendMoneyMail("��;������",0,pUser->name,pUser->id,rev->charge,"ȡ������ʧ�ܷ����ķ���",(DWORD)handle,MAIL_TYPE_AUCTION,bid.itemID))
            pUser->sendSysChat(Cmd::INFO_TYPE_FAIL,"�޷�ȡ������,��Ʒ�Ѿ��ɽ������,�����ѷ���������");
          else
            Zebra::logger->error("%s ȡ������ʱû�ҵ���¼,�˻�����ʧ�� auctionID=%u retCode=%d charge=%u",pUser->name,rev->auctionID,retcode,rev->charge);
          SessionService::dbConnPool->putHandle(handle);
          return true;
        }

        if (!sendAuctionItem((DWORD)handle,rev->auctionID,AUCTION_STATE_CANCEL,true))
        {
          Zebra::logger->error("%s ȡ������������Ʒʧ�� auctionID=%u charge=%u itemID=%u",pUser->name,rev->auctionID,rev->charge,bid.itemID);
          SessionService::dbConnPool->putHandle(handle);
          return false;
        }
        SessionService::dbConnPool->putHandle(handle);

        std::ostringstream s;
        s<<"�㸶��";
        if (rev->charge/10000) s<<rev->charge/10000<<"��";
        if ((rev->charge%10000)/100) s<<(rev->charge%10000)/100<<"��";
        if (rev->charge%100) s<<rev->charge%100<<"��";
        pUser->sendSysChat(Cmd::INFO_TYPE_GAME,"�㸶��%s,ȡ���� %s ������,���Ҵ�������ȡ��Ʒ",s.str().c_str(),bid.item.object.strName);
        Zebra::logger->info("[����]%s ȡ������ %s auctionID=%u itemID=%u",pUser->name,bid.item.object.strName,rev->auctionID,bid.itemID);
      }
      break;
    case PARA_AUCTION_GET_LIST:
      {
        t_getListAuction_SceneSession * rev = (t_getListAuction_SceneSession *)cmd;

        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (!pUser)
        {
          Zebra::logger->error("doAuctionCmd(PARA_AUCTION_GET_LIST): ��ѯʱδ�ҵ����");
          return false;
        }

        connHandleID handle = SessionService::dbConnPool->getHandle();
        if ((connHandleID)-1 == handle)
        {
          error("doAuctionCmd(PARA_AUCTION_GET_LIST): �õ����ݿ���ʧ��");
          return false;
        }

        zRTime ct;
        std::string escapeName;
        SessionService::dbConnPool->escapeString(handle,pUser->name,escapeName);
        char where[128];
        bzero(where,sizeof(where));
        if (rev->list == 3)//�����б�
          _snprintf(where,sizeof(where)-1,"STATE=%u AND OWNER='%s'",AUCTION_STATE_NEW,escapeName.c_str());
        else
          _snprintf(where,sizeof(where)-1,"STATE=%u AND (BIDDER='%s' OR BIDDER2='%s')",AUCTION_STATE_NEW,escapeName.c_str(),escapeName.c_str());

        auctionBidInfo *bid = 0,*tempPoint = 0;
        DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`AUCTION`",auction_bid_define,where,NULL,(BYTE **)&bid);
        SessionService::dbConnPool->putHandle(handle);

        if ((DWORD)-1 == retcode)
        {
          error("%s ��ѯ������Ʒʱ���� retCode=%d",pUser->name,retcode);
          return false;
        }

        if (bid)
        {
          stAddListAuction al;
          al.list = rev->list;
          tempPoint = &bid[0];
          for (DWORD i=0; i<retcode; i++)
          {
            al.auctionID = tempPoint->auctionID;
            al.minMoney = tempPoint->minMoney;
            al.maxMoney = tempPoint->maxMoney;
            al.minGold = tempPoint->minGold;
            al.maxGold = tempPoint->maxGold;
            al.endTime = tempPoint->endTime>ct.sec()?tempPoint->endTime-ct.sec():0;
            al.bidType = tempPoint->bidType;
            bcopy(&tempPoint->item.object,&al.item,sizeof(t_Object),sizeof(al.item));
            strncpy(al.owner,tempPoint->owner,MAX_NAMESIZE);
            if (0==strcmp(tempPoint->bidder,pUser->name))
              al.mine = true;
            else
              al.mine = false;

            pUser->sendCmdToMe(&al,sizeof(al));

            tempPoint++;
          }
          SAFE_DELETE_VEC(bid);
        }
      }
      break;
      /*
    case PARA_AUCTION_BID_LIST:
      {
        t_bidListAuction_SceneSession * rev = (t_bidListAuction_SceneSession *)cmd;

        UserSession * pUser = UserSessionManager::getInstance()->getUserByTempID(rev->userID);
        if (!pUser) return false;

        bcopy(&rev->bidList[0],&pUser->bidList[0],sizeof(pUser->bidList));
      }
      break;
      */
    default:
      break;
  }
  return false;
}

/* \brief ��ʱ������ݿ�
 * 
 * �˻ء�ɾ�����ڵ��ʼ�
 * 
 */
void AuctionService::checkDB()
{
  using namespace Cmd::Session;

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {               
    error("[����]checkDB: �õ����ݿ���ʧ��");
    return;
  }

  zRTime ct;
  char where[128];

  //��������ʱ�������
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where),"STATE=%u AND ENDTIME<%lu",AUCTION_STATE_NEW,ct.sec());

  auctionBidInfo * bidList,* tempPoint;
  DWORD retcode = SessionService::dbConnPool->exeSelect(handle,"`AUCTION`",auction_bid_define,where,NULL,(BYTE **)&bidList);
  if ((DWORD)-1 == retcode)
  {
    error("[����]checkDB: ������ʧ�� retCode=%d",retcode);
    SessionService::dbConnPool->putHandle(handle);
    return;
  }

  if (bidList)
  {
    tempPoint = &bidList[0];
    for (DWORD i=0; i< retcode; i++)
    {
      bool toOnwer = true;
      if (0!=strcmp(tempPoint->bidder,""))
        toOnwer = false;
      if (sendAuctionItem((DWORD)handle,tempPoint->auctionID,AUCTION_STATE_TIMEOVER,toOnwer))
        Zebra::logger->info("[����]����ʱ�����,%s��Ʒ %s auctionID=%u itemID=%u",toOnwer?"�˻�":"����",tempPoint->item.object.strName,tempPoint->auctionID,tempPoint->itemID);
      else
        Zebra::logger->error("[����]ʱ������˻���Ʒʧ�� auctionID=%u itemID=%u",tempPoint->auctionID,tempPoint->itemID);
      tempPoint++;
    }
    SAFE_DELETE_VEC(bidList);
  }

  //ɾ�����ڵ�
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where),"ENDTIME<%lu",ct.sec()-864000);
  retcode = SessionService::dbConnPool->exeDelete(handle,"`AUCTION`",where);
  if (retcode) Zebra::logger->debug("[����]ɾ�� %u ��10��֮ǰ��������¼",retcode);

  SessionService::dbConnPool->putHandle(handle);
}

/* \brief ������������Ʒ
 * ��������ġ����ġ��˻ص�,ͬʱ����Ǯ
 * 
 * \param h ���ݿ���
 * \param auctionID ������¼��ID
 * \param newState ����֮��Ҫ���õ�״̬
 * \param toOwner �Ƿ��͸���Ʒ������
 * 
 */
bool AuctionService::sendAuctionItem(DWORD h,DWORD auctionID,BYTE newState,bool toOwner)
{
  using namespace Cmd::Session;

  connHandleID handle = (connHandleID)h;

  if ((connHandleID)-1 == handle)
  {               
    error("[����]sendAuctionItem: ��Ч�����ݿ���");
    return false;
  }

  char where[128];
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"ID=%u",auctionID);

  auctionBidInfo bid;
  DWORD retcode = SessionService::dbConnPool->exeSelectLimit(handle,"`AUCTION`",auction_bid_define,where,NULL,1,(BYTE*)&bid);
  if ((DWORD)-1 == retcode)
  {
    error("[����]sendAuctionItem ���ݿ��ѯ���� auctionID=%u retcode=%d",auctionID,retcode);
    return false;
  }
  if (1 != retcode)
    return false;

  t_sendMail_SceneSession sm;
  sm.mail.state = MAIL_STATE_NEW;
  strncpy(sm.mail.fromName,"��;������",MAX_NAMESIZE);
  sm.mail.type = MAIL_TYPE_AUCTION;
  zRTime ct;
  sm.mail.createTime = ct.sec();
  sm.mail.delTime = sm.mail.createTime + 60*60*24*7;
  sm.mail.accessory = 1;
  sm.mail.itemGot = 0;
  sm.mail.sendMoney = 0;
  sm.mail.recvMoney = 0;
  sm.mail.itemID = bid.itemID;

  char buf[128];
  bzero(buf,sizeof(buf));
  Cmd::stRemoveListAuction rl;
  rl.auctionID = auctionID;
  if (!toOwner)//����,��������
  {
    if (strcmp(bid.bidder,""))
    {
      strncpy(sm.mail.toName,bid.bidder,MAX_NAMESIZE);
      sm.mail.toID = bid.bidderID;
      sm.mail.itemID = bid.itemID;
      _snprintf(buf,sizeof(buf),"����������Ʒ %s �ɹ��۳�,��õ����ӡ�ϵͳ��ȡ1%%�ĳɽ�����ΪӶ��",bid.item.object.strName);
      MailService::getMe().sendMoneyMail("��;������",0,bid.owner,bid.ownerID,bid.minMoney*99/100,buf,(DWORD)handle,MAIL_TYPE_AUCTION,bid.itemID);//��������Ǯ

      UserSession * u = UserSessionManager::getInstance()->getUserSessionByName(bid.owner);
      if (u)
      {
        u->sendSysChat(Cmd::INFO_TYPE_GAME,"��Ʒ %s �����ɹ�,��ע������ʼ�",bid.item.object.strName);
        rl.list = 4;
        u->sendCmdToMe(&rl,sizeof(rl));
      }
      u = UserSessionManager::getInstance()->getUserSessionByName(bid.bidder);
      if (u)
      {
        u->sendSysChat(Cmd::INFO_TYPE_GAME,"��Ʒ %s ���ĳɹ�,��ע������ʼ�",bid.item.object.strName);
        rl.list = 4;
        u->sendCmdToMe(&rl,sizeof(rl));
      }

      _snprintf(sm.mail.text,sizeof(sm.mail.text),"���� %s �ɹ�",bid.item.object.strName);
      strncpy(sm.mail.title,"���ĳɹ�",sizeof(sm.mail.title));
      Zebra::logger->info("[����]%s Ӯ����Ʒ %s auctionID=%u itemID=%u",bid.bidder,bid.item.object.strName,auctionID,bid.itemID);
    }
    else
    {
      Zebra::logger->error("[����]sendAuctionItem: �����ߵ�����Ϊ�� auctionID=%u,itemID=%u",auctionID,bid.itemID);
      return false;
    }
  }
  else
  {
    strncpy(sm.mail.toName,bid.owner,MAX_NAMESIZE);
    sm.mail.toID = bid.ownerID;
    if (newState==AUCTION_STATE_CANCEL)
    {
      strncpy(sm.mail.title,"ȡ������",sizeof(sm.mail.title));
      _snprintf(sm.mail.text,sizeof(sm.mail.text),"��ȡ������ %s",bid.item.object.strName);
    }
    else
    {
      strncpy(sm.mail.title,"��Ʒ���˻�",sizeof(sm.mail.title));
      _snprintf(sm.mail.text,sizeof(sm.mail.text),"�����з�������� %s",bid.item.object.strName);
    }

    UserSession * u = UserSessionManager::getInstance()->getUserSessionByName(bid.owner);
    if (u)
    {
      u->sendSysChat(Cmd::INFO_TYPE_GAME,"�����Ʒ %s ��������,��ע������ʼ�",bid.item.object.strName);
      rl.list = 4;
      u->sendCmdToMe(&rl,sizeof(rl));
    }
    if (strcmp(bid.bidder,""))
    {
      _snprintf(buf,sizeof(buf),"�㾺����Ʒ %s ʧ��,�˻ص�����",bid.item.object.strName);
      MailService::getMe().sendMoneyMail("��;������",0,bid.bidder,bid.bidderID,bid.minMoney,buf,(DWORD)handle,MAIL_TYPE_AUCTION,bid.itemID);//��������Ǯ
      u = UserSessionManager::getInstance()->getUserSessionByName(bid.bidder);
      if (u)
      {
        rl.list = 4;
        u->sendCmdToMe(&rl,sizeof(rl));
      }
    }
    Zebra::logger->info("[����]%s ȡ����Ʒ %s auctionID=%u itemID=%u",bid.owner,bid.item.object.strName,auctionID,bid.itemID);
  }
  bcopy(&bid.item,&sm.item,sizeof(SessionObject),sizeof(sm.item));

  if (MailService::getMe().sendMail((DWORD)handle,sm))
  {
    bid.state = newState;
    retcode = SessionService::dbConnPool->exeUpdate(handle,"`AUCTION`",auction_bid_define,(BYTE*)&bid,where);
    if (retcode!=1)
    {
      error("[����]sendAuctionItem: ��������״̬ʧ�� auctionID=%u itemID=%u",auctionID,bid.itemID);
      return false;
    }
    return true;
  }
  else
  {
    Zebra::logger->error("[����]sendAuctionItem: ��������������Ʒʧ�� auctionID=%u itemID=%u",auctionID,bid.itemID);
    return false;
  }
}

/* \brief �����������ɾ��������������¼
 * �������������ĺͳ��ۼ�¼ȫ��ɾ��
 * 
 * \param name ��ɫ����
 * 
 */
void AuctionService::delAuctionRecordByName(char * name)
{
  using namespace Cmd::Session;

  connHandleID handle = SessionService::dbConnPool->getHandle();
  if ((connHandleID)-1 == handle)
  {
    error("[����]delAuctionRecordByName: �õ����ݿ���ʧ�� name=%s",name);
    return;
  }

  std::string escapeName;
  SessionService::dbConnPool->escapeString(handle,name,escapeName);
  char where[128];
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"OWNER='%s' AND STATE=%u",escapeName.c_str(),AUCTION_STATE_NEW);

  BYTE state = AUCTION_STATE_DEL;
  DWORD retcode = SessionService::dbConnPool->exeUpdate(handle,"`AUCTION`",auction_state_define,&state,where);

  char b[sizeof(DWORD) + MAX_NAMESIZE + 1];
  bzero(b,sizeof(b));
  bzero(where,sizeof(where));
  _snprintf(where,sizeof(where) - 1,"BIDDER='%s' AND STATE=%u",escapeName.c_str(),AUCTION_STATE_NEW);
  retcode = SessionService::dbConnPool->exeUpdate(handle,"`AUCTION`",auction_bidder_define,(BYTE*)b,where);

  SessionService::dbConnPool->putHandle(handle);

  Zebra::logger->info("[����]ɾ����ɫ����������¼��name=%s",name);
}

#define getMessage(msg,msglen,pat)  \
  do  \
{  \
  va_list ap;  \
  bzero(msg,msglen);  \
  va_start(ap,pat);    \
  vsnprintf(msg,msglen - 1,pat,ap);  \
  va_end(ap);  \
}while(false)

bool AuctionService::error(const char * msg,...)
{
  char buf[MAX_CHATINFO];
  bzero(buf,sizeof(buf));
  getMessage(buf,MAX_CHATINFO,msg);

  Zebra::logger->error("[����]%s",buf);
  SessionService::getInstance().reportGm("[����]",buf);

  return true;
}
