#include "GatewayServer.h"


/**
* \brief ���ݹ���id�õ�������Ϣ
*
*
* \param country_id: ����id
* \return ������Ϣ
*/
CountryInfo::Info *CountryInfo::getInfo(DWORD country_id)
{
	for(StrVec_iterator iter = country_info.begin() ; iter != country_info.end() ; iter ++)
	{
		if (iter->countryid == country_id)
		{
			return &*iter;
		}
	}
	return NULL;
}
/**
* \brief �������ļ��ж�ȡ������Ϣ
*
* \return ��ȡ�Ƿ�ɹ�
*/
bool CountryInfo::reload()
{
	zXMLParser parser;
	if (parser.initFile(Zebra::global["confdir"] + "scenesinfo.xml"))
	{
		xmlNodePtr root=parser.getRootNode("ScenesInfo");
		xmlNodePtr countryNode=parser.getChildNode(root,"countryinfo");
		if (countryNode)
		{
			xmlNodePtr subnode = parser.getChildNode(countryNode,"country");
			while(subnode)
			{
				if (strcmp((char*)subnode->name,"country") == 0)
				{
					CountryDic info;
					bzero(&info,sizeof(info));
					parser.getNodePropNum(subnode,"id",&info.id,sizeof(info.id));
					parser.getNodePropStr(subnode,"name",info.name,sizeof(info.name));
					parser.getNodePropNum(subnode,"mapID",&info.mapid,sizeof(info.mapid));
					parser.getNodePropNum(subnode,"function",&info.function,sizeof(info.function));
					Zebra::logger->info("���¶�ȡ��������(%d,%s,%d,%d)",info.id,info.name,info.mapid,info.function);
					CountryMap_iter iter = country_dic.find(info.id);
					if (iter !=country_dic.end())
					{
						Zebra::logger->debug("reload�����ֵ�ɹ�%s",info.name);
						iter->second.function=info.function;
					}
					for(StrVec_iterator iter_1 = country_info.begin() ; iter_1 != country_info.end() ; iter_1 ++)
					{
						if (info.id == (*iter_1).countryid)
						{
							Zebra::logger->debug("reload������Ϣ�ɹ�%s",info.name);
							(*iter_1).function=info.function;
							break;
						}
					}
				}
				subnode = parser.getNextNode(subnode,NULL);
			}
		}
	}
	return true;
}
/**
* \brief �������ļ��ж�ȡ������Ϣ
*
* \return ��ȡ�Ƿ�ɹ�
*/
bool CountryInfo::init()
{
	bool inited = false;
	zXMLParser parser;

	if (parser.initFile(Zebra::global["confdir"] + "scenesinfo.xml"))
	{
		xmlNodePtr root=parser.getRootNode("ScenesInfo");
		xmlNodePtr mapNode=parser.getChildNode(root,"mapinfo");
		if (mapNode)
		{
			xmlNodePtr subnode = parser.getChildNode(mapNode,"map");
			while(subnode)
			{
				if (strcmp((char*)subnode->name,"map") == 0)
				{
					MapDic info;
					bzero(&info,sizeof(info));
					parser.getNodePropNum(subnode,"mapID",&info.id,sizeof(info.id));
					parser.getNodePropStr(subnode,"name",info.name,sizeof(info.name));
					parser.getNodePropStr(subnode,"fileName",info.filename,sizeof(info.filename));
					parser.getNodePropNum(subnode,"backto",&info.backto,sizeof(info.backto));
					Zebra::logger->info("���ص�ͼ����(%d,%s,%s,%d)",info.id,info.name,info.filename,info.backto);
					map_dic.insert(MapMap_value_type(info.id,info));
				}
				subnode = parser.getNextNode(subnode,NULL);
			}
		}
		xmlNodePtr countryNode=parser.getChildNode(root,"countryinfo");
		if (countryNode)
		{
			country_info.clear();
			xmlNodePtr subnode = parser.getChildNode(countryNode,"country");
			while(subnode)
			{
				if (strcmp((char*)subnode->name,"country") == 0)
				{
					CountryDic info;
					bzero(&info,sizeof(info));
					parser.getNodePropNum(subnode,"id",&info.id,sizeof(info.id));
					parser.getNodePropStr(subnode,"name",info.name,sizeof(info.name));
					parser.getNodePropNum(subnode,"mapID",&info.mapid,sizeof(info.mapid));
					parser.getNodePropNum(subnode,"function",&info.function,sizeof(info.function));
					parser.getNodePropNum(subnode,"type",&info.type,sizeof(info.type));
					Zebra::logger->info("���ع�������(%d,%s,%d,%d,%d)",info.id,info.name,info.mapid,info.function,info.type);
					country_dic.insert(CountryMap_value_type(info.id,info));
					Info info_1;
					info_1.countryid = info.id;
					info_1.countryname = info.name;
					info_1.function = info.function;
					info_1.type     = info.type;
					if (info.mapid)
					{
						MapMap_iter map_iter = map_dic.find(info.mapid);
						if (map_iter == map_dic.end())
						{
							Zebra::logger->error("�õ�%d��ͼ����ʧ��",info.mapid);
							continue;
						}
						inited = true;
						info_1.mapname = info_1.countryname + "��" + map_iter->second.name;
					}
					country_info.push_back(info_1);
				}
				subnode = parser.getNextNode(subnode,NULL);
			}
		}
	}
	if (inited)
	{
		for(StrVec_iterator iter = country_info.begin() ; iter != country_info.end() ; iter++)
		{
			Zebra::logger->info("��ȡ������Ϣ:%s(%d),%s",
				(*iter).countryname.c_str(),(*iter).countryid,(*iter).mapname.c_str());
		}
	}
	return inited;
}

