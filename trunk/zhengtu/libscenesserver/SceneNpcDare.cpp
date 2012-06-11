/**
 * \brief NPC����ս����ģ��
 *
 * 
 */

#include <zebra/ScenesServer.h>

using namespace NpcDareDef;

/**
 * \brief �ȽϽ�ս��Ʒ
 *
 */

class NpcDareObjectCompare:public UserObjectCompare 
{
  public:
    DWORD  dwObjectID;

    bool isIt(zObject *object)
    {
      if (object->data.dwObjectID == dwObjectID) return true;
      return false;
    }
};



/**
 * \brief ִ����սָ��
 *
 *
 * \param rev ��սָ��
 * \param cmdLen ��Ϣ����
 * \return �Ƿ�ɹ�
 */
bool SceneUser::doNpcDareCmd(const Cmd::stDareUserCmd *rev,DWORD cmdLen)
{
#ifdef _DEBUG
       Zebra::logger->debug("doNpcDareCmd����[%d]����Ϣ",rev->byParam);
#endif 
  switch (rev->byParam)
  {
    case Cmd::QUESTION_NPCDARE_INFO_PARA:
      {
        Cmd::Session::t_questionNpcDare_SceneSession send;
        Cmd::stQuestionNpcDareInfo* ptCmd = (Cmd::stQuestionNpcDareInfo*)rev;
          
        send.dwUserID = this->id;
        send.dwCountryID = this->scene->getCountryID();
        send.dwMapID = this->scene->getRealMapID();
        send.dwNpcID = ptCmd->dwNpcID;
        send.byType = ptCmd->byType;
        sessionClient->sendCmd(&send,sizeof(send));
        return true;
      }
      break;
    case Cmd::NPCDARE_DARE_PARA:
      {
        Cmd::stDareNpcDare * pCmd = (Cmd::stDareNpcDare *) rev;
        SceneNpc *pNpc = SceneNpcManager::getMe().getNpcByTempID(pCmd->dwNpcID);
        if (pNpc)
        {
          if (scene->checkTwoPosIInNine(getPosI(),pNpc->getPosI()))
          {
            if (scene->getCountryID() == this->charbase.country)
            {
              if (this->charbase.septid !=0)
              {
                /*
                NpcDareObjectCompare found;
                found.dwObjectID = CREATE_NPCDARE_NEED_ITEM;
                zObject *itemobj = packs.uom.getObject(found);// ���ҵ���
                if (itemobj)
                {
                  packs.removeObject(itemobj); //notify and delete
                  Cmd::Session::t_NpcDare_Dare_SceneSession send;

                  send.dwCountryID = scene->getCountryID();
                  send.dwMapID = scene->getRealMapID();
                  send.dwNpcID = pNpc->npc->id;
                  send.dwUserID = this->id;
                  sessionClient->sendCmd(&send,sizeof(send));
                  return true;
                }
                else
                {
                  Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"ȱ����ս������ߣ�");
                }
                */
                if (this->packs.checkMoney(4000))
                {
                  Cmd::Session::t_NpcDare_Dare_SceneSession send;

                  send.dwCountryID = scene->getCountryID();
                  send.dwMapID = scene->getRealMapID();
                  send.dwNpcID = pNpc->npc->id;
                  send.dwUserID = this->id;
                  sessionClient->sendCmd(&send,sizeof(send));
                  return true;
                }
                else
                {
                  Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"ȱ���㹻��Ǯ��������ս��");
                }
              }
              else
              {
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�㻹û�м��岻�ܷ�����ս��");
              }
            }
            else
            {
              Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�����Լ������޷�������ս��");
            }
          }
          else
          {
            Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"����̫Զ���޷��Ի���");
          }
        }
        return true;
      }
      break;
    case Cmd::NPCDARE_GETGOLD_PARA:
      {
        Cmd::stDareNpcDare * pCmd = (Cmd::stDareNpcDare *) rev;
        Cmd::Session::t_NpcDare_GetGold_SceneSession send;
        send.dwUserID = this->id;
        send.dwNpcID = pCmd->dwNpcID;
        send.dwCountryID = this->scene->getCountryID();
        send.dwMapID = this->scene->getRealMapID();
        sessionClient->sendCmd(&send,sizeof(send));

        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

