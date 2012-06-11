#include <zebra/ScenesServer.h>

Dice::Dice(SceneUser * u1,SceneUser * u2,DWORD m)
  :tempid1(0),tempid2(0),value1(0),value2(0),continue1(false),continue2(false)
{
  bzero(name1,sizeof(name1));
  bzero(name2,sizeof(name2));

  round = 0;
  money = m;

  if (!u1 || !u2) return;

  tempid1 = u1->tempid;
  tempid2 = u2->tempid;

  strncpy(name1,u1->name,MAX_NAMESIZE);
  strncpy(name2,u2->name,MAX_NAMESIZE);

  gameState = DICE_STATE_END;
}

bool Dice::setReady(SceneUser * user)
{
  if (!user->packs.checkMoney(money) || !user->packs.removeMoney(money,"С��Ϸ:Ѻ��"))
  {
    sendAllInfo("%s ��Ǯ����",user->name);
    return false;
  }

  if (user->tempid==tempid1)
  {
    continue1 = true;
    sendAllInfo("%s ׼�����",name1);
  }
  else if (user->tempid==tempid2)
  {
    continue2 = true;
    sendAllInfo("%s ׼�����",name2);
  }

  if (continue1 && continue2)
    return init();

  return true;
}

bool Dice::init()
{
  SceneUser * u1 = SceneUserManager::getMe().getUserByTempID(tempid1);
  SceneUser * u2 = SceneUserManager::getMe().getUserByTempID(tempid2);
  if (!u1||!u2) return false;

  if (round>0)//n��Ŀ
  {
    round++;
    //money *= 2;
  }
  else
    round = 1;

  continue1 = continue2 = false;
  value1 = value2 = 0;

  zRTime t;
  startTime = t.sec();

  Cmd::stEnterMiniGame send;
  u1->sendCmdToMe(&send,sizeof(send));
  u2->sendCmdToMe(&send,sizeof(send));

  gameState = DICE_STATE_ROLLING;

  std::ostringstream os;
  os<<u1->name<<" vs "<<u2->name<<" �� "<<round<<" �� Ѻ�� ";
  if (money/10000) os<<money/10000<<"��";
  if ((money%10000)/100) os<<(money%10000)/100<<"��";
  if (money%100) os<<money%100<<"��";

  sendAllInfo(os.str().c_str());
  Zebra::logger->debug("%s",os.str().c_str());

  return true;
}

bool Dice::timer(DWORD time,SceneUser * u)
{
  //if (!u) return false;
  if (Dice::DICE_STATE_ROLLING==gameState && time>=startTime+20)
  {
    if (!value1) rotate(tempid1);
    if (!value2) rotate(tempid2);
  }
  return true;
}

bool Dice::rotate(DWORD id)
{
  if (id==tempid1)
  {
    value1 = randBetween(1,6);

    Cmd::stDiceNumMiniGame send;
    send.tempid = id;
    send.num = value1;
    sendCmdToAll(&send,sizeof(send));

    sendAllInfo("%s ���� %u ��",name1,value1);
  }
  if (id==tempid2)
  {
    value2 = randBetween(1,6);

    Cmd::stDiceNumMiniGame send;
    send.tempid = id;
    send.num = value2;
    sendCmdToAll(&send,sizeof(send));

    sendAllInfo("%s ���� %u ��",name2,value2);
  }

  if (value1 && value2)
    judge();

  return true;
}

bool Dice::judge()
{
  SceneUser * pUser1 = SceneUserManager::getMe().getUserByTempID(tempid1);
  SceneUser * pUser2 = SceneUserManager::getMe().getUserByTempID(tempid2);

  Cmd::stResultMiniGame send;
  send.res = 1;
  if (value1>value2)
  {
    if (pUser1)
    {
      pUser1->sendCmdToMe(&send,sizeof(send));
      pUser1->packs.addMoney(money*2,"С��Ϸ:Ӯ��");

      Zebra::logger->info("%s �� %s ��С��Ϸ,Ӯ�ý�Ǯ %u",name1,name2,money*2);
    }

    send.res = 0;
    if (pUser2) pUser2->sendCmdToMe(&send,sizeof(send));
  }
  else
    if (value2>value1)
    {
      if (pUser2)
      {
        pUser2->sendCmdToMe(&send,sizeof(send));
        pUser2->packs.addMoney(money*2,"С��Ϸ:Ӯ��");

        Zebra::logger->info("%s �� %s ��С��Ϸ,Ӯ�ý�Ǯ %u",name2,name1,money*2);
      }

      send.res = 0;
      if (pUser1) pUser1->sendCmdToMe(&send,sizeof(send));
    }
    else
    {
      send.res = 2;
      if (pUser1)
      {
        pUser1->packs.addMoney(money,"С��Ϸ:ƽ��");
        pUser1->sendCmdToMe(&send,sizeof(send));
        Zebra::logger->info("%s �� %s ��С��Ϸ,ƽ�ַ��ؽ�Ǯ %u",name1,name2,money);
      }
      if (pUser2)
      {
        pUser2->packs.addMoney(money,"С��Ϸ:ƽ��");
        pUser2->sendCmdToMe(&send,sizeof(send));
        Zebra::logger->info("%s �� %s ��С��Ϸ,ƽ�ַ��ؽ�Ǯ %u",name2,name1,money);
      }
    }

  gameState = DICE_STATE_END;

  return true;
}

