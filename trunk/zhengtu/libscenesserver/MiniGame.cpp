#include <zebra/ScenesServer.h>

bool SceneUser::doMiniGameCmd(const Cmd::stMiniGameUserCmd *cmd,DWORD cmdLen)
{
  using namespace Cmd;
  switch (cmd->byParam)
  {
    case INVITE_MINI_PARA:
      {
        stInviteMiniGame * rev = (stInviteMiniGame *)cmd;

        if (miniGame)
        {
          Channel::sendSys(this,INFO_TYPE_FAIL,"���Ƚ��������ڽ��е���Ϸ");
          return true;
        }

        if (!packs.checkMoney(rev->money))
        {
          Channel::sendSys(this,INFO_TYPE_FAIL,"��û����ô��Ǯ");
          return true;
        }

        if (rev->money<100 || rev->money>10000)
        {
          Channel::sendSys(this,INFO_TYPE_FAIL,"Ѻ�������1����1��֮��");
          return true;
        }

        SceneUser * pUser = SceneUserManager::getMe().getUserByTempID(rev->tempid);

        if (!pUser || !scene->checkTwoPosIInNine(getPosI(),pUser->getPosI()))
        {
          Channel::sendSys(this,INFO_TYPE_FAIL,"�Է����ڸ��������ܺ���һ����");
          return true;
        }

        if (!isset_state(pUser->sysSetting,Cmd::USER_SETTING_MINIGAME))
        {
          Channel::sendSys(this,INFO_TYPE_FAIL,"%s �����κ�����С��Ϸ",pUser->name);
          return true;
        }

        if (pUser->miniGame)
        {
          Channel::sendSys(this,INFO_TYPE_FAIL,"%s �Ѿ��ͱ����濪��",pUser->name);
          return true;
        }

        rev->tempid = tempid;
        pUser->sendCmdToMe(rev,cmdLen);
        return true;
      }
      break;
    case INVITE_RET_MINI_PARA:
      {
        stInviteRetMiniGame * rev = (stInviteRetMiniGame *)cmd;
        SceneUser * pUser = SceneUserManager::getMe().getUserByTempID(rev->tempid);
        if (!pUser || !scene->checkTwoPosIInNine(getPosI(),pUser->getPosI()))
        {
          Channel::sendSys(this,INFO_TYPE_FAIL,"�Է��Ѿ���Զ�ˣ����ܺ���һ����");
          return true;
        }

        if (0==rev->ret)//��ͬ����
        {
          Channel::sendSys(pUser,INFO_TYPE_FAIL,"%s ��ͬ���������Ϸ",name);
          return true;
        }

        if (!packs.checkMoney(rev->money))
        {
          Channel::sendSys(this,INFO_TYPE_FAIL,"��û����ô��Ǯ");
          Channel::sendSys(pUser,INFO_TYPE_FAIL,"%s û����ô��Ǯ",name);
          return true;
        }

        if (miniGame)
        {
          Channel::sendSys(pUser,INFO_TYPE_FAIL,"%s �Ѿ��ͱ����濪��",name);
          return true;
        }

        if (pUser->miniGame)
        {
          Channel::sendSys(this,INFO_TYPE_FAIL,"%s �Ѿ��ͱ����濪��",pUser->name);
          return true;
        }

        //������Ϸ
        miniGame = pUser->miniGame = new Dice(this,pUser,rev->money);
        if (!miniGame) return false;

        stStartMiniGame send;
        send.money = rev->money;

        send.tempid = pUser->tempid;
        sendCmdToMe(&send,sizeof(send));
        send.tempid = tempid;
        pUser->sendCmdToMe(&send,sizeof(send));

        return true;
      }
      break;
    case STOP_DICE_MINI_PARA:
      {
        if (miniGame && Dice::DICE_STATE_ROLLING==miniGame->getState())
          miniGame->rotate(tempid);
        return true;
      }
      break;
    case END_MINI_PARA:
      {
        if (miniGame)
        {
          Dice * temp = miniGame;
          miniGame->endGame(this);

          delete temp;
          miniGame = 0;
        }
        return true;
      }
      break;
    case CONTINUE_MINI_PARA:
      {
        if (miniGame && Dice::DICE_STATE_END==miniGame->getState())
        {
          Cmd::stContinueMiniGame * rev = (Cmd::stContinueMiniGame *)cmd;
          if (0==rev->ret || !miniGame->setReady(this))
          {
            Dice * temp = miniGame;
            miniGame->endGame();

            delete temp;
            miniGame = 0;

            return true;
          }
        }
      }
      break;
    default:
      break;
  }
  return false;
}
