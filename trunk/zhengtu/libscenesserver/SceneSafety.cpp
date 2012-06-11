/**
 * \brief ʵ�ֲƲ���������Ĵ���
 *
 */
#include <zebra/ScenesServer.h>

/**
  * \brief �����û��Ʋ���������
  *
  *
  * \param rev: ����
  * \param cmdLen: �����
  *
  * \return ���������TRUE,����ΪFALSE
  *
  *
  */
bool SceneUser::doSafetyCmd(const Cmd::stSafetyUserCmd *rev,DWORD cmdLen)
{
  switch(rev->byParam)
  {
    case SET_SAFETY_PARA:
      {
        Cmd::stSetSafetyUserCmd* cmd = (Cmd::stSetSafetyUserCmd*)rev;

        if (cmd->state>0)
        {
          this->safety = 1;
          this->safety_setup = 0;
          Cmd::set_state((BYTE*)&this->safety_setup,Cmd::SAFE_SPLIT_OBJECT);
          Cmd::set_state((BYTE*)&this->safety_setup,Cmd::SAFE_THIRD_PACK);
          Cmd::set_state((BYTE*)&this->safety_setup,Cmd::SAFE_GOLD_STOCK);

          Cmd::stNotifySafetyUserCmd send;
          send.byState = Cmd::SAFETY_OPEN;
          send.safe_setup = this->safety_setup;
          sendCmdToMe(&send,sizeof(send));
        }

        this->save(Cmd::Record::OPERATION_WRITEBACK);
        return true;
      }
      break;
    case SET_TEMP_UNSAFETY_PARA:
      {
        this->temp_unsafety_state = 1;

        Cmd::stNotifySafetyUserCmd send;
        send.byState = Cmd::SAFETY_TEMP_CLOSE;
        send.safe_setup = this->safety_setup;

        sendCmdToMe(&send,sizeof(send));
        return true;
      }
      break;
    case SET_SAFETY_DETAIL_PARA:
      {
        Cmd::stNotifySafetyUserCmd send;
        Cmd::stSetSafetyDetailUserCmd* cmd = (Cmd::stSetSafetyDetailUserCmd*)rev;

        if (this->safety==1)
        {
          send.byState = Cmd::SAFETY_OPEN;

          if (this->temp_unsafety_state == 1)
            send.byState = Cmd::SAFETY_TEMP_CLOSE;

          this->safety_setup = 0;

          if (Cmd::isset_state((BYTE*)&cmd->safe_setup,Cmd::SAFE_SPLIT_OBJECT))
          {
            Cmd::set_state((BYTE*)&this->safety_setup,Cmd::SAFE_SPLIT_OBJECT);
          }

          if (Cmd::isset_state((BYTE*)&cmd->safe_setup,Cmd::SAFE_THIRD_PACK))
          {
            Cmd::set_state((BYTE*)&this->safety_setup,Cmd::SAFE_THIRD_PACK);
          }

          if (Cmd::isset_state((BYTE*)&cmd->safe_setup,Cmd::SAFE_GOLD_STOCK))
          {
            Cmd::set_state((BYTE*)&this->safety_setup,Cmd::SAFE_GOLD_STOCK);
          }

          send.safe_setup = this->safety_setup;
          sendCmdToMe(&send,sizeof(send));
        }

        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

