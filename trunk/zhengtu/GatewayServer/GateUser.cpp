/**
* \brief ʵ�������û���
*/

#include "GatewayServer.h"

GateUser::GateUser(DWORD accID,GatewayTask *histask):zUser(),GateSelectUserSession(accID)
{
	initState();
	gatewaytask=histask;
	backSelect=false;
	scene=NULL;
	sceneTempID=0;
	mapScreenIndex=(DWORD)-1;
	hide=false;
	inserted=false;
	quiz = false;
	logout=false; 
	bzero(jpegPassport,sizeof(jpegPassport));
	bzero(sysSetting,sizeof(sysSetting));
	if (gatewaytask)
	{
		gatewaytask->pUser=this;
	}
	dupIndex = 0;
}

GateUser::~GateUser()
{
	//lock();
	//if (gatewaytask)
	//{
	//gatewaytask->pUser=NULL;
	//gatewaytask->Terminate();
	//gatewaytask=NULL;
	//}
	//unlock();
}


/**
* \brief ж��һ�������û�����Ϣ
*
*
*/
void GateUser::final()
{
	lock();
	if (tempid!=0)
	{
		//GateUserManager::getInstance()->getUserByID(this->id);
		if (this->scene && this->isPlayState())
		{
			this->scene->removeIndex(this,sceneTempID);
		}
		GateUserManager::getInstance()->removeUser(this);
		GateUserManager::getInstance()->removeCountryUser(this);
		tempid=0;
	}
	unlock();
	GateUserManager::getInstance()->removeUserOnlyByAccID(accid);
}

/**
* \brief ˢ�½�ɫ��Ϣ
*
*/
void GateUser::refreshCharInfo()
{
	using namespace Cmd;
	BYTE buf[zSocket::MAX_DATASIZE];
	stUserInfoUserCmd *cmd = (stUserInfoUserCmd *)buf;
	constructInPlace(cmd);
	bcopy(userinfo,cmd->charInfo,sizeof(cmd->charInfo),sizeof(cmd->charInfo));
	cmd->size = 1;
	//int size = 0;
	//void *ret = jpeg_Passport(jpegPassport,sizeof(jpegPassport),&size);
	//if (ret)
	//{
	//	if (size >  0 && size <= (int)(zSocket::MAX_DATASIZE - sizeof(stUserInfoUserCmd) - 100))
	//	{
	//		//Zebra::logger->debug("����ͼ����֤�룺%s",jpegPassport);
	//		cmd->size = size;
	//		bcopy(ret,cmd->data,size);
	//	}
	//	free(ret);
	//}
	if (sendCmd(cmd,sizeof(stUserInfoUserCmd) + cmd->size))
	{
		selectState();
	}
}

/**
* \brief ��֤����
*
*
* \param passport: ��֤������
* \return ��֤�Ƿ�ͨ��
*/
bool GateUser::checkPassport(const char *passport)
{
#if 0
	if (0 == strncmp(jpegPassport,passport,sizeof(jpegPassport)))
	{
		Zebra::logger->debug("��֤������ɹ�");
		return true;
	}
	else
	{
		Zebra::logger->error("��֤�����");
		using namespace Cmd;
		stServerReturnLoginFailedCmd cmd;
		cmd.byReturnCode=LOGIN_RETURN_JPEG_PASSPORT;
		if (sendCmd(&cmd,sizeof(cmd)))
		{
			refreshCharInfo();
		}
		return false;
	}
#else
	return true;
#endif
}

