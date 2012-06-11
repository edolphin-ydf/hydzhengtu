/**
 * \brief 实现国家相关指令的处理
 *
 * 
 */
#include <zebra/ScenesServer.h>

/**
 * \brief 执行军队相关命令
 *
 *
 * \param rev 挑战指令
 * \param cmdLen 消息长度
 * \return 是否成功
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

