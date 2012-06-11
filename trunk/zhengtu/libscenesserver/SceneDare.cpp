/**
 * \brief ʵ�ֽ�ս�������
 *
 * 
 */

#include <zebra/ScenesServer.h>

using namespace DareDef;

/**
 * \brief ִ����սָ��
 *
 *
 * \param rev ��սָ��
 * \param cmdLen ��Ϣ����
 * \return �Ƿ�ɹ�
 */
bool SceneUser::doDareCmd(const Cmd::stDareUserCmd *rev,DWORD cmdLen)
{
  switch (rev->byParam)
  {
    case Cmd::ACTIVE_UNION_CITY_DARE_PARA:
      {
        Cmd::Session::t_UnionCity_Dare_SceneSession send;
        Cmd::stActiveUnionCityDare* cmd = (Cmd::stActiveUnionCityDare*)rev;
        int need_money = 0;
        if (this->scene->getCountryID() == PUBLIC_COUNTRY)
        {
          need_money = CREATE_UNION_NEUTRAL_CITY_DARE_NEED_PRICE_MONEY;
        } 
        else  if (this->scene->getRealMapID() == 139)
        {
          need_money = CREATE_UNION_KING_CITY_DARE_NEED_PRICE_MONEY;
        }
        else
        {
          need_money = CREATE_UNION_CITY_DARE_NEED_PRICE_MONEY;
        }
        
        if (this->charbase.unionid == 0)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"����û�н�����ᣬ���ܷ�����ս");
          return true;
        }

        if (packs.checkMoney(need_money) 
          && packs.removeMoney(need_money,"���ս�۳���ս����")) 
        {
          Zebra::logger->info("[���ս]:%s �۳������ս��ս���� %d",
              this->name,need_money);  

          send.dwCountryID = this->charbase.country;

          if (cmd->toCountryID == 0 || cmd->toCountryID>20)
          {
            send.dwToCountryID = this->scene->getCountryID();
          }
          else
          {
            send.dwToCountryID = cmd->toCountryID;
          }

          send.dwCityID = this->scene->getRealMapID();
          send.dwFromUserID = this->id;
          send.dwFromUnionID = this->charbase.unionid; 
          sessionClient->sendCmd(&send,sizeof(send));
        }
        else
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"�������������ܷ�����ս");
        }

        return true;
      }
      break;
    /*case Cmd::ACTIVE_DARE_PARA:
      {
        Cmd::stActiveDareCmd *pCmd = (Cmd::stActiveDareCmd *)rev;

        Cmd::Session::t_activeDare_SceneSession send;
        send.dwWarID = pCmd->dwWarID;
        Zebra::logger->debug("�սӵ��Ựת��������ACTIVE_DARE_PARA����");

        if (packs.checkMoney(CREATE_DARE_NEED_PRICE_GOLD) && packs.removeMoney(CREATE_DARE_NEED_PRICE_GOLD)) {
          send.dwStatus = Cmd::Session::SCENE_ACTIVEDARE_SUCCESS;
        }
        else
        {
          //����û���㹻��Ǯ
          Channel::sendSys(this,Cmd::INFO_TYPE_GAME,"û���㹻��Ǯ��֧����ս���ã�");
          send.dwStatus = Cmd::Session::SCENE_ACTIVEDARE_FAIL;
        }

        sessionClient->sendCmd(&send,sizeof(Cmd::Session::t_activeDare_SceneSession));



        return true;
      }
      break;*/
  }
  return false;
}