/**
* \brief ������Ϸ״̬
*
*
*/
void GateUser::playState(SceneClient *s,DWORD scene_tempid)
{
	lock();
	if (scene_tempid)
	{
		this->sceneTempID=scene_tempid;
	}
	if (s)
	{
		this->scene=s;
		this->scene->freshIndex(this,this->sceneTempID,(DWORD)-1);
	}
	systemstate = SYSTEM_STATE_PLAY;
	unlock();
}
// [ranqd] Add ���û����ͷ�������Ϣ
void GateUser::SendCountryInfo()
{
	const unsigned int _size = GatewayService::getInstance().country_info.getCountrySize()*sizeof(Cmd::Country_Info) + sizeof(Cmd::stCountryInfoUserCmd);
	char *Buf = new char[_size];
	bzero(Buf,sizeof(*Buf));
	Cmd::stCountryInfoUserCmd *ciu = (Cmd::stCountryInfoUserCmd*)Buf;
	constructInPlace(ciu);
	ciu->size = GatewayService::getInstance().country_info.getAll((char *)ciu->countryinfo);
	sendCmd(ciu,sizeof(Cmd::stCountryInfoUserCmd) + ciu->size *sizeof(Cmd::Country_Info));
	delete[] Buf;
	printf("���ͷ������б�\n");
}
/**
* \brief ��ѡ��һ����ɫ���г�ʼ����
*
*
* \return �����Ƿ�ɹ�
*/
bool GateUser::beginSelect()
{
	lock();
	if ( this->isWaitUnregState() )
	{
		initState(); 
	}
	if (!logout && isInitState())
	{
		//���û����������
		if (GateUserManager::getInstance()->addUserOnlyByAccID(this))
		{
			//��RecordServer��������ѡ����Ϣ
			Cmd::Record::t_Get_SelectInfo_GateRecord send;
			send.accid=accid;
			send.countryid = CountryID;

			if (recordClient->sendCmd(&send,sizeof(send)))
			{
				unlock();
				return true;
			}
			else
			{
				GateUserManager::getInstance()->removeUserOnlyByAccID(accid);
				unlock();
				return false;
			}
		}
		Zebra::logger->error("����ʺ�(%ld)ʧ��",accid);


		//��ʱ���û���������Ч,Ҳ�����û��Ѿ��ڱ�����ص�½,���ܶ�Sesion�ͳ�������ж��(����down��һ��,ԭ��δ֪)
		/*
		GateUser *pUser = GateUserManager::getInstance()->getUserByAccID(accid);
		if (pUser)
		{
		if (sessionClient)
		{
		//����Session������
		Cmd::Session::t_unregUser_GateSession send;
		send.dwUserID=pUser->id;
		send.dwSceneTempID=pUser->sceneTempID;
		send.retcode=Cmd::Session::UNREGUSER_RET_ERROR;
		sessionClient->sendCmd(&send,sizeof(send));
		}
		if (pUser->scene)
		{
		//����Scene������
		Cmd::Scene::t_Unreg_LoginScene scnd;
		scnd.dwUserID=pUser->id;
		scnd.dwSceneTempID=pUser->sceneTempID;
		scnd.retcode=Cmd::Scene::UNREGUSER_RET_ERROR;
		pUser->scene->sendCmd(&scnd,sizeof(scnd));
		}
		}
		// */
	}
	unlock();
	return false;
}

/**
* \brief ��ʼ��Ϸ
*
*
* \return true
*/
bool GateUser::beginGame()
{
	return true;
}


