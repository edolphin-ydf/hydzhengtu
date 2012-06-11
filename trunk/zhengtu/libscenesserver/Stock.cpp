#include <zebra/ScenesServer.h>


bool SceneUser::doStockCmd(const Cmd::stStockSceneUserCmd *ptCmd,DWORD cmdLen)
{
  using namespace Cmd;
  using namespace Cmd::Bill;
  switch(ptCmd->byParam)
  {
    case TRANSFER_FUND_SAVE_STOCKPARA:
      {
        stTransferFundStockSaveUserCmd *rev = (stTransferFundStockSaveUserCmd *)ptCmd;
        if (!this->packs.checkGold(rev->dwGold) || !this->packs.checkMoney(rev->dwMoney))
        {
          Zebra::logger->debug("%s(%d)��Ʊ��ֵʱ���ֽ���,���������",this->name,this->id);
        }
        else
        {
          this->stockSave(rev->dwMoney,rev->dwGold);
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}
bool SceneUser::stockSave(DWORD mon,DWORD go)
{
  DWORD realmon=0,realgo=0;
  bool bret=false;
  if (mon)
  {
    if (this->packs.removeMoney(mon,"��Ʊ��ֵ"))
    {
      realmon=mon;
      bret=true;
    }
  }
  if ((!mon && !bret && go) ||(mon && bret && go))
  {
    if (this->packs.removeGold(go,"��Ʊ��ֵ",false))
    {
      realgo=go;
      bret=true;
    }
  }
  if (bret)
  {
    Cmd::Bill::t_Stock_Save save;
    save.dwMoney=realmon;
    save.dwGold=realgo;
    this->sendSceneCmdToBill(&save,sizeof(save));
    Zebra::logger->debug("%s(%d)��Ʊ��ֵ�ɹ�,���:%d,����:%d",this->name,this->id,go,mon);
  }
  else
  {
    Zebra::logger->debug("%s(%d)��Ʊ��ֵʧ��,���:%d,����:%d",this->name,this->id,go,mon);
  }
  return bret;
}




