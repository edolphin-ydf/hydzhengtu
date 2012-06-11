/**
 * \brief ϵͳ��ʾ��Ϣ
 *
 * 
 */
#include <zebra/ScenesServer.h>

MessageSystem *MessageSystem::instance = NULL;

/**
 * \brief ������Ϣϵͳ�������ļ�
 *
 *
 * \return �����Ƿ�ɹ�
 */
bool MessageSystem::init()
{
  final();

  rwlock.wrlock();
  zXMLParser xml;
  if (!xml.initFile(Zebra::global["confdir"] + "messageSystem.xml"))
  {
    Zebra::logger->error("����messageSystem.xmlʧ��");
    return false;
  }

  xmlNodePtr root = xml.getRootNode("messageSystem");
  if (root)
  {
    xmlNodePtr node = xml.getChildNode(root,"messages");
    while(node)
    {
      if (strcmp((char *)node->name,"messages") == 0)
      {
        t_Message mm;
        char strtime[64];
        struct tm tm;
        int login = 0;

        bzero(&mm,sizeof(mm));
        constructInPlace(&mm.cmd);
        mm.endtime=-1;
        xml.getNodePropNum(node,"login",&login,sizeof(login));
        if (login)
          mm.login = true;
        else
          mm.login = false;
        xml.getNodePropNum(node,"interval",&mm.interval,sizeof(mm.interval));
        if (xml.getNodePropStr(node,"starttime",strtime,sizeof(strtime)))
        {
          if (strptime(strtime,"%Y/%m/%d %H:%M:%S",&tm)!=NULL)
          {
            time_t t=mktime(&tm);
            if (t!=(time_t)-1) mm.starttime=t;
          }
        }
        if (xml.getNodePropStr(node,"endtime",strtime,sizeof(strtime)))
        {
          if (strptime(strtime,"%Y/%m/%d %H:%M:%S",&tm)!=NULL)
          {
            time_t t=mktime(&tm);
            if (t!=(time_t)-1) mm.endtime=t;
          }
        }
        mm.count=xml.getChildNodeNum(node,"message");
        //�Ƿ�˳����
        int order=0;
        if (xml.getNodePropNum(node,"order",&order,sizeof(order)) && order)
          mm.order=0;//���㿪ʼ,˳����
        else
          mm.order=mm.count;

        bzero(mm.cmd.pstrName,sizeof(mm.cmd.pstrName));
        mm.cmd.dwType = Cmd::CHAT_TYPE_SYSTEM;

        xmlNodePtr snode = xml.getChildNode(node,"message");
        while(snode)
        {
          if (strcmp((char *)snode->name,"message") == 0)
          {
            xml.getNodeContentStr(snode,mm.cmd.pstrChat,sizeof(mm.cmd.pstrChat));
            //Zebra::logger->debug("%s",mm.cmd.pstrChat);
          }
          messages.push_back(mm);

          snode = xml.getNextNode(snode,NULL);
        }

      }

      node = xml.getNextNode(node,NULL);
    }
    rwlock.unlock();
    Zebra::logger->info("��ʼ���Զ���Ϣϵͳ�ɹ�");
    return true;
  }
  rwlock.unlock();

  Zebra::logger->error("������Ϣϵͳ�����ļ�ʧ��");
  return false;
}

/**
 * \brief ж����Ϣϵͳ
 *
 *
 */
void MessageSystem::final()
{
  rwlock.wrlock();
  while(!messages.empty())
  {
    messages.pop_back();
  }
  rwlock.unlock();
}

/**
 * \brief ��ָ���û�����ָ�����͵���Ϣ
 *
 *
 * \param sceneUser: �û�
 * \param init: �Ƿ��Ǹոյ�½
 */
void MessageSystem::check(SceneUser *sceneUser,const bool init)
{
  rwlock.rdlock();
  if (!messages.empty())
  {
    if (init)
    {
      int count=0;
      time_t t=time(NULL);
      while(count<(int)messages.size() && messages[count].count!=0)
      {
        if (messages[count].login &&//��½����
            t>=messages[count].starttime &&//�ѿ�ʼ
            (messages[count].endtime==-1 || t<=messages[count].endtime))//δ����
        {
          //Zebra::logger->debug("���ͣ�%s",messages[count].cmd.pstrChat);
          sceneUser->sendCmdToMe(&messages[count].cmd,sizeof(messages[count].cmd));
        }
        //Zebra::logger->debug("%s",messages[count].cmd.pstrChat);
        count++;
      }

    }
    else
    {
      int count=0;
      time_t t=time(NULL);
      while(count<(int)messages.size() && messages[count].count!=0)
      {
        if ((messages[count].interval==0 && t==messages[count].starttime) ||//һ���Է���
            (messages[count].interval!=0 && t%messages[count].interval==0 &&//��ʱ����
             t>=messages[count].starttime &&//�ѿ�ʼ
             (messages[count].endtime==-1 || t<=messages[count].endtime)//δ����
            ))
        {
          int i;
          if (messages[count].count==messages[count].order)
            i=randBetween(count,count+messages[count].count-1);//�����һ�������û�
          else
          {
            i=count+sceneUser->messageOrder;
            if (messages[count].count-1<=sceneUser->messageOrder)
              sceneUser->messageOrder=0;
            else
              sceneUser->messageOrder++;
          }
          sceneUser->sendCmdToMe(&messages[i].cmd,sizeof(messages[i].cmd));
        }
        count+=messages[count].count;
      }
    }
  }
  rwlock.unlock();
}