/**
* \brief ָ�����
*
*
* \return true
*/
void GateUser::cmdFilter(Cmd::stNullUserCmd *cmd,DWORD &type,char *name,DWORD &cmdLen)
{
	using namespace Cmd;
	switch (cmd->byCmd)
	{
	case CHAT_USERCMD:
		if (ALL_CHAT_USERCMD_PARAMETER == cmd->byParam)
		{
			Cmd::stChannelChatUserCmd *rev = (Cmd::stChannelChatUserCmd *)cmd;

			type=rev->dwType;
			strncpy(name,rev->pstrName,MAX_NAMESIZE);
			/**
			* \brief �������ָ����ѹ����ָ��
			* ������ʡѹ���ͼ���
			*
			*/
			BYTE buf[zSocket::MAX_DATASIZE];
			Cmd::stServerChannelChatUserCmd *ret = (Cmd::stServerChannelChatUserCmd *)buf;
			constructInPlace(ret);
			ret->dwType=rev->dwType;
			ret->dwSysInfoType=rev->dwSysInfoType;
			ret->dwCharType=rev->dwCharType;
			ret->dwChannelID=rev->dwChannelID;
			ret->dwFromID=rev->dwFromID;
			ret->dwChatTime=rev->dwChatTime;
			ret->size=rev->size;
			int nameLen=strlen(rev->pstrName)+1;
			int chatLen=strlen(rev->pstrChat)+1;
			if (nameLen>1)
				strncpy(ret->info,rev->pstrName,nameLen);
			else
				bzero((void*)ret->info,1);
			if (chatLen>1)
			{
				bcopy(rev->pstrChat,ret->info+nameLen,chatLen, sizeof(buf) - sizeof(Cmd::stServerChannelChatUserCmd) - nameLen );
			}
			else
				bzero((void*)(ret->info+nameLen),1);
			char *temp =ret->info + nameLen + chatLen;
			if (rev->size)
			{
				bcopy(rev->tobject_array,temp,rev->size * sizeof(stTradeObject), sizeof(buf) - sizeof(Cmd::stServerChannelChatUserCmd) - nameLen - chatLen);
			}
			cmdLen = cmdLen - (MAX_NAMESIZE - nameLen) - (MAX_CHATINFO - chatLen);
			bcopy(ret,cmd,cmdLen,zSocket::MAX_DATASIZE);
		}
		break;
	case RELATION_USERCMD:
		if (RELATION_STATUS_PARA == cmd->byParam)
		{

			stRelationStatusCmd *rev = (stRelationStatusCmd *)cmd;

			if (rev->byState == Cmd::RELATION_QUESTION)
			{
				strncpy(name,rev->name,MAX_NAMESIZE);
			}
		}
		break;
	default:
		break;
	}
}
/**
* \brief �������ݸ��ͻ���
*
*
* \param pstrCmd: ���͵�ָ��
* \param nCmdLen: ָ���
* \return �����Ƿ�ɹ�
*/
bool GateUser::sendCmd(const void *pstrCmd,const DWORD nCmdLen,const DWORD type,const char *strName,const bool hasPacked)
{
	if (!hasPacked)
	{
		DWORD cmdLen=nCmdLen;
		using namespace Cmd;
		stNullUserCmd *cmd = (stNullUserCmd *)pstrCmd;
		switch (cmd->byCmd)
		{
		case CHAT_USERCMD:
			if (ALL_CHAT_USERCMD_PARAMETER == cmd->byParam)
			{
				stChannelChatUserCmd *rev = (stChannelChatUserCmd *)pstrCmd;
				/**
				* \brief �������ָ����ѹ����ָ��
				* ������ʡѹ���ͼ���
				*
				*/
				BYTE buf[zSocket::MAX_DATASIZE];//={0};
				stServerChannelChatUserCmd *ret = (stServerChannelChatUserCmd *)buf;
				constructInPlace(ret);
				ret->dwType=rev->dwType;
				ret->dwSysInfoType=rev->dwSysInfoType;
				ret->dwCharType=rev->dwCharType;
				ret->dwChannelID=rev->dwChannelID;
				ret->dwFromID=rev->dwFromID;
				ret->dwChatTime=rev->dwChatTime;
				ret->size=rev->size;
				int nameLen=strlen(rev->pstrName)+1;
				int chatLen=strlen(rev->pstrChat)+1;
				if (nameLen>1)
					strncpy(ret->info,rev->pstrName,nameLen);
				else
					bzero((void*)ret->info,1);
				if (chatLen>1)
				{
					bcopy(rev->pstrChat,ret->info+nameLen,chatLen,sizeof(buf) - sizeof(stServerChannelChatUserCmd) - nameLen);
				}
				else
					bzero(ret->info+nameLen,1);
				char *temp =ret->info + nameLen + chatLen;
				if (rev->size)
				{
					bcopy(rev->tobject_array,temp,rev->size * sizeof(stTradeObject),sizeof(buf) - sizeof(stServerChannelChatUserCmd) - nameLen - chatLen);
				}
				cmdLen = cmdLen - (MAX_NAMESIZE - nameLen) - (MAX_CHATINFO - chatLen);
				//ϵͳ���ü��
				if (!this->checkChatCmd(rev->dwType,rev->pstrName)) return true; 
				if (gatewaytask)
				{
					return gatewaytask->sendCmd(ret,cmdLen);
				}
			}
			break;
		case RELATION_USERCMD:
			if (RELATION_STATUS_PARA == cmd->byParam)
			{
				stRelationStatusCmd *rev = (stRelationStatusCmd *)pstrCmd;
				std::set<std::string>::iterator sIterator;
				if (rev->byState == Cmd::RELATION_QUESTION)
				{
					rwlock.rdlock();
					sIterator = blacklist.find(rev->name);
					rwlock.unlock();
					if (sIterator != blacklist.end()) return true;
				}
			}
			break;
		default:
			break;
		}
		if (gatewaytask)
		{
			return gatewaytask->sendCmd(pstrCmd,cmdLen);
		}
	}
	else
	{
		if (type)
		{
			if (!this->checkChatCmd(type,strName)) return true;
		}
		else if (strName[0])
		{
			std::set<std::string>::iterator sIterator;

			rwlock.rdlock();
			sIterator = blacklist.find(strName);
			rwlock.unlock();
			if (sIterator != blacklist.end()) return true;
		}
		if (gatewaytask)
		{
			return gatewaytask->sendCmdNoPack(pstrCmd,nCmdLen);
		}
	}
	return false;
}