/**
* \brief ���ù�������
*
* \param ptCmd ����ָ��
*/
void CountryInfo::setCountryOrder(Cmd::Session::CountrOrder *ptCmd)
{
	mutex.lock(); 
	bzero(country_order,sizeof(country_order));
	for(int i = 0 ; i < (int)ptCmd->size ; i++)
	{
		country_order[i]=ptCmd->order[i].country;
	}
	mutex.unlock(); 
}
// [ranqd Add] ��ȡ��������״̬
int CountryInfo::getCountryState( CountryInfo::Info cInfo )
{
	DWORD now = cInfo.Online_Now;
	DWORD max = cInfo.Online_Max;
	if( max == 0 )				return STATE_SERVICING;
	if( now == max + 10000 )	return STATE_SERVICING;
	if( now < max * 0.7 )		return STATE_NOMARL;
	if( now < max * 0.9 )		return STATE_GOOD;
	if( now < max )				return STATE_BUSY;
	if( now >= max )			return STATE_FULL;
}
/**
* \brief �õ����й�������
*
*
* \param buf:����������Ƶ�buf
* \return ��ȡ���Ĺ�������
*/
// [ranqd] �����Ч����ID��100����Ϊս��ʹ��
#define MAX_VALUE_COUNTRYID 100

int CountryInfo::getAll(char *buf)
{
	int size = 0;
	if (!buf)
	{
		return size;
	}
	mutex.lock(); 
	Cmd::Country_Info *info = (Cmd::Country_Info*)buf;
	for(StrVec_iterator iter = country_info.begin() ; iter != country_info.end() ; iter++)
	{
		bool ok=true;
		for(int i = 0; country_order[i] != 0; i ++)
		{
			if ((*iter).countryid == country_order[i])
			{
				ok=false;
				break;
			}
		}
		if (ok && (*iter).countryid < MAX_VALUE_COUNTRYID )
		{
			//���������ҿ���ע��
			//if (!((*iter).function & 0x1))
			//{
			info[size].id = (*iter).countryid;
			info[size].enableLogin = ((*iter).function & 0x2)==0?1:0;
			info[size].enableRegister = ((*iter).function & 0x1)==0?1:0;
			info[size].Online_Statue  = getCountryState((*iter));
			info[size].type           = (*iter).type;
			bcopy((*iter).countryname.c_str(),info[size].pstrName,MAX_NAMESIZE,sizeof(info[size].pstrName));
			size++;
			//}
		}
	}
	for(int i = 0; country_order[i] != 0; i ++)
	{
		for(StrVec_iterator iter = country_info.begin() ; iter != country_info.end() ; iter++)
		{
			if ((*iter).countryid < MAX_VALUE_COUNTRYID && (*iter).countryid == country_order[i])
			{
				//���������ҿ���ע��
				//if (!((*iter).function & 0x1))
				//{
				info[size].id = (*iter).countryid;
				info[size].enableLogin = ((*iter).function & 0x2)==0?1:0;
				info[size].enableRegister = ((*iter).function & 0x1)==0?1:0;
				info[size].Online_Statue  = getCountryState((*iter));
				info[size].type           = (*iter).type;
				bcopy((*iter).countryname.c_str(),info[size].pstrName,MAX_NAMESIZE,sizeof(info[size].pstrName));
				size++;
				break;
				//}
			}
		}
	}
	mutex.unlock(); 
	return size;
}
/**
* \brief �õ���������
*
* \return ��������
*/
int CountryInfo::getCountrySize()
{
	return country_info.size();
}
/**
* \brief ����hash���mapid�õ������ļ��б����mapid
*
* \param map_id: hash���mapid
* \return �����ļ��е�mapid
*/
DWORD CountryInfo::getRealMapID(DWORD map_id)
{
	return map_id & 0x0000FFFF;
}
/**
* \brief ����hash��ĵ�ͼ���Ƶõ������ļ��еĵ�ͼ����
*
* \param name: hash��ĵ�ͼ����
* \return �����ļ��еĵ�ͼ����
*/
const char *CountryInfo::getRealMapName(const char *name)
{
	char *real = const_cast<char*>(strstr(name,"��"));
	if (real != NULL)
	{
		return real + 2;
	}
	else
	{
		return name;
	}
}
/**
* \brief ������id�Ƿ�Ϸ�
*
*
* \param country_id:����id
* \return ������ڸù���id���ع���id,���򷵻�-1
*/
DWORD CountryInfo::getCountryID(DWORD country_id)
{
	Info *info = getInfo(country_id);
	if (info)
	{
		return info->countryid;
	}
	return (DWORD)-1;
}
/**
* \brief ���ݹ���id�õ���������
*
*
* \param country_id:����id
* \return �ҵ����ع������Ʒ��򷵻�""
*/
std::string CountryInfo::getCountryName(DWORD country_id)
{
	Info *info = getInfo(country_id);
	if (info)
	{
		return info->countryname;
	}
	return "";
}
/**
* \brief ���߹���id�õ��ù��ҳ�����map������
*
*
* \param country_id:��������
* \return �����ص�ͼ����
*/
std::string CountryInfo::getMapName(DWORD country_id)
{
	Info *info = getInfo(country_id);
	if (info)
	{
		return info->mapname;
	}
	return "";
}

