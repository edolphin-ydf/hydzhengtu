/**
 * \brief ʵ��ʦ������Ĵ���
 *
 */
#include <zebra/ScenesServer.h>

/**
  * \brief һ���Ƚ���
  *
  *  ���ڲ��ҽ���ʦ������Ҫ�ĵ����Ƿ����
  *  
  *
  */
class SchoolObjectCompare:public UserObjectCompare 
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
  * \brief �����û�ʦ������
  *
  * ����Ĺ�ϵ��������:
  *
  * Cmd::ADD_MEMBER_TO_SCHOOL_PARA
  *
  * Cmd::SCHOOL_STATUS_CHECK_PARA:
  *
  *  Cmd::CREATE_SCHOOL_PARA:
  *
  * \param rev: ʦ������
  * \param cmdLen: �����
  *
  * \return ���������TRUE,����ΪFALSE
  *
  *
  */
bool SceneUser::doSchoolCmd(const Cmd::stSchoolUserCmd *rev,DWORD cmdLen)
{
  switch(rev->byParam)
  {
    case Cmd::ADD_MEMBER_TO_SCHOOL_PARA:
      {
        Cmd::stAddMemberToSchoolCmd *ptCmd=(Cmd::stAddMemberToSchoolCmd *)rev;
        switch(ptCmd->byState)
        {
          case Cmd::TEACHER_QUESTION:
            {
              if (charbase.level <TEACHER_LEVEL)
              {
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�����ﵽ%d����������ͽ��",TEACHER_LEVEL);
                return true;
              }
              //SceneUser *pUser=scene->getUserByName(ptCmd->memberName);
              SceneUser *pUser=SceneUserManager::getMe().getUserByName(ptCmd->memberName);
              if (pUser)
              {
                if (pUser != this)
                {
                  if (pUser->charbase.level >10)
                  {
                    /*
                    if (!isset_state(pUser->sysSetting,Cmd::USER_SETTING_SCHOOL))
                    {
                      Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��� %s ����ʦ��δ����",pUser->name);
                      return true;
                    }
                    */
                    if (scene->checkTwoPosIInNine(getPosI(),pUser->getPosI()))
                    {
                      forwardSession(rev,cmdLen);
                    }
                    else
                    {
                      Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"����̫Զ�޷�����");
                    }
                  }
                  else
                  {
                    Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�Է�δ��10��,������Ϊͽ�ܣ�");
                  }
                }
                else
                {
                  Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�ѵ��������Լ�Ϊͽ��?");
                }
              }
              else
              {
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��� %s ����ͬһ��ͼ,�޷���Ӧ����",ptCmd->memberName);
              }
              return true;
            }
            break;
          case Cmd::TEACHER_ANSWER_YES:
            {
              SceneUser *pUser=scene->getUserByTempID(ptCmd->memberID);
              if (pUser)
              {
                forwardSession(rev,cmdLen);
              }
              else
              {
                Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�Է��Ѿ��뿪,���������������");
              }
              return true;
            }
            break;
          case Cmd::TEACHER_ANSWER_NO:
            {
              SceneUser *pUser=scene->getUserByTempID(ptCmd->memberID);
              if (pUser)
              {
                Channel::sendSys(pUser,Cmd::INFO_TYPE_FAIL,"%s��Ը�������ͽ��,�ܾ����������",name);
              }
              return true;
            }
            break;
          default:
            break;
        }
      }
      break;
    case Cmd::SCHOOL_STATUS_CHECK_PARA:
    case Cmd::CREATE_SCHOOL_PARA:
      {
        if (charbase.level >= CREATE_SCHOOL_REQUEST_LEVEL)
        {
          SchoolObjectCompare found;
          found.dwObjectID = CREATE_SCHOOL_REQUEST_ITEM_ID;
          zObject *itemobj = packs.uom.getObject(found);// ���ҵ���
          if (itemobj)
          {
/*
            zObject *gold=packs.getGold();
            if (gold&&gold->data.dwNum >= CREATE_SCHOOL_REQUEST_PRICE_GOLD)
            {
              forwardSession(rev,cmdLen);
            }
*/
            if (packs.checkMoney(CREATE_SCHOOL_REQUEST_PRICE_GOLD) /*&& packs.removeMoney(CREATE_SCHOOL_REQUEST_PRICE_GOLD)*/) {
              forwardSession(rev,cmdLen);
            }
            else
            {
              //����û���㹻��Ǯ
              Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"û���㹻��Ǯ����������!");
            }
          }
          else 
          {
            //����û�е���
            Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"ȱ����������޷���������!");
          }
        }
        else
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"��ļ��𲻹����ܴ�������,�ٻ�ȥ�ú�������!");
        }
        return true;
      }
      break;
    default:
      break;
  }
  return false;
}