/**
* \brief ����ע��һ���û�
*
*/
void GateUser::unreg(bool out)
{
	//��ֹ���ӵ�̫��
	bool need=false;
	lock();
	if (out)
	{
		logout=true;
	}
	if (isWaitPlayState())
	{
	}
	else if ( isPlayState() )
	{
		need=true;
		this->waitUnregState();
		if (this->scene)
		{
			this->scene->removeIndex(this,sceneTempID);
		}
		Zebra::logger->info("ע��%s(%ld)",name,id);
		//*
	}
	unlock();
	if (need)
	{
		Cmd::Session::t_unregUser_GateSession send;
		send.dwUserID=id;
		send.dwSceneTempID=sceneTempID;
		send.retcode=Cmd::Session::UNREGUSER_RET_LOGOUT;
		sessionClient->sendCmd(&send,sizeof(send));
	}
	dupIndex = 0;
	/*/
	Cmd::Scene::t_Unreg_LoginScene send;
	send.dwUserID=id;
	send.dwSceneTempID=sceneTempID;
	scene->sendCmd(&send,sizeof(send));
	// */
}

/**
* \brief ����ע��һ���û�
*
*
* \param charno: ��ɫ���
*/
void GateUser::reg(int charno)
{
	dupIndex = 0;
	Cmd::Session::t_regUser_GateSession send;

	send.accid=accid;
	send.dwID=id;
	send.dwTempID=tempid;
	send.dwMapID=userinfo[charno].mapid ;
	send.wdLevel = userinfo[charno].level;
	send.wdOccupation = userinfo[charno].face;  
	send.wdCountry = userinfo[charno].country;
	strncpy(send.OldMap, userinfo[charno].OldMap, MAX_PATH);		//sky �ϵ�ͼλ��

	Zebra::logger->fatal("reg OldMap:%s", send.OldMap);

	strncpy((char*)send.byCountryName,userinfo[charno].countryName,sizeof(send.byCountryName));
	strncpy((char *)send.byName,name,MAX_NAMESIZE);
	strncpy((char *)send.byMapName,userinfo[charno].mapName,MAX_NAMESIZE);
	waitPlayState();

	sessionClient->sendCmd(&send,sizeof(send));
}

/**
* \brief ��һ����ɫ��ӵ�������
*
*
* \param name: ��ɫ����
*/
void GateUser::addBlackList(const char *name)
{
	rwlock.wrlock();
	blacklist.insert(blackListValueType(name));
	rwlock.unlock();
}

/**
* \brief ��һ����ɫ�Ӻ�������ɾ��
*
*
* \param name: ��ɫ����
*/
void GateUser::removeBlackList(const char *name)
{
	std::set<std::string>::iterator sIterator;
	rwlock.wrlock();
	sIterator = blacklist.find(name);
	if (sIterator != blacklist.end()) blacklist.erase(sIterator);
	rwlock.unlock();
}

/**
* \brief �ж�����
*
*/
void GateUser::Terminate()
{
	if (gatewaytask)
		gatewaytask->TerminateWait();
}