/**
* \brief ���ݹ���id�õ��ù����Ƿ������½
* \param country_id:��������
* \return true���� false������
*/
bool CountryInfo::isEnableLogin(DWORD country_id)
{
	Info *info = getInfo(country_id);
	if (info)
	{
		return !(info->function&0x2);
	}
	return false;
}

/**
* \brief ���ݹ���id�õ��ù����Ƿ�����ע��
* \param country_id:��������
* \return true���� false������
*/
bool CountryInfo::isEnableRegister(DWORD country_id)
{
	Info *info = getInfo(country_id);
	if (info)
	{
		return !(info->function&0x1);
	}
	return false;
}

/**
* \brief ���ݹ���ID����Թ���function��״̬���
* \param country_id:��������
*/
void CountryInfo::processChange(GateUser *pUser,Cmd::Scene::t_ChangeCountryStatus *rev)
{
	if (!rev||!pUser) return;
	switch(rev->oper)
	{
	case Cmd::Scene::ENABLE_REGISTER:
		{
			Info *info = getInfo(rev->country);
			if (info)
			{
				info->function = info->function&(~0x1);
			}
		}
		break;
	case Cmd::Scene::DISABLE_REGISTER:
		{
			Info *info = getInfo(rev->country);
			if (info)
			{
				info->function = info->function|0x1;
			}
		}
		break;
	case Cmd::Scene::ENABLE_LOGIN:
		{
			Info *info = getInfo(rev->country);
			if (info)
			{
				info->function = info->function&(~0x2);
			}
		}
		break;
	case Cmd::Scene::DISABLE_LOGIN:
		{
			Info *info = getInfo(rev->country);
			if (info)
			{
				info->function = info->function|0x2;
			}
		}
		break;
	case Cmd::Scene::LIST_COUNTRY_INFO:
		{
			for(StrVec_iterator iter = country_info.begin() ; iter != country_info.end() ; iter++)
			{
				Cmd::stChannelChatUserCmd send;
				send.dwType=Cmd::CHAT_TYPE_SYSTEM;
				send.dwSysInfoType = Cmd::INFO_TYPE_GAME;
				bzero(send.pstrName,sizeof(send.pstrName));
				bzero(send.pstrChat,sizeof(send.pstrChat));

				if (pUser)
				{
					sprintf((char*)send.pstrChat,"����:%s ID:%d ע��:%s ��¼:%s",
						(*iter).countryname.c_str(),(*iter).countryid,(((*iter).function&0x1)==0)?"��":"��",(((*iter).function&0x2)==0)?"��":"��");
					pUser->sendCmd(&send,sizeof(send));
				}
			}
		}
		break;
	default:
		break;
	}
}
// [ranqd] ���¹�������״̬
void CountryInfo::UpdateCountryOnline( DWORD country_id, DWORD online_numbers )
{
	for(StrVec_iterator iter = country_info.begin() ; iter != country_info.end() ; iter++)
	{
		if( (*iter).countryid == country_id )
		{
			(*iter).Online_Now = online_numbers;
			return;
		}
	}
}