void Dice::endGame(SceneUser * u)
{
  SceneUser * pUser1 = SceneUserManager::getMe().getUserByTempID(tempid1);
  SceneUser * pUser2 = SceneUserManager::getMe().getUserByTempID(tempid2);

  if (gameState==DICE_STATE_ROLLING)
  {
    if (pUser1 && pUser1!=u)
    {
      pUser1->packs.addMoney(money,"С��Ϸ:��;�˳�");
      Zebra::logger->info("%s �� %s ��С��Ϸ��;����,���ؽ�Ǯ %u state=DICE_STATE_ROLLING",name1,name2,money);
    }
    if (pUser2 && pUser2!=u)
    {
      pUser2->packs.addMoney(money,"С��Ϸ:��;�˳�");
      Zebra::logger->info("%s �� %s ��С��Ϸ��;����,���ؽ�Ǯ %u state=DICE_STATE_ROLLING",name2,name1,money);
    }
  }
  else if (gameState==DICE_STATE_END)
  {
    if (pUser1 && continue1)
    {
      pUser1->packs.addMoney(money,"С��Ϸ:��;�˳�");
      Zebra::logger->info("%s �� %s ��С��Ϸ��;����,���ؽ�Ǯ %u state=DICE_STATE_END",name1,name2,money);
    }
    if (pUser2 && continue2)
    {
      pUser2->packs.addMoney(money,"С��Ϸ:��;�˳�");
      Zebra::logger->info("%s �� %s ��С��Ϸ��;����,���ؽ�Ǯ %u state=DICE_STATE_END",name2,name1,money);
    }
  }

  Cmd::stEndMiniGame send;
  send.reason = 1;

  if (pUser1)
  {
    pUser1->sendCmdToMe(&send,sizeof(send));
    pUser1->miniGame = 0;
  }
  if (pUser2)
  {
    pUser2->sendCmdToMe(&send,sizeof(send));
    pUser2->miniGame = 0;
  }
}

DWORD Dice::getMoney()
{
  return money;
}

DWORD Dice::getTheOtherTempID(DWORD id)
{
  if (id==tempid1) return tempid2;
  if (id==tempid2) return tempid1;
  return 0;
}

Dice::DiceState Dice::getState()
{
  return gameState;
}

bool Dice::sendCmdToAll(const void *cmd,const DWORD len)
{
  SceneUser * pUser1 = SceneUserManager::getMe().getUserByTempID(tempid1);
  SceneUser * pUser2 = SceneUserManager::getMe().getUserByTempID(tempid2);

  if (pUser1) pUser1->sendCmdToMe(cmd,len);
  if (pUser2) pUser2->sendCmdToMe(cmd,len);

  return pUser1 && pUser2;
}

#define getMessage(msg,msglen,pat)  \
      do  \
{   \
      va_list ap; \
      bzero(msg,msglen); \
      va_start(ap,pat);      \
      vsnprintf(msg,msglen - 1,pat,ap);    \
      va_end(ap); \
}while(false)

bool Dice::sendAllInfo(const char *pattern,...)
{
  char buf[MAX_CHATINFO];
  getMessage(buf,MAX_CHATINFO,pattern);

  Cmd::stChannelChatUserCmd send; 
  zRTime ctv;                 
  send.dwChatTime = ctv.sec();
  send.dwType=Cmd::CHAT_TYPE_MINIGAME;
  strncpy((char *)send.pstrChat,buf,MAX_CHATINFO-1);
  strncpy(send.pstrName,"ϵͳ",MAX_NAMESIZE-1);

  return sendCmdToAll(&send,sizeof(send));
}
