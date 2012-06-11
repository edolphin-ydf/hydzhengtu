/**
 * \brief ʵ�ֹ������ָ��Ĵ���
 *
 * 
 */
#include <zebra/ScenesServer.h>

/**
 * \brief ִ�о����������
 *
 *
 * \param rev ��սָ��
 * \param cmdLen ��Ϣ����
 * \return �Ƿ�ɹ�
 */
bool SceneUser::doArmyCmd(const Cmd::stArmyUserCmd *rev,DWORD cmdLen)
{
  Zebra::logger->debug("doArmyCmd receive byParam:[%d]",rev->byParam);

  switch (rev->byParam)
  {
    case REQ_ARMY_LIST_PARA:
      {
        Cmd::stReqArmyListUserCmd* cmd = (Cmd::stReqArmyListUserCmd*)rev;
        Cmd::Session::t_ReqArmyList_SceneSession  send;

        send.byType = cmd->byType;
        send.dwUserID = this->id;
        send.dwCityID = this->scene->getRealMapID();
        sessionClient->sendCmd(&send,sizeof(send));
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

