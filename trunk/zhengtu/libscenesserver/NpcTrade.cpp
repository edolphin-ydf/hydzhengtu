/**
 * \brief Npc�����Ի���
 *
 * 
 */

#include <zebra/ScenesServer.h>

NpcTrade *NpcTrade::instance = NULL;

/**
 * \brief ��ȡnpctade�����ļ�
 *
 *
 * \return ��ȡ�����ļ��Ƿ�ɹ�1111
 */
bool NpcTrade::init()
{
  final();

  zXMLParser xml;
  if (!xml.initFile(Zebra::global["confdir"] + "npctrade.xml"))
  {
    Zebra::logger->error("����npctrade.xmlʧ��");
    return false;
  }

  xmlNodePtr root = xml.getRootNode("NpcTrade");
  if (root)
  {
    rwlock.wrlock();

    xmlNodePtr node = xml.getChildNode(root,NULL);
    while(node)
    {
      if (strcmp((char *)node->name,"npc") == 0)
      {
        NpcDialog dialog;
        xml.getNodePropNum(node,"id",&dialog.npcid,sizeof(dialog.npcid));
        xmlNodePtr snode = xml.getChildNode(node,NULL);
        while(snode)
        {
          if (strcmp((char *)snode->name,"menu") == 0)
          {
            xml.getNodeContentStr(snode,dialog.menu,sizeof(dialog.menu));
          }
          else if (strcmp((char *)snode->name,"item") == 0)
          {
            NpcItem item;
            xml.getNodePropNum(snode,"id",&item.id,sizeof(item.id));
            xml.getNodePropNum(snode,"kind",&item.kind,sizeof(item.kind));
            xml.getNodePropNum(snode,"lowlevel",&item.lowLevel,sizeof(item.lowLevel));
            if (!xml.getNodePropNum(snode,"level",&item.level,sizeof(item.level))) item.level=1000;
            xml.getNodePropNum(snode,"action",&item.action,sizeof(item.action));
            xml.getNodePropNum(snode,"itemlevel",&item.itemlevel,sizeof(item.itemlevel));
            dialog.items.insert(NpcItemMultiMap::value_type(item.id,item));
          }

          snode = xml.getNextNode(snode,NULL);
        }

        dialogs.insert(NpcDialogMultiMap::value_type(dialog.npcid,dialog));
      }

      node = xml.getNextNode(node,NULL);
    }
    rwlock.unlock();
    Zebra::logger->info("��ʼ��Npc����ϵͳ�ɹ�.");
    return true;
  }

#if 0
  for(NpcDialogMultiMap::iterator it = dialogs.begin(); it != dialogs.end(); it++)
  {
    Zebra::logger->debug("%u,%s",it->second.id,it->second.menu);
    for(NpcItemMultiMap::iterator item = it->second.items.begin(); item != it->second.items.end(); item++)
    {
      Zebra::logger->debug("%u,%u,%u",item->second.id,item->second.kind,item->second.action);
    }
  }
#endif

  Zebra::logger->error("����npc���������ļ�ʧ��");
  return false;
}

/**
 * \brief ����npcid�õ�menu����
 *
 *
 * \param npcid: npcid
 * \param menuTxt: �˵�����(���)
 * \return �ҵ�����true,���򷵻�false
 */
bool NpcTrade::getNpcMenu(const DWORD npcid,char *menuTxt)
{
  rwlock.rdlock();
  for(NpcDialogMultiMap::iterator it = dialogs.begin(); it != dialogs.end(); it++)
  {
    if (it->second.npcid == npcid)
    {
      //Zebra::logger->debug("%u,%s",it->second.id,it->second.menu);
      strcpy(menuTxt,it->second.menu);
      rwlock.unlock();
      return true;
    }
  }
  rwlock.unlock();

  return false;
}

/**
 * \brief ����id�������жϲ����Ƿ�Ϸ�
 *
 *
 * \param npcid: npcid
 * \param item: NpcItem
 * \return ���Խ��еĲ�������true,���򷵻�false
 */
bool NpcTrade::verifyNpcAction(const DWORD npcid,const NpcItem &item)
{
  if (0 == npcid) return false;

  rwlock.rdlock();
  for(NpcDialogMultiMap::const_iterator npcDialog = dialogs.begin(); npcDialog != dialogs.end(); npcDialog++)
  {
    if (npcDialog->second.npcid == npcid)
    {
      //�ҵ���Npc
      for(NpcItemMultiMap::const_iterator it = npcDialog->second.items.begin(); it != npcDialog->second.items.end(); it++)
      {
        //Zebra::logger->debug("%u,%u,%u,%u,%u",it->second.id,it->second.kind,it->second.lowLevel,it->second.level,it->second.action);
        switch(item.action & it->second.action)
        {
          case NPC_BUY_OBJECT:
          case NPC_SELL_OBJECT:
          case NPC_REPAIR_OBJECT:
          case NPC_MAKE_OBJECT:
          case NPC_UPDATE_OBJECT:
          case NPC_MERGE_OBJECT :
          case NPC_ENCHANCE_OBJECT :
          case NPC_MERGE_SOUL_OBJECT:
          case NPC_HOLE_OBJECT :
          case NPC_STORE_OBJECT :
		  case  NPC_ZSMAKE_OBJECT: //ת�����
	      case NPC_ZSUPDATE_OBJECT: //ת�����
          case NPC_DECOMPOSE_OBJECT:
            
            //idΪ���ʾ�����κ���Ʒ,kindΪ���ʾ�κ�����
            if (it->second.id!=0)
            {
              if (item.id==it->second.id && (item.itemlevel==it->second.itemlevel || it->second.itemlevel==0))
              {
                rwlock.unlock();
                return true;
              }
            }
            else
            {
              if ((item.kind==it->second.kind || it->second.kind==0)
                  && item.level<=it->second.level && item.level>=it->second.lowLevel)
              {
                rwlock.unlock();
                return true;
              }
            }
            break;
        }
      }
      //rwlock.unlock();
    }
  }
  rwlock.unlock();

  return false;
}

/**
 * \brief ж��npctrade
 *
 */
void NpcTrade::final()
{
  rwlock.wrlock();
  dialogs.clear();
  rwlock.unlock();
}

