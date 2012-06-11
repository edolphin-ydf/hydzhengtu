/**
 * \brief ʵ�־����������
 *
 * 
 */
#include <zebra/ScenesServer.h>

using namespace QuizDef;

/**
 * \brief �ȽϽ�ս��Ʒ
 *
 */
/*
class DareObjectCompare:public UserObjectCompare 
{
  public:
    DWORD  dwObjectID;

    bool isIt(zObject *object)
    {
      if (object->data.dwObjectID == dwObjectID) return true;
      return false;
    }
};
*/


/**
 * \brief ִ����սָ��
 *
 *
 * \param rev ��սָ��
 * \param cmdLen ��Ϣ����
 * \return �Ƿ�ɹ�
 */
bool SceneUser::doQuizCmd(const Cmd::stQuizUserCmd *rev,DWORD cmdLen)
{
  //Zebra::logger->debug("doQuizCmd receive byParam:[%d]",rev->byParam);

  switch (rev->byParam)
  {
    case Cmd::QUERY_QUIZ_PARA:
      {
        Cmd::stQueryQuiz* cmd = (Cmd::stQueryQuiz*)rev;
        
        if (cmd->byMsgType == Cmd::QUIZ_YES)
        {
          this->setState(zSceneEntry::SceneEntry_Hide);

          //֪ͨ9��ɾ���ý�ɫ
          Cmd::stRemoveUserMapScreenUserCmd remove;
          remove.dwUserTempID = this->tempid;
          this->scene->sendCmdToNine(this->getPosI(),&remove,sizeof(remove),this->dupIndex);

          //�Ƴ�block
          this->scene->clearBlock(this->getPos());  
          this->isQuiz = true;
        }
        
        return true;
      }
      break;
    case Cmd::CREATE_QUIZ_PARA:
      {//��������������;�����ʼָ���Session
        Cmd::stCreateQuiz* cmd = (Cmd::stCreateQuiz*)rev;
        Cmd::Session::t_createQuiz_SceneSession send;
        DWORD needMoney = 0;
        DWORD subjects = 20;

        if (this->charbase.answerCount == 0)
        {
          this->charbase.answerCount = 1;
        }

        if (this->charbase.answerCount>5)
        {
          Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�������Ѿ��ش�5����Ŷ,����Ϣ��Ϣ��");
          return true;

        }

//        needMoney = (DWORD)(0.05*this->charbase.level * (0.5+0.5*0.02*this->charbase.level));
//  ��Ǯ��ȡ������=��int(��ɫ�ȼ�/20 + 1) * ��n + int(��ɫ�ȼ�/30 + 1)��
        needMoney = (DWORD)((this->charbase.level/20+1) * ((this->charbase.answerCount)+(this->charbase.level/30+1)));
        needMoney = needMoney * 100;
/*
        if (this->charbase.answerCount>1)
        {
          needMoney = (DWORD)(needMoney * (this->charbase.answerCount));
        }
*/

        if (needMoney<100)
        {
          needMoney = 100;
        }
        
        if (cmd->byType == Cmd::CREATE_QUESTION)
        {
          if (this->packs.checkMoney(needMoney))
          {
            cmd->dwMoney = needMoney;
            this->sendCmdToMe(cmd,sizeof(Cmd::stCreateQuiz));
          }
          else
          {
            Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,
                "�������� %d �ģ����ܽ����ʴ�",needMoney);
          }

          return true;
        }
        
        if (cmd->byType == Cmd::CREATE_YES)
        {
          if (needMoney>0)  
          {
            if (this->packs.checkMoney(needMoney) 
                && this->packs.removeMoney(needMoney,"����"))
            {
              Zebra::logger->info("[���˴���]:%s ���δ��⻨��%d,��%d�δ���",
                  this->name,needMoney,this->charbase.answerCount);

              this->charbase.answerCount++;
              
              send.dwUserID = this->id;
              send.type = QuizDef::PERSONAL_QUIZ;
              send.dwSubjects = subjects; 
              sessionClient->sendCmd(&send,sizeof(send));
              // ��������״̬
              this->setState(zSceneEntry::SceneEntry_Hide);

              //֪ͨ9��ɾ���ý�ɫ
              Cmd::stRemoveUserMapScreenUserCmd remove;
              remove.dwUserTempID = this->tempid;
              this->scene->sendCmdToNine(this->getPosI(),&remove,sizeof(remove),this->dupIndex);

              //�Ƴ�block
              this->scene->clearBlock(this->getPos());
              this->isQuiz = true;
            }
            else
            {
              Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,
                "��������,���ܽ��������ʴ�");
            }
          }
          else
          {
            Channel::sendSys(this,Cmd::INFO_TYPE_FAIL,"�ȼ�����10��,���ܽ��������ʴ�");
          }  

          return true;
        }

        return true;
      }
      break;
    default:
      break;
  }
  
  return false;
}

