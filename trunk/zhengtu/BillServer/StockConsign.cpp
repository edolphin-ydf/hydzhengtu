#include "BillServer.h"

bool Consign::sendWaitDataToUser(BillUser *pUser)
{
  if (pUser)
  {
    static char Buf[sizeof(Cmd::stConsignGoldListStockUserCmd) + 720 * 60 * sizeof(Cmd::StockList)];
    bzero(Buf,sizeof(Buf));
    connHandleID handle = BillService::dbConnPool->getHandle();

    if ((connHandleID)-1 != handle)
    {
      DBFieldSet* history = BillService::metaData->getFields("CONSIGNGOLD");
      DBRecordSet* recordset = NULL;
      std::ostringstream oss;         
      DBRecord column,order,group,where;                           
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
      if (history)
      {
        Cmd::stConsignGoldListStockUserCmd *waitgold = (Cmd::stConsignGoldListStockUserCmd*)Buf;
        bzero(waitgold,sizeof(Cmd::stConsignGoldListStockUserCmd));
        constructInPlace(waitgold);
        oss << "accid = " << pUser->id;
        where.put("accid",oss.str());
        recordset = BillService::dbConnPool->exeSelect(handle,history,NULL,&where,NULL,0,NULL);
        if (recordset && !recordset->empty())
        {
          for(DWORD i=0; i < recordset->size(); i++)
          {
            waitgold->list[waitgold->size].id = recordset->get(i)->get("id");
            waitgold->list[waitgold->size].dwNum = recordset->get(i)->get("num");
            waitgold->list[waitgold->size].dwPrice = recordset->get(i)->get("price");
            waitgold->list[waitgold->size].dwTime = recordset->get(i)->get("time");
            //Zebra::logger->debug("%s(%d)δ�ɽ��ĸ��˽��ί�е�:%d,%d,%d,%d",pUser->name,pUser->id,waitgold->list[waitgold->size].id,waitgold->list[waitgold->size].dwNum,waitgold->list[waitgold->size].dwPrice,waitgold->list[waitgold->size].dwTime);
            waitgold->size++;
          }
          if (waitgold->size)
          {
            pUser->sendCmdToMe(waitgold,sizeof(Cmd::stConsignGoldListStockUserCmd)+sizeof(Cmd::StockList)*waitgold->size);
          }
        }
      }
      SAFE_DELETE(recordset);
      history = BillService::metaData->getFields("CONSIGNMONEY");
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
      if (history)
      {
        Cmd::stConsignMoneyListStockUserCmd *waitmoney = (Cmd::stConsignMoneyListStockUserCmd*)Buf;
        bzero(waitmoney,sizeof(Cmd::stConsignMoneyListStockUserCmd));
        constructInPlace(waitmoney);
        oss.str("");
        oss << "accid = " << pUser->id;
        where.clear();
        where.put("accid",oss.str());
        recordset = BillService::dbConnPool->exeSelect(handle,history,NULL,&where,NULL,0,NULL);
        if (recordset && !recordset->empty())
        {
          for(DWORD i=0; i < recordset->size(); i++)
          {
            waitmoney->list[waitmoney->size].id = recordset->get(i)->get("id");
            waitmoney->list[waitmoney->size].dwNum = recordset->get(i)->get("num");
            waitmoney->list[waitmoney->size].dwPrice = recordset->get(i)->get("price");
            waitmoney->list[waitmoney->size].dwTime = recordset->get(i)->get("time");
            //Zebra::logger->debug("%s(%d)δ�ɽ��ĸ�������ί�е�:%d,%d,%d,%d",pUser->name,pUser->id,waitmoney->list[waitmoney->size].id,waitmoney->list[waitmoney->size].dwNum,waitmoney->list[waitmoney->size].dwPrice,waitmoney->list[waitmoney->size].dwTime);
            waitmoney->size++;
          }
          if (waitmoney->size)
          {
            pUser->sendCmdToMe(waitmoney,sizeof(Cmd::stConsignMoneyListStockUserCmd)+sizeof(Cmd::StockList)*waitmoney->size);
          }
        }
      }
      SAFE_DELETE(recordset);
      BillService::dbConnPool->putHandle(handle);
      return true;
    }
  }
  return false;
}

bool Consign::sendFirstFiveToUser(BillUser *pUser)
{
    if (pUser)
    {
      return pUser->sendCmdToMe(firstfive,sizeof(Cmd::stFirstFiveListStockUserCmd)+sizeof(Cmd::FirstTen)*firstfive->size);
    }
    return false;
}