/**
* \brief ��ȡ�ʺ�
*
*/
const char* GateUser::getAccount()
{
	if (gatewaytask)
		return gatewaytask->account;
	return NULL;
}
bool GateUser::forwardSceneBill(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	if (gatewaytask)
	{
		return gatewaytask->forwardSceneBill(pNullCmd,nCmdLen);
	}
	return false;
}
bool GateUser::forwardScene(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	if (gatewaytask)
	{
		return gatewaytask->forwardScene(pNullCmd,nCmdLen);
	}
	return false;
}
bool GateUser::forwardBillScene(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	if (gatewaytask)
	{
		return gatewaytask->forwardBillScene(pNullCmd,nCmdLen);
	}
	return false;
}
bool GateUser::forwardBill(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	if (gatewaytask)
	{
		return gatewaytask->forwardBill(pNullCmd,nCmdLen);
	}
	return false;
}
void GateUser::setVip(bool b)
{
	if (gatewaytask)
	{
		gatewaytask->setVip(b);
	}
}
void GateSelectUserSession::setSelectUserInfo(const Cmd::Record::t_Ret_SelectInfo_GateRecord *ptCmd)
{
	bcopy(ptCmd->info,userinfo,sizeof(userinfo),sizeof(userinfo));
	for(int i = 0; i < Cmd::MAX_CHARINFO; i++)
	{
		CountryInfo::Info *info = GatewayService::getInstance().country_info.getInfo(userinfo[i].country);
		if (info)
		{
			strncpy(userinfo[i].countryName, info->countryname.c_str(), sizeof(userinfo[i].countryName));
		}
	}
}
/**
* \brief �����û�ѡ��Ľ�ɫ
*
*
* \param info: ѡ��Ľ�ɫ��Ϣ
*/
void GateSelectUserSession::putSelectUserInfo(const Cmd::SelectUserInfo &info)
{
	for(int i = 0; i < Cmd::MAX_CHARINFO; i++)
	{
		if (userinfo[i].id==0 || userinfo[i].id==(DWORD)-1)
		{
			bcopy(&info,&userinfo[i],sizeof(info),sizeof(info));
			CountryInfo::Info *info = GatewayService::getInstance().country_info.getInfo(userinfo[i].country);
			if (info)
			{
				strncpy(userinfo[i].countryName,info->countryname.c_str(),sizeof(userinfo[i].countryName));
			}
			Zebra::logger->info("id=%d name=%s level=%d mapName=%s",
				userinfo[i].id,userinfo[i].name,userinfo[i].level,userinfo[i].mapName);
			break;
		}
	}
}
bool GateUser::checkChatCmd(DWORD type,const char *strName)
{               
	using namespace Cmd;
	switch (type)
	{
	case CHAT_TYPE_NINE:
		//case CHAT_TYPE_SHOPADV:
		{
			if (!isset_state(sysSetting,USER_SETTING_CHAT_NINE))
				return false;
		}
		break;  
		/*
		case CHAT_TYPE_PRIVATE: 
		case CHAT_TYPE_FRIEND_PRIVATE:
		case CHAT_TYPE_UNION_PRIVATE:
		case CHAT_TYPE_OVERMAN_PRIVATE:
		case CHAT_TYPE_FAMILY_PRIVATE:
		{
		if (!isset_state(sysSetting,USER_SETTING_CHAT_PRIVATE))
		return false;
		}
		break;
		*/
	case CHAT_TYPE_UNION_AFFICHE: 
	case CHAT_TYPE_UNION:
		{
			if (!isset_state(sysSetting,USER_SETTING_CHAT_UNION))
				return false;
		}
		break;
	case CHAT_TYPE_OVERMAN_AFFICHE:
	case CHAT_TYPE_OVERMAN:
		{
			if (!isset_state(sysSetting,USER_SETTING_CHAT_SCHOOL))
				return false;
		}
		break;
	case CHAT_TYPE_FAMILY_AFFICHE:
	case CHAT_TYPE_FAMILY:
		{
			if (!isset_state(sysSetting,USER_SETTING_CHAT_FAMILY))
				return false;
		}
		break;
	case CHAT_TYPE_FRIEND_AFFICHE:
	case CHAT_TYPE_FRIEND:
		{
			if (!isset_state(sysSetting,USER_SETTING_CHAT_FRIEND))
				return false;
		}
		break;
	case CHAT_TYPE_COUNTRY:
		{
			if (!isset_state(sysSetting,USER_SETTING_CHAT_COUNTRY))
				return false;
		}
		break;
	case CHAT_TYPE_TEAM:
		{
			if (!isset_state(sysSetting,USER_SETTING_CHAT_TEAM))
				return false;
		}
		break;
	case CHAT_TYPE_WHISPER:
		{
			if (!isset_state(sysSetting,USER_SETTING_CHAT_WHISPER))
				return false;
		}
		break;
	case CHAT_TYPE_AREA:
		{
			if (!isset_state(sysSetting,USER_SETTING_CHAT_AREA))
				return false;
		}
		break;
	case CHAT_TYPE_WORLD:
		{
			if (!isset_state(sysSetting,USER_SETTING_CHAT_WORLD))
				return false;
		}
		break;
	default:
		break;
	}
	// �����������Ϣ
	std::set<std::string>::iterator sIterator;

	rwlock.rdlock();
	sIterator = blacklist.find(strName);
	rwlock.unlock();
	if (sIterator != blacklist.end()) return false;
	return true;
}