bool Consign::updateHistory(DWORD id,DWORD acc,DWORD num,DWORD commitprice,DWORD price,DWORD comtime,bool type,DWORD sysmoney)
{
  connHandleID handle = BillService::dbConnPool->getHandle();

  if ((connHandleID)-1 != handle)
  {
    DBRecord column;
    DBFieldSet* historytbl = NULL;
    if (type == Cmd::STOCK_GOLD)
    {
      historytbl = BillService::metaData->getFields("CONSIGNGOLDHISTORY");
    }
    else if (type == Cmd::STOCK_MONEY)
    {
      historytbl = BillService::metaData->getFields("CONSIGNMONEYHISTORY");
    }
    column.clear();
    column.put("id",id);
    column.put("accid",acc);
    column.put("num",num);
    column.put("commitprice",commitprice);
    column.put("price",price);
    column.put("committime",comtime);
    column.put("oktime",BillUser::getRealMinTime());
    column.put("sysmoney",sysmoney);
    if ((DWORD)-1 == BillService::dbConnPool->exeInsert(handle,historytbl,&column))
    {
      Zebra::logger->debug("Consign::updateHistory���ݿ����!");
      BillService::dbConnPool->putHandle(handle);
      return false;
    }      
    else
    {
      if (type == Cmd::STOCK_GOLD)
      {
      BillUser::logger("��Ʊ�����ʷ",id,NULL,price,num,acc,NULL);
      }
      else if (type == Cmd::STOCK_MONEY)
      {
      BillUser::logger("��Ʊ������ʷ",id,NULL,price,num,acc,NULL);
      }
    }
    BillService::dbConnPool->putHandle(handle);
    return true;
  }
  return false;
}
ConsignGoldManager *ConsignGoldManager::instance=NULL;
ConsignGoldManager::ConsignGoldManager()
{
  firstfive->byType=Cmd::STOCK_GOLD;
}
ConsignGoldManager::~ConsignGoldManager()
{
}
ConsignGoldManager *ConsignGoldManager::getInstance()
{
  if (instance == NULL)
    instance = new ConsignGoldManager();
  return instance;
}
void ConsignGoldManager::delInstance()
{
  SAFE_DELETE(instance);
}
bool ConsignGoldManager::init()
{
  connHandleID handle = BillService::dbConnPool->getHandle();

  if ((connHandleID)-1 != handle)
  {
    DBFieldSet* goldtbl = BillService::metaData->getFields("CONSIGNGOLD");
    DBRecordSet* goldset = NULL;
    DBRecord column,order,group;
    if (goldtbl)
    {
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
      column.put("num","sum(num) as num");
      column.put("price","");
      order.put("price","asc");
      group.put("price","");
      goldset = BillService::dbConnPool->exeSelect(handle,goldtbl,&column,NULL,&order,5,&group);
      firstfive->size = 0;
      if (goldset && !goldset->empty())
      {
        for(DWORD i=0; i < goldset->size() ; i++)
        {
          firstfive->list[firstfive->size].dwNum = goldset->get(i)->get("num");
          firstfive->list[firstfive->size].dwPrice = goldset->get(i)->get("price");
          firstfive->size ++;
        }
      }
      SAFE_DELETE(goldset)
    }
    BillService::dbConnPool->putHandle(handle);
  }
  return true;
}
bool Consign::cancelListAll()
{
  connHandleID handle = BillService::dbConnPool->getHandle();

  if ((connHandleID)-1 != handle)
  {
    DBFieldSet* canceltbl = BillService::metaData->getFields("CONSIGNGOLD");
    if (canceltbl)
    {
      DBRecordSet* cancelset = NULL;
      cancelset = BillService::dbConnPool->exeSelect(handle,canceltbl,NULL,NULL,NULL,0,NULL);
      if (cancelset && !cancelset->empty())
      {
        for(DWORD i=0; i < cancelset->size() ; i++)
        {
          DWORD num=cancelset->get(i)->get("num");
          DWORD accid=cancelset->get(i)->get("accid");
          Consign::addGold(accid,(DWORD)(num*1.02f));
          std::ostringstream oss;
          DBRecord column,where;
          DWORD listid = cancelset->get(i)->get("id");
          oss << "id = " << listid;
          where.put("id",oss.str());
          oss.str("");
          oss <<"accid="<<accid;
          where.put("accid",oss.str());
          if ((DWORD)-1 == BillService::dbConnPool->exeDelete(handle,canceltbl,&where))
          {
            Zebra::logger->debug("Consign::cancelListAll���ݿ����!");
            BillService::dbConnPool->putHandle(handle);
            return false;
          }      
          else
          {
            BillUser::logger("��Ʊ��ҳ���",accid,NULL,listid,(DWORD)(num*1.02f),0,NULL);
          }
        }
        SAFE_DELETE(cancelset)
      }
    }
    canceltbl = BillService::metaData->getFields("CONSIGNMONEY");
    if (canceltbl)
    {
      DBRecordSet* cancelset = NULL;
      cancelset = BillService::dbConnPool->exeSelect(handle,canceltbl,NULL,NULL,NULL,0,NULL);
      if (cancelset && !cancelset->empty())
      {
        for(DWORD i=0; i < cancelset->size() ; i++)
        {
          DWORD num=cancelset->get(i)->get("num");
          DWORD accid=cancelset->get(i)->get("accid");
          Consign::addMoney(accid,(DWORD)(num*1.02f));
          std::ostringstream oss;
          DBRecord column,where;
          DWORD listid = cancelset->get(i)->get("id");
          oss << "id = " << listid;
          where.put("id",oss.str());
          oss.str("");
          oss <<"accid="<<accid;
          where.put("accid",oss.str());
          if ((DWORD)-1 == BillService::dbConnPool->exeDelete(handle,canceltbl,&where))
          {
            Zebra::logger->debug("Consign::cancelListAll���ݿ����!");
            BillService::dbConnPool->putHandle(handle);
            return false;
          }      
          else
          {
            BillUser::logger("��Ʊ���ҳ���",accid,NULL,listid,(DWORD)(num*1.02f),0,NULL);
          }
        }
        SAFE_DELETE(cancelset)
      }
    }
    BillService::dbConnPool->putHandle(handle);
    ConsignHistoryManager::getInstance()->update();
  }
  return true;
}
bool ConsignGoldManager::cancelList(BillUser *pUser,DWORD listid)
{
  if (pUser)
  {
    connHandleID handle = BillService::dbConnPool->getHandle();
    Cmd::stReturnCancelListStockUserCmd send;
    send.id=listid;
    send.byType=Cmd::STOCK_GOLD;
    send.byReturn=Cmd::STOCK_CANCEL_ERROR;

    if ((connHandleID)-1 != handle)
    {
      DBFieldSet* canceltbl = BillService::metaData->getFields("CONSIGNGOLD");
      if (canceltbl)
      {
        std::ostringstream oss;
        DBRecord column,where;
        DBRecordSet* cancelset = NULL;
        oss << "id = " << listid;
        where.put("id",oss.str());
        oss.str("");
        oss <<"accid="<<pUser->id;
        where.put("accid",oss.str());
        cancelset = BillService::dbConnPool->exeSelect(handle,canceltbl,NULL,&where,NULL,1,NULL);
        if (cancelset && !cancelset->empty())
        {
          DWORD num=cancelset->get(0)->get("num");
          pUser->addGold(num,"��Ʊ��������");
          if ((DWORD)-1 == BillService::dbConnPool->exeDelete(handle,canceltbl,&where))
          {
            Zebra::logger->debug("ConsignGoldManager::cancelList ���ݿ����!");
            BillService::dbConnPool->putHandle(handle);
            return false;
          }      
          else
          {
            pUser->increaseGoldListNum();
            BillUser::logger("��Ʊ��ҳ���",pUser->id,pUser->name,listid,num,0,NULL);
          }
          send.byReturn=Cmd::STOCK_CANCEL_OK;
          SAFE_DELETE(cancelset)
        }
        else
        {
          Zebra::logger->debug("%s(%d)���������ڵĽ��ί�е�:%d",pUser->account,pUser->id,listid);
        }
      }
      BillService::dbConnPool->putHandle(handle);
      ConsignHistoryManager::getInstance()->update();
    }
    return pUser->sendCmdToMe(&send,sizeof(send));
  }
  return false;
}
bool ConsignGoldManager::trade()
{
  connHandleID handle = BillService::dbConnPool->getHandle();

  if ((connHandleID)-1 != handle)
  {
    DBFieldSet* goldtbl = BillService::metaData->getFields("CONSIGNGOLD");
    DBRecordSet* goldset = NULL;
    DBRecord column,order,where;
    std::ostringstream oss;         
    if (goldtbl)
    {
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
      order.put("price","asc");
      goldset = BillService::dbConnPool->exeSelect(handle,goldtbl,NULL,NULL,&order,1,NULL);
      if (goldset && !goldset->empty())
      {
        ConsignTrait list;
        list.id=goldset->get(0)->get("id");
        list.accid=goldset->get(0)->get("accid");
        list.num=goldset->get(0)->get("num");
        list.price=goldset->get(0)->get("price");
        list.time=goldset->get(0)->get("time");
        DWORD changegold=list.num;
        DWORD sysmoney=0;
        ConsignMoneyManager::getInstance()->trade(list,sysmoney);
        if (list.num && list.num < changegold)
        {
          where.clear();
          oss.str("");
          oss << "id=" << list.id;
          where.put("id",oss.str());
          column.clear();
          column.put("num",list.num);

          if ((DWORD)-1 == BillService::dbConnPool->exeUpdate(handle,goldtbl,&column,&where))
          {
            Zebra::logger->debug("ConsignGoldManager::trade ���ݿ����!");
          }
          else
          {
            BillUser::logger("��Ʊ���",list.id,NULL,list.num,changegold-list.num,0,"�������ֽ���");
            updateHistory(list.id,list.accid,changegold-list.num,list.price,list.price,list.time,Cmd::STOCK_GOLD,sysmoney);
          }
        }
        else if (!list.num)
        {
          where.clear();
          oss.str("");
          oss << "id=" << list.id;
          where.put("id",oss.str());
          if ((DWORD)-1 == BillService::dbConnPool->exeDelete(handle,goldtbl,&where))
          {
            Zebra::logger->debug("ConsignGoldManager::trade ���ݿ����!");
          }      
          else
          {
            BillUser *pUser = BillUserManager::getInstance()->getUserByID(list.accid);
            if (pUser)
            {
              pUser->increaseGoldListNum();
            }
            BillUser::logger("��Ʊ���",list.id,NULL,list.num,changegold-list.num,0,"����ȫ������");
            updateHistory(list.id,list.accid,changegold-list.num,list.price,list.price,list.time,Cmd::STOCK_GOLD,sysmoney);
          }
        }
        SAFE_DELETE(goldset)
      }
    }
    BillService::dbConnPool->putHandle(handle);
  }
  return true;
}
bool ConsignGoldManager::trade(ConsignTrait &moneylist,DWORD &sysmoney)
{
  sysmoney=0;
  connHandleID handle = BillService::dbConnPool->getHandle();

  if ((connHandleID)-1 != handle)
  {
    DBFieldSet* goldtbl = BillService::metaData->getFields("CONSIGNGOLD");
    DBRecordSet* goldset = NULL;
    DBRecord column,order,where;
    std::ostringstream oss;         
    if (goldtbl)
    {
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
      DWORD getgold=0;
      DWORD getmoney=0;
      ConsignTrait list;
      /// ÿ��ƥ�䴦��һ������
      while(moneylist.num/moneylist.price)
      {
        oss.str("");
        where.clear();
        oss << "price <= " << moneylist.price;
        where.put("price",oss.str());
        //order.put("time","asc");
        goldset = BillService::dbConnPool->exeSelect(handle,goldtbl,NULL,&where,NULL/*&order*/,1,NULL);
        if (goldset && !goldset->empty())
        {
          DWORD changegold=0;
          list.id=goldset->get(0)->get("id");
          list.accid=goldset->get(0)->get("accid");
          list.num=goldset->get(0)->get("num");
          list.price=goldset->get(0)->get("price");
          list.time=goldset->get(0)->get("time");
          changegold=list.num; 
          if (int(list.num - moneylist.num / moneylist.price) >= 0)
          {
            getgold+=moneylist.num/moneylist.price;
            //�����ϵͳ�Ե�
            if (moneylist.price >= list.price)
            {
              getmoney=(DWORD)(moneylist.num/moneylist.price) * list.price;
              sysmoney+=((DWORD)(moneylist.num/moneylist.price) * moneylist.price - getmoney);
            }
            else
            {
              getmoney=(DWORD)(moneylist.num/moneylist.price) * moneylist.price;
            }
            moneylist.num=moneylist.num%moneylist.price;
            list.num -= getgold; 
          }
          else
          {
            getgold += list.num;
            moneylist.num -=(list.num*moneylist.price);
            if (moneylist.price >= list.price)
            {
              getmoney=list.num * list.price;
              sysmoney+=(list.num * moneylist.price - getmoney);
            }
            else
            {
              getmoney=list.num * moneylist.price;
            }
            list.num = 0;
          }
          if (list.num)
          {
            where.clear();
            oss.str("");
            oss << "id=" << list.id;
            where.put("id",oss.str());
            column.clear();
            column.put("num",list.num);

            if ((DWORD)-1 == BillService::dbConnPool->exeUpdate(handle,goldtbl,&column,&where))
            {
              Zebra::logger->debug("ConsignGoldManager::trade ���ݿ����!");
            }      
            else
            {
              BillUser::logger("��Ʊ���",list.id,NULL,list.num,changegold-list.num,0,"�������ֽ���");
              updateHistory(list.id,list.accid,changegold-list.num,list.price,/*moneylist.price,*/list.price,list.time,Cmd::STOCK_GOLD,sysmoney);
              sysmoney=0;
            }
          }
          else
          {
            where.clear();
            oss.str("");
            oss << "id=" << list.id;
            where.put("id",oss.str());
            if ((DWORD)-1 == BillService::dbConnPool->exeDelete(handle,goldtbl,&where))
            {
              Zebra::logger->debug("ConsignGoldManager::trade ���ݿ����!");
            }      
            else
            {
              BillUser *pUser = BillUserManager::getInstance()->getUserByID(list.accid);
              if (pUser)
              {
                pUser->increaseGoldListNum();
              }
              BillUser::logger("��Ʊ���",list.id,NULL,list.num,changegold-list.num,0,"����ȫ������");
              updateHistory(list.id,list.accid,changegold-list.num,list.price,list.price,/*moneylist.price,*/list.time,Cmd::STOCK_GOLD,sysmoney);
              sysmoney=0;
            }
          }
          if (getmoney)
          {
            BillUser *pUser = BillUserManager::getInstance()->getUserByID(list.accid);
            if (pUser)
            {
              pUser->addMoney(getmoney,"��Ʊ�õ�");
            }
            else
            {
              Consign::addMoney(list.accid,getmoney); 
            }
          }
          if (getgold)
          {
            BillUser *pUser = BillUserManager::getInstance()->getUserByID(moneylist.accid);
            if (pUser)
            {
              pUser->addGold(getgold,"��Ʊ�õ�");
            }
            else
            {
              Consign::addGold(moneylist.accid,getgold); 
            }
          }
          getmoney=0;
          getgold=0;
          SAFE_DELETE(goldset)
        }
        else
        {
          break;
        }
      }
    }
    BillService::dbConnPool->putHandle(handle);
  }
  return true;
}
ConsignMoneyManager *ConsignMoneyManager::instance=NULL;
ConsignMoneyManager::ConsignMoneyManager()
{
  firstfive->byType=Cmd::STOCK_MONEY;
}
ConsignMoneyManager::~ConsignMoneyManager()
{
}
ConsignMoneyManager *ConsignMoneyManager::getInstance()
{
  if (instance == NULL)
    instance = new ConsignMoneyManager();
  return instance;
}
void ConsignMoneyManager::delInstance()
{
  SAFE_DELETE(instance);
}
bool ConsignMoneyManager::init()
{
  connHandleID handle = BillService::dbConnPool->getHandle();

  if ((connHandleID)-1 != handle)
  {
    DBFieldSet* moneytbl = BillService::metaData->getFields("CONSIGNMONEY");
    DBRecordSet* moneyset = NULL;
    DBRecord column,order,group;
    if (moneytbl)
    {
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
      column.put("num","sum(num) as num");
      column.put("price","price");
      order.put("price","desc");
      group.put("price","");
      moneyset = BillService::dbConnPool->exeSelect(handle,moneytbl,&column,NULL,&order,5,&group);
      firstfive->size = 0;
      if (moneyset && !moneyset->empty())
      {
        firstfive->size = 0;
        for(DWORD i=0; i < moneyset->size() ; i++)
        {
          firstfive->list[firstfive->size].dwNum = moneyset->get(i)->get("num");
          firstfive->list[firstfive->size].dwPrice = moneyset->get(i)->get("price");
          firstfive->size ++;
        }
      }
      SAFE_DELETE(moneyset)
    }
    BillService::dbConnPool->putHandle(handle);
  }
  return true;
}
bool ConsignMoneyManager::cancelList(BillUser *pUser,DWORD listid)
{
  connHandleID handle = BillService::dbConnPool->getHandle();
  if (pUser)
  {
    Cmd::stReturnCancelListStockUserCmd send;
    send.id=listid;
    send.byType=Cmd::STOCK_MONEY;
    send.byReturn=Cmd::STOCK_CANCEL_ERROR;

    if ((connHandleID)-1 != handle)
    {
      DBFieldSet* canceltbl = BillService::metaData->getFields("CONSIGNMONEY");
      if (canceltbl)
      {
        std::ostringstream oss;
        DBRecord column,where;
        DBRecordSet* cancelset = NULL;
        oss << "id = " << listid;
        where.put("id",oss.str());
        oss.str("");
        oss <<"accid="<<pUser->id;
        where.put("accid",oss.str());
        cancelset = BillService::dbConnPool->exeSelect(handle,canceltbl,&column,&where,NULL,1,NULL);
        if (cancelset && !cancelset->empty())
        {
          DWORD num=cancelset->get(0)->get("num");
          pUser->addMoney(num,"��Ʊ�򵥳���");
          if ((DWORD)-1 == BillService::dbConnPool->exeDelete(handle,canceltbl,&where))
          {
            Zebra::logger->debug("ConsignMoneyManager::cancelList ���ݿ����!");
            BillService::dbConnPool->putHandle(handle);
            return false;
          }      
          else
          {
            pUser->increaseMoneyListNum();
            BillUser::logger("��Ʊ���ҳ���",pUser->id,pUser->name,listid,num,0,NULL);
          }
          send.byReturn=Cmd::STOCK_CANCEL_OK;
          SAFE_DELETE(cancelset)
        }
        else
        {
          Zebra::logger->debug("%s(%d)���������ڵĽ��ί�е�:%d",pUser->account,pUser->id,listid);
        }
      }
      BillService::dbConnPool->putHandle(handle);
      ConsignHistoryManager::getInstance()->update();
    }
    return pUser->sendCmdToMe(&send,sizeof(send));
  }
  return false;
}
bool ConsignMoneyManager::trade()
{
  connHandleID handle = BillService::dbConnPool->getHandle();

  if ((connHandleID)-1 != handle)
  {
    DBFieldSet* moneytbl = BillService::metaData->getFields("CONSIGNMONEY");
    DBRecordSet* moneyset = NULL;
    DBRecord column,order,where;
    std::ostringstream oss;         
    if (moneytbl)
    {
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
      order.put("price","desc");
      moneyset = BillService::dbConnPool->exeSelect(handle,moneytbl,NULL,NULL,&order,1,NULL);
      if (moneyset && !moneyset->empty())
      {
        ConsignTrait list;
        list.id=moneyset->get(0)->get("id");
        list.accid=moneyset->get(0)->get("accid");
        list.num=moneyset->get(0)->get("num");
        list.price=moneyset->get(0)->get("price");
        list.time=moneyset->get(0)->get("time");
        DWORD changemoney=list.num;
        DWORD sysmoney=0;
        ConsignGoldManager::getInstance()->trade(list,sysmoney);
        if (list.num && list.num < changemoney && list.num >= list.price)
        {
          where.clear();
          oss.str("");
          oss << "id=" << list.id;
          where.put("id",oss.str());
          column.clear();
          column.put("num",list.num);

          if ((DWORD)-1 == BillService::dbConnPool->exeUpdate(handle,moneytbl,&column,&where))
          {
            Zebra::logger->debug("ConsignMoneyManager::trade ���ݿ����!");
          }
          else
          {
            BillUser::logger("��Ʊ����",list.id,NULL,list.num,changemoney-list.num,0,"�������ֽ���");
            updateHistory(list.id,list.accid,changemoney-list.num,list.price,list.price,list.time,Cmd::STOCK_MONEY,sysmoney);
          }
        }
        else if (!list.num)
        {
          where.clear();
          oss.str("");
          oss << "id=" << list.id;
          where.put("id",oss.str());
          if ((DWORD)-1 == BillService::dbConnPool->exeDelete(handle,moneytbl,&where))
          {
            Zebra::logger->debug("ConsignMoneyManager::trade ���ݿ����!");
          }      
          else
          {
            BillUser *pUser = BillUserManager::getInstance()->getUserByID(list.accid);
            if (pUser)
            {
              pUser->increaseMoneyListNum();
            }
            BillUser::logger("��Ʊ����",list.id,NULL,list.num,changemoney-list.num,0,"����ȫ������");
            updateHistory(list.id,list.accid,changemoney-list.num,list.price,list.price,list.time,Cmd::STOCK_MONEY,sysmoney);
          }
        }
        // ���������һ�����Ҳ�򲻵���ϵͳ����
        else if (list.num < list.price)
        {
          //���������һ�����,����(����Ӧ�ò�������������)
          BillUser *pUser = BillUserManager::getInstance()->getUserByID(list.accid);
          if (pUser)
          {
            cancelList(pUser,list.id);
          }
        }
      }
      SAFE_DELETE(moneyset);
    }
    BillService::dbConnPool->putHandle(handle);
  }
  return true;
}
bool ConsignMoneyManager::trade(ConsignTrait &goldlist,DWORD &sysmoney)
{
  sysmoney=0;
  connHandleID handle = BillService::dbConnPool->getHandle();

  if ((connHandleID)-1 != handle)
  {
    DBFieldSet* moneytbl = BillService::metaData->getFields("CONSIGNMONEY");
    DBRecordSet* moneyset = NULL;
    DBRecord column,order,where;
    std::ostringstream oss;         
    if (moneytbl)
    {
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
      DWORD getgold=0;
      DWORD getmoney=0;
      ConsignTrait list;
      /// ÿ��ƥ�䴦��һ����
      while(goldlist.num)
      {
        oss.str("");
        where.clear();
        order.clear();
        oss << "price >= " << goldlist.price;
        where.put("price",oss.str());
        oss.str("");
        oss << "num >= price";
        where.put("num",oss.str());
        order.put("price","desc");
        moneyset = BillService::dbConnPool->exeSelect(handle,moneytbl,NULL,&where,&order,1,NULL);
        if (moneyset && !moneyset->empty())
        {
          DWORD changemoney=0;
          list.id=moneyset->get(0)->get("id");
          list.accid=moneyset->get(0)->get("accid");
          list.num=moneyset->get(0)->get("num");
          list.price=moneyset->get(0)->get("price");
          list.time=moneyset->get(0)->get("time");
          changemoney=list.num; 
          if ((int)(list.num - goldlist.num * list.price) >= 0)
          {
            getgold=goldlist.num;
            //���ϵͳ�Ե�
            //getmoney+=goldlist.num * list.price;
            if (list.price >= goldlist.price)
            {
              getmoney=goldlist.num * goldlist.price;
              sysmoney+=(goldlist.num * list.price - getmoney);
            }
            else
            {
              getmoney=goldlist.num * list.price;
            }
            list.num -= goldlist.num * list.price; 
            goldlist.num=0;
          }
          else
          {
            getgold = list.num/list.price;
            goldlist.num -=getgold;
            if (list.price >= goldlist.price)
            {
              //getmoney+=getgold*list.price;
              getmoney=getgold * goldlist.price;
              sysmoney+=(getgold * list.price - getmoney);
            }
            else
            {
              getmoney=getgold*list.price;
            }
            list.num -= getgold * list.price;
          }
          if (list.num && list.num/list.price)
          {
            where.clear();
            oss.str("");
            oss << "id=" << list.id;
            where.put("id",oss.str());
            column.clear();
            column.put("num",list.num);

            if ((DWORD)-1 == BillService::dbConnPool->exeUpdate(handle,moneytbl,&column,&where))
            {
              Zebra::logger->debug("ConsignMoneyManager::trade ���ݿ����!");
            }      
            else
            {
              BillUser::logger("��Ʊ����",list.id,NULL,list.num,changemoney-list.num,0,"�������ֽ���");
              updateHistory(list.id,list.accid,changemoney-list.num,list.price,/*goldlist.price,*/list.price,list.time,Cmd::STOCK_MONEY,sysmoney);
              sysmoney=0;
            }
          }
          else if (!list.num)
          {
            where.clear();
            oss.str("");
            oss << "id=" << list.id;
            where.put("id",oss.str());
            if ((DWORD)-1 == BillService::dbConnPool->exeDelete(handle,moneytbl,&where))
            {
              Zebra::logger->debug("ConsignMoneyManager::trade ���ݿ����!");
            }      
            else
            {
              BillUser *pUser = BillUserManager::getInstance()->getUserByID(list.accid);
              if (pUser)
              {
                pUser->increaseMoneyListNum();
              }
              BillUser::logger("��Ʊ����",list.id,NULL,list.num,changemoney-list.num,0,"����ȫ������");
              updateHistory(list.id,list.accid,changemoney-list.num,list.price,list.price,/*goldlist.price,*/list.time,Cmd::STOCK_MONEY,sysmoney);
              sysmoney=0;
            }
          }
          else
          {
            //���������һ�����,����(����Ӧ�ò�������������)
            BillUser *pUser = BillUserManager::getInstance()->getUserByID(list.accid);
            if (pUser)
            {
              cancelList(pUser,list.id);
            }
          }
          if (getgold)
          {
            BillUser *pUser = BillUserManager::getInstance()->getUserByID(list.accid);
            if (pUser)
            {
              pUser->addGold(getgold,"��Ʊ�õ�");
            }
            else
            {
              Consign::addGold(list.accid,getgold); 
            }
          }
      if (getmoney)
      {
        BillUser *pUser = BillUserManager::getInstance()->getUserByID(goldlist.accid);
        if (pUser)
        {
          pUser->addMoney(getmoney,"��Ʊ�õ�");
        }
        else
        {
          Consign::addMoney(goldlist.accid,getmoney); 
        }
      }
          getgold=0;
          getmoney=0;
          SAFE_DELETE(moneyset);
        }
        else
        {
          break;
        }
      }
    }
    BillService::dbConnPool->putHandle(handle);
  }
  return true;
}
ConsignHistoryManager *ConsignHistoryManager::instance=NULL;
ConsignHistoryManager::ConsignHistoryManager():_one_min(60)
{
  bzero(&newHistoryMoney,sizeof(newHistoryMoney));
  bzero(&newHistoryGold,sizeof(newHistoryGold));
}
ConsignHistoryManager::~ConsignHistoryManager()
{
}
ConsignHistoryManager *ConsignHistoryManager::getInstance()
{
  if (instance == NULL)
    instance = new ConsignHistoryManager();
  return instance;
}
void ConsignHistoryManager::delInstance()
{
  SAFE_DELETE(instance);
}
bool ConsignHistoryManager::init()
{
  connHandleID handle = BillService::dbConnPool->getHandle();

  if ((connHandleID)-1 != handle)
  {
    //��ȡ��ʷ���
    DBFieldSet* history = BillService::metaData->getFields("CONSIGNGOLDHISTORY");
    DBRecordSet* recordset = NULL;
    std::ostringstream oss;         
    DBRecord column,order,group,where;                           
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
    if (history)
    {
      column.put("num","sum(num) as num");
      column.put("price","price");
      column.put("oktime","oktime");
      order.put("oktime","desc");
      group.put("oktime","");
      recordset = BillService::dbConnPool->exeSelect(handle,history,&column,NULL,&order,0,&group);
      if (recordset && !recordset->empty())
      {
        for(DWORD i=0; i < recordset->size() && i < 1024000; i++)
        {
          Cmd::ConsignHistoryType trait;
          trait.dwTotal = recordset->get(i)->get("num");
          trait.dwPrice = recordset->get(i)->get("price");
          trait.dwTime = recordset->get(i)->get("oktime");
          //Zebra::logger->debug("��ʷ�������:(ʱ��:%d,�۸�:%d,�ɽ���:%d)",trait.dwTime,trait.dwPrice,trait.dwTotal);
          historyGold.insert(HistoryIndex::value_type(trait.dwTime/3600,trait));
        }
      }
      SAFE_DELETE(recordset);
    }
    //��ȡ��ʷ����
    history = BillService::metaData->getFields("CONSIGNMONEYHISTORY");
    if (history)
    {
      column.clear();
      column.put("num","sum(num) as num");
      column.put("price","price");
      column.put("oktime","oktime");
      order.clear();
      order.put("oktime","desc");
      group.clear();
      group.put("oktime","");
      recordset = BillService::dbConnPool->exeSelect(handle,history,&column,NULL,&order,0,&group);
      if (recordset && !recordset->empty())
      {
        for(DWORD i=0; i < recordset->size() && i < 1024000; i++)
        {
          Cmd::ConsignHistoryType trait;
          trait.dwTotal = recordset->get(i)->get("num");
          trait.dwPrice = recordset->get(i)->get("price");
          trait.dwTime = recordset->get(i)->get("oktime");
          //Zebra::logger->debug("��ʷ��������:(ʱ��:%d,�۸�:%d,�ɽ���:%d)",trait.dwTime,trait.dwPrice,trait.dwTotal);
          historyMoney.insert(HistoryIndex::value_type(trait.dwTime/3600,trait));
        }
      }
    SAFE_DELETE(recordset);
    }
    BillService::dbConnPool->putHandle(handle);
  }
  return true;
}
bool ConsignHistoryManager::update()
{
  //if (_one_min(BillTimeTick::currentTime))
  {
    //�����ż�
    ConsignGoldManager::getInstance()->init();
    //���ż�
    ConsignMoneyManager::getInstance()->init();
    connHandleID handle = BillService::dbConnPool->getHandle();

    if ((connHandleID)-1 != handle)
    {
      DBFieldSet* history = BillService::metaData->getFields("CONSIGNGOLDHISTORY");
      DBRecordSet* recordset = NULL;
      std::ostringstream oss;         
      DBRecord column,order,group,where;                           
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
      if (history)
      {
        column.put("num","sum(num) as num");
        column.put("price","price");
        column.put("oktime","oktime");
        oss << "oktime = " << BillTimeTick::currentTime.sec() / 60 -1 ;
        where.put("oktime",oss.str());
        group.put("oktime","");
        recordset = BillService::dbConnPool->exeSelect(handle,history,&column,&where,NULL,1,&group);
        if (recordset && !recordset->empty())
        {
          newHistoryGold.dwTotal = recordset->get(0)->get("num");
          newHistoryGold.dwPrice = recordset->get(0)->get("price");
          newHistoryGold.dwTime = recordset->get(0)->get("oktime");
          historyGold.insert(HistoryIndex::value_type(newHistoryGold.dwTime/3600,newHistoryGold));
          //Zebra::logger->debug("������ʷ����:(ʱ��:%d,�۸�:%d,�ɽ���:%d",newHistoryGold.dwTime,newHistoryGold.dwPrice,newHistoryGold.dwTotal);
        }
      }
      SAFE_DELETE(recordset);
      history = BillService::metaData->getFields("CONSIGNMONEYHISTORY");
      /*
         DBRecordSet* exeSelect(DBFieldSet* table,DBRecord* column,DBRecord* where,DBRecord* order = NULL,
         DWORD limit=0,
         DBRecord* groupby = NULL,DBRecord* having = NULL);
      // */
      if (history)
      {
        column.clear();
        column.put("num","sum(num) as num");
        column.put("price","price");
        column.put("oktime","oktime");
        oss.str("");
        oss << "oktime = " << BillTimeTick::currentTime.sec() / 60 -1 ;
        where.clear();
        where.put("oktime",oss.str());
        group.clear();
        group.put("oktime","");
        recordset = BillService::dbConnPool->exeSelect(handle,history,&column,&where,NULL,1,&group);
        if (recordset && !recordset->empty())
        {
          newHistoryMoney.dwTotal = recordset->get(0)->get("num");
          newHistoryMoney.dwPrice = recordset->get(0)->get("price");
          newHistoryMoney.dwTime = recordset->get(0)->get("oktime");
          historyMoney.insert(HistoryIndex::value_type(newHistoryMoney.dwTime/3600,newHistoryMoney));
          //Zebra::logger->debug("������ʷ����:(ʱ��:%d,�۸�:%d,�ɽ���:%d",newHistoryMoney.dwTime,newHistoryMoney.dwPrice,newHistoryMoney.dwTotal);
        }
      }
      SAFE_DELETE(recordset);
      BillService::dbConnPool->putHandle(handle);
    }
  }
  return true;
}
bool ConsignHistoryManager::sendDataToUser(BillUser *pUser,DWORD begintime,DWORD num)
{
  if (pUser)
  {
    //TODO  ��num������
    if (num && num<=720)
    {
      static char Buf[sizeof(Cmd::stHistoryGoldStockUserCmd) + 720 * 60 * sizeof(Cmd::ConsignHistoryType)];
      bzero(Buf,sizeof(Buf));
      Cmd::stHistoryGoldStockUserCmd *sendgold = (Cmd::stHistoryGoldStockUserCmd*)Buf;
      bzero(sendgold,sizeof(Cmd::stHistoryGoldStockUserCmd));
      constructInPlace(sendgold);
      DWORD begin=begintime/3600;
      for(DWORD i = 0; i < num ; i++)
      {
        HistoryRange range = historyGold.equal_range(begin--);
        HistoryIndex::iterator iter;
        for(iter = range.first;iter !=range.second;iter++)
        {
          //Zebra::logger->debug("�����������ʷ����:(ʱ��:%d,�۸�:%d,�ɽ���:%d",iter->second.dwTime,iter->second.dwPrice,iter->second.dwTotal);
          bcopy(&iter->second,&sendgold->list[sendgold->size],sizeof(Cmd::ConsignHistoryType),sizeof(Cmd::ConsignHistoryType));
          sendgold->size++;
        }
      }
      if (sendgold->size)
      {
        return pUser->sendCmdToMe(sendgold,sizeof(Cmd::stHistoryGoldStockUserCmd) +sizeof(Cmd::ConsignHistoryType)* sendgold->size);
      }
      Cmd::stHistoryMoneyStockUserCmd *sendmoney = (Cmd::stHistoryMoneyStockUserCmd*)Buf;
      bzero(sendmoney,sizeof(Cmd::stHistoryMoneyStockUserCmd));
      constructInPlace(sendmoney);
      begin=begintime/3600;
      for(DWORD i = 0; i < num ; i++)
      {
        HistoryRange range = historyMoney.equal_range(begin--);
        HistoryIndex::iterator iter;
        for(iter = range.first;iter !=range.second;iter++)
        {
          //Zebra::logger->debug("�������������ʷ����:(ʱ��:%d,�۸�:%d,�ɽ���:%d",iter->second.dwTime,iter->second.dwPrice,iter->second.dwTotal);
          bcopy(&iter->second,&sendmoney->list[sendmoney->size],sizeof(Cmd::ConsignHistoryType),sizeof(Cmd::ConsignHistoryType));
          sendmoney->size++;
        }
      }
      if (sendmoney->size)
      {
        return pUser->sendCmdToMe(sendmoney,sizeof(Cmd::stHistoryMoneyStockUserCmd) + sizeof(Cmd::ConsignHistoryType) * sendmoney->size);
      }
    }
    else
    {
      //ֻ�����������ɵ�����
      bool need=false;
      Cmd::stNewHistoryStockUserCmd send;
      if (newHistoryGold.dwTime)
      {
        send.gold = newHistoryGold;
        need=true;
      }
      if (newHistoryMoney.dwTime)
      {
        send.gold = newHistoryGold;
        need=true;
      }
      if (need)
      {
        return pUser->sendCmdToMe(&send,sizeof(send));
      }
    }
  }
  return true;
}
bool ConsignHistoryManager::sendSelfDataToUser(BillUser *pUser,DWORD begintime,DWORD num)
{
  if (pUser)
  {
    //TODO  ��num������
    if (num && num<=720)
    {
      connHandleID handle = BillService::dbConnPool->getHandle();

      if ((connHandleID)-1 != handle)
      {
        //��ȡ��ʷ���
        DBFieldSet* history = BillService::metaData->getFields("CONSIGNGOLDHISTORY");
        DBRecordSet* recordset = NULL;
        std::ostringstream oss;         
        DBRecord order,group,where;                           
        static char Buf[sizeof(Cmd::stSelfHistoryGoldStockUserCmd) + 720 * 60 * sizeof(Cmd::stSelfHistoryGoldStockUserCmd)];
        DWORD min = begintime; 
        if (!begintime)
          min = BillUser::getRealMinTime();
        DWORD begin=min/3600 - num;
        if (history)
        {
          Cmd::stSelfHistoryGoldStockUserCmd *sendgold = (Cmd::stSelfHistoryGoldStockUserCmd*)Buf;
          bzero(sendgold,sizeof(Cmd::stSelfHistoryGoldStockUserCmd));
          constructInPlace(sendgold);
          oss << "accid = "<<pUser->id;
          where.put("accid",oss.str());
          oss.str("");
          oss << "committime > "<<begin;
          where.put("committime",oss.str());
          recordset = BillService::dbConnPool->exeSelect(handle,history,NULL,&where,NULL,0,NULL);
          if (recordset && !recordset->empty())
          {
            for(DWORD i=0; i < recordset->size() && i < num * 60; i++)
            {
              sendgold->list[sendgold->size].dwID = recordset->get(i)->get("id");
              sendgold->list[sendgold->size].wdNum= recordset->get(i)->get("num");
              sendgold->list[sendgold->size].wdCommitPrice = recordset->get(i)->get("commitprice");
              sendgold->list[sendgold->size].wdPrice = recordset->get(i)->get("price");
              sendgold->list[sendgold->size].dwCommitTime = recordset->get(i)->get("committime");
              sendgold->list[sendgold->size].dwOkTime = recordset->get(i)->get("oktime");
              //Zebra::logger->debug("���%s(%d)�����Լ������ʷ����:%d,%d,%d,%d,%d",pUser->account,pUser->id,sendgold->list[sendgold->size].dwID,sendgold->list[sendgold->size].wdNum,sendgold->list[sendgold->size].wdCommitPrice,sendgold->list[sendgold->size].wdPrice,sendgold->list[sendgold->size].dwCommitTime,sendgold->list[sendgold->size].dwOkTime);
              sendgold->size++;
            }
            SAFE_DELETE(recordset);
          }
          if (sendgold->size)
          {
            pUser->sendCmdToMe(sendgold,sizeof(Cmd::stSelfHistoryGoldStockUserCmd) + sizeof(Cmd::ConsignSelfHistoryType) * sendgold->size);
          }
        }
        history = BillService::metaData->getFields("CONSIGNMONEYHISTORY");
        if (history)
        {
          Cmd::stSelfHistoryMoneyStockUserCmd *sendmoney = (Cmd::stSelfHistoryMoneyStockUserCmd*)Buf;
          bzero(sendmoney,sizeof(Cmd::stSelfHistoryMoneyStockUserCmd));
          constructInPlace(sendmoney);
          oss.str("");
          oss << "accid = "<<pUser->id;
          where.clear();
          where.put("accid",oss.str());
          oss.str("");
          oss << "committime > "<<begin;
          where.put("committime",oss.str());
          recordset = BillService::dbConnPool->exeSelect(handle,history,NULL,&where,NULL,0,NULL);
          if (recordset && !recordset->empty())
          {
            for(DWORD i=0; i < recordset->size() && i < num * 60; i++)
            {
              sendmoney->list[sendmoney->size].dwID = recordset->get(i)->get("id");
              sendmoney->list[sendmoney->size].wdNum= recordset->get(i)->get("num");
              sendmoney->list[sendmoney->size].wdCommitPrice = recordset->get(i)->get("commitprice");
              sendmoney->list[sendmoney->size].wdPrice = recordset->get(i)->get("price");
              sendmoney->list[sendmoney->size].dwCommitTime = recordset->get(i)->get("committime");
              sendmoney->list[sendmoney->size].dwOkTime = recordset->get(i)->get("oktime");
              //Zebra::logger->debug("���%s(%d)�����Լ�������ʷ����:%d,%d,%d,%d,%d,%d",pUser->account,pUser->id,sendmoney->list[sendmoney->size].dwID,sendmoney->list[sendmoney->size].wdNum,sendmoney->list[sendmoney->size].wdCommitPrice,sendmoney->list[sendmoney->size].wdPrice,sendmoney->list[sendmoney->size].dwCommitTime,sendmoney->list[sendmoney->size].dwOkTime);
              sendmoney->size++;
            }
            SAFE_DELETE(recordset);
          }
          if (sendmoney->size)
          {
            pUser->sendCmdToMe(sendmoney,sizeof(Cmd::stSelfHistoryMoneyStockUserCmd) + sizeof(Cmd::ConsignSelfHistoryType) * sendmoney->size);
          }
        }
        BillService::dbConnPool->putHandle(handle);
      }
    }
    else
    {
      Zebra::logger->debug("%u�����Լ���ʷ������������:%d",pUser->id,num);
    }
  }
  return true;
}
bool Consign::addGold(DWORD accid,DWORD num)
{
  connHandleID handle = BillService::dbConnPool->getHandle();
  if ((connHandleID)-1 != handle)
  {
    DBFieldSet* balance = BillService::metaData->getFields("BALANCE");
    DBRecord column,where;                           
    std::ostringstream oss;         
    if (balance)
    {
      oss <<"accid="<<accid;
      where.put("accid",oss.str());
      oss.str("");
      oss << "gold + "<< num;
      column.put("gold",oss.str());
      if ((DWORD)-1 == BillService::dbConnPool->exeUpdate(handle,
            balance,&column,&where))
      {
      }
      else
      {
        BillUser::logger("���",accid,NULL,0,num,1,"��Ʊ���߽���");
      }
    }
    BillService::dbConnPool->putHandle(handle);
  }
  return true;
}
bool Consign::addMoney(DWORD accid,DWORD num)
{
  connHandleID handle = BillService::dbConnPool->getHandle();
  if ((connHandleID)-1 != handle)
  {
    DBFieldSet* balance = BillService::metaData->getFields("BALANCE");
    DBRecord column,where;                           
    std::ostringstream oss;         
    if (balance)
    {
      oss <<"accid="<<accid;
      where.put("accid",oss.str());
      oss.str(""); 
      oss << "money + " << num;
      column.put("money",oss.str());
      if ((DWORD)-1 == BillService::dbConnPool->exeUpdate(handle,
            balance,&column,&where))
      {
      }
      else
      {
        BillUser::logger("����",accid,NULL,0,num,1,"��Ʊ���߽���");
      }
    }
    BillService::dbConnPool->putHandle(handle);
  }
  return true;
}
