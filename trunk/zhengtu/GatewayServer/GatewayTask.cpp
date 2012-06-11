/**
* \brief 定义登陆连接任务
*
*/

#include "GatewayServer.h"
#include <zebra/csTurn.h>

extern DWORD merge_version;

#define checkTimeInterval 30000
/**
* \brief 构造函数
*
* \param pool 所属连接池指针
* \param sock TCP/IP套接口
* \param addr 地址
*/
GatewayTask::GatewayTask(
						 zTCPTaskPool *pool,
						 const SOCKET sock,
						 const struct sockaddr_in *addr) :
zTCPTask(pool,sock,addr,true,false),
//v_lastSampleTime(GatewayTimeTick::currentTime),
_retset_gametime(3600),
initTime(),
lastCheckTime(GatewayTimeTick::currentTime)
{

	Zebra::logger->debug("GatewayTask::GatewayTask");
	recycle_wait = 5000;
	//v_lastSampleTime.addDelay(sampleInterval);
	v_lastSampleTime=0;
	lastCheckTime.addDelay(checkTimeInterval);
	haveCheckTime = true;
	v_samplePackets = 0;
	accid = 0;
	loginTempID = 0;
	versionVerified = false;
	dwTimestampServer=0;
	qwGameTime=0;
	accountVerified = ACCOUNTVERIFY_NONE;
	pUser=NULL;
	vip_user=true;
	nextChatTime.now();
	nextCountryChatTime.now();
	bzero(numPassword,sizeof(numPassword));
	numPwd = 0;

	mSocket->setEncMethod(CEncrypt::ENCDEC_RC5);
	//mSocket->enc.set_key_rc5((const BYTE *)Zebra::global["rc5_key"].c_str(),16,12);
	BYTE key[16] = {28,196,25,36,193,125,86,197,35,92,194,41,31,240,37,223};
	mSocket->set_key_rc5((const BYTE *)key,16,12);
}

/**
* \brief 析构函数
*
*/
GatewayTask::~GatewayTask()
{
	if (pUser)
	{
		//pUser->lock();
		//pUser->gatewaytask=NULL;
		//if (pUser->isWaitPlayState() || pUser->isPlayState())
		//{
		//  pUser->unreg();
		//pUser->unlock();
		//}
		//else
		//{
		//pUser->unlock();
		//  pUser->final();
		//}
		// */
		SAFE_DELETE(pUser);
	}
}

/**
* \brief 验证版本号
* \param pNullCmd 待验证的指令
* \return 验证版本号码是否成功
*/
bool GatewayTask::verifyVersion(const Cmd::stNullUserCmd *pNullCmd)
{
	Zebra::logger->debug("GatewayTask::verifyVersion");
	using namespace Cmd;
	if (LOGON_USERCMD == pNullCmd->byCmd
		&& USER_VERIFY_VER_PARA == pNullCmd->byParam)
	{
		stUserVerifyVerCmd *ptCmd = (stUserVerifyVerCmd *)pNullCmd;
		if (GatewayService::getInstance().verify_client_version == 0 || ptCmd->version >= GatewayService::getInstance().verify_client_version)
		{
			Zebra::logger->debug("客户端连接通过版本号验证[%u]",ptCmd->version);
			return true;
		}
		else
		{
			Zebra::logger->debug("客户端连接没有通过版本号验证[%u][%u]",ptCmd->version,GatewayService::getInstance().verify_client_version);
			return false;
		}
	}

	Zebra::logger->error("客户端连接没有通过版本号验证");
	return false;
}

/**
* \brief 验证登陆网关指令
* 验证登陆网关的指令
* \param pNullCmd 待验证的登陆指令
* \return 验证登陆指令是否成功
*/
bool GatewayTask::verifyACCID(const Cmd::stNullUserCmd *pNullCmd)
{
	Zebra::logger->debug("GatewayTask::verifyACCID");
	using namespace Cmd;
	if (LOGON_USERCMD == pNullCmd->byCmd
		&& PASSWD_LOGON_USERCMD_PARA == pNullCmd->byParam)
	{
		stPasswdLogonUserCmd *ptCmd = (stPasswdLogonUserCmd *)pNullCmd;
		Zebra::logger->info("用户登录：%u %u",ptCmd->loginTempID,ptCmd->dwUserID);
		ZES_cblock des_key;
		if (LoginSessionManager::getInstance().verify(ptCmd->loginTempID,ptCmd->dwUserID,numPassword,&des_key))
		{
			accid = ptCmd->dwUserID;
			loginTempID = ptCmd->loginTempID;
			strncpy(account,ptCmd->pstrName,MAX_ACCNAMESIZE);
			if (numPassword[0])
			{
				numPwd = atoi(numPassword);
				Zebra::logger->debug("[财保]: %s 数字密码为: %u",account,numPwd);
			}

			//设置zSocket的加密密钥
			mSocket->setEncMethod(CEncrypt::ENCDEC_DES);
			mSocket->set_key_des(&des_key);

			Cmd::stMergeVersionCheckUserCmd send;
			send.dwMergeVersion = merge_version;
			sendCmd(&send,sizeof(send));

			//Zebra::logger->debug("设置des密钥：%u %u %u %u %u %u %u %u",des_key[0],des_key[1],des_key[2],des_key[3],des_key[4],des_key[5],des_key[6],des_key[7]);

			//Zebra::logger->debug("客户端连接同步验证成功,等待计费服务器返回");
			return true;
		}
	}

	Zebra::logger->error("客户端连接同步验证失败");
	return false;
}

/**
* \brief 连接确认
*
* \return 如果没有数据返回0,验证成功返回1,验证失败返回-1
*/
int GatewayTask::verifyConn()
{
	Zebra::logger->debug("GatewayTask::verifyConn");
	int retcode = mSocket->recvToBuf_NoPoll();
	if (retcode > 0)
	{
		while(true)
		{
			BYTE pstrCmd[zSocket::MAX_DATASIZE];
			int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd,sizeof(pstrCmd));
			if (nCmdLen <= 0)
				//这里只是从缓冲取数据包,所以不会出错,没有数据直接返回
				return 0;
			else
			{
				using namespace Cmd;

				stNullUserCmd *pNullCmd = (stNullUserCmd *)pstrCmd;
				if (versionVerified)
				{
					if (verifyACCID(pNullCmd))
						return 1;
					else
						return -1;
				}
				else if (verifyVersion(pNullCmd))
				{
					versionVerified = true;
				}
				else
				{
					Zebra::logger->error("客户端连接验证失败");
					return -1;
				}
			}
		}
	}
	else
		return retcode;
}

/**
* \brief 连接同步等待
*
*
* \return 错误返回-1,成功返回1,继续等待返回0
*/
int GatewayTask::waitSync()
{
	switch(accountVerified)
	{
	case ACCOUNTVERIFY_NONE:
		{
			using namespace Cmd::Bill;
			t_LoginVerify_Gateway tCmd;
			tCmd.accid = accid;
			tCmd.loginTempID = loginTempID;
			if (accountClient->sendCmd(&tCmd,sizeof(tCmd)))
				accountVerified = ACCOUNTVERIFY_WAITING;
			else
				return -1;
		}
		break;
	case ACCOUNTVERIFY_WAITING:
		{
			//无限期等待计费验证通过
		}
		break;
	case ACCOUNTVERIFY_SUCCESS:
		{
			return 1;
		}
		break;
	case ACCOUNTVERIFY_FAILURE:
		{
			return -1;
		}
		break;
	}

	return 0;
}

/**
* \brief 用户下线
*
* \return 1:可以回收,2继续延时
*/
int GatewayTask::recycleConn()
{
	/*
	if (recycle_wait == 5000)
	{
	//通知计费服务器用户下线
	using namespace Cmd::Bill;
	t_Logout_Gateway tCmd;

	tCmd.accid = accid;
	tCmd.loginTempID = loginTempID;
	accountClient->sendCmd(&tCmd,sizeof(tCmd));
	}
	// */
	if ((int)recycle_wait > 0)
	{
		recycle_wait -= 200;
		return 0;
	}
	else
	{
		//通知计费服务器用户下线
		using namespace Cmd::Bill;
		t_Logout_Gateway tCmd;

		tCmd.accid = accid;
		tCmd.loginTempID = loginTempID;
		accountClient->sendCmd(&tCmd,sizeof(tCmd));
		// */
		return 1;
	}
}

/**
* \brief 需要主动断开客户端的连接
*
* \param method 连接断开方式
*/
void GatewayTask::Terminate(const TerminateMethod method)
{
	if (pUser)
	{
		Zebra::logger->info("用户Terminate(%ld,%ld,%s)注销,ip=%s",pUser->accid,pUser->id,pUser->name,getIP());

		Cmd::Mini::t_UserLogout_Gateway s;
		s.userID = pUser->id;
		miniClient->sendCmd(&s,sizeof(s));

		if (!isTerminate())
		{
			pUser->unreg(true);
		}
		pUser->final();
	}
	else
	{
		Zebra::logger->warn("注销但用户已经不存在");
	}
	// */
	zTCPTask::Terminate(method);
}
/**
* \brief 新用户登陆
*
*/
void GatewayTask::addToContainer()
{
	GateUser *pUser=new GateUser(this->accid,this);
	if (pUser)
	{
		//发送游戏时间,开始计时
		using namespace Cmd;
		stGameTimeTimerUserCmd cmd;
		cmd.qwGameTime = Zebra::qwGameTime;
		if (!qwGameTime)
		{
			qwGameTime = Zebra::qwGameTime; 
			GameTimeSyn.now();
		}
		sendCmd(&cmd,sizeof(cmd));
		initTime = GatewayTimeTick::currentTime;

		pUser->SendCountryInfo(); // 发送服务器列表

		pUser->selectServerState(); // [ranqd] 设置用户处于选择服务器状态

		//const unsigned int _size = GatewayService::getInstance().country_info.getCountrySize()*sizeof(Cmd::Country_Info) + sizeof(Cmd::stCountryInfoUserCmd);
		//   char *Buf = new char[_size];
		//   bzero(Buf,sizeof(*Buf));
		//   Cmd::stCountryInfoUserCmd *ciu = (Cmd::stCountryInfoUserCmd*)Buf;
		//   constructInPlace(ciu);
		//   ciu->size = GatewayService::getInstance().country_info.getAll((char *)ciu->countryinfo);
		//   pUser->sendCmd(ciu,sizeof(Cmd::stCountryInfoUserCmd) + ciu->size *sizeof(Cmd::Country_Info));

		//[ranqd] 等待用户选择了服务器再发送角色信息
		//if (pUser->beginSelect())
		//{
		//  //通知登陆服务器,网关连接数
		//  GatewayService::getInstance().notifyLoginServer();
		//}

		//delete[] Buf;
	}
}

/**
* \brief 从容器中删除
*
*/
void GatewayTask::removeFromContainer()
{
	//通知登陆服务器,网关连接数
	GatewayService::getInstance().notifyLoginServer();
}

/**
* \brief 添加一个连接线程
*
*
* \return 成功返回ture,否则返回false
*/
bool GatewayTask::uniqueAdd()
{
	return GatewayTaskManager::getInstance().uniqueAdd(this);
}

/**
* \brief 删除一个连接线程
*
*
* \return 成功返回ture,否则返回false
*/
bool GatewayTask::uniqueRemove()
{
	return GatewayTaskManager::getInstance().uniqueRemove(this);
}

/**
* \brief 场景用户将消息转发到Bill
*
*
* \param pNullCmd: 需要转发的指令
* \param nCmdLen: 指令长度
* \return 转发是否成功
*/
bool GatewayTask::forwardBillScene(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	if (pUser)
	{
		BYTE buf[zSocket::MAX_DATASIZE];
		Cmd::Bill::t_Scene_ForwardBill *sendCmd=(Cmd::Bill::t_Scene_ForwardBill *)buf;
		constructInPlace(sendCmd);
		sendCmd->dwAccid=pUser->accid;
		sendCmd->size=nCmdLen;
		bcopy(pNullCmd,sendCmd->data,nCmdLen,sizeof(buf) - sizeof(Cmd::Bill::t_Scene_ForwardBill));
		accountClient->sendCmd(buf,sizeof(Cmd::Bill::t_Scene_ForwardBill)+nCmdLen);
		//Zebra::logger->debug("转发%ld的消息到%ld场景",pUser->id,pUser->sceneTempID);
		return true;
	}
	return false;
}
/**
* \brief 将消息转发到Bill
*
*
* \param pNullCmd: 需要转发的指令
* \param nCmdLen: 指令长度
* \return 转发是否成功
*/
bool GatewayTask::forwardBill(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	if (pUser)
	{
		BYTE buf[zSocket::MAX_DATASIZE];
		Cmd::Bill::t_Bill_ForwardBill *sendCmd=(Cmd::Bill::t_Bill_ForwardBill *)buf;
		constructInPlace(sendCmd);
		sendCmd->dwAccid=pUser->accid;
		sendCmd->size=nCmdLen;
		bcopy(pNullCmd,sendCmd->data,nCmdLen,sizeof(buf) - sizeof(Cmd::Bill::t_Bill_ForwardBill) );
		accountClient->sendCmd(buf,sizeof(Cmd::Bill::t_Bill_ForwardBill)+nCmdLen);
		//Zebra::logger->debug("转发%ld的消息到%ld场景",pUser->id,pUser->sceneTempID);
		return true;
	}
	return false;
}
/**
* \brief 将Bill的消息转发到场景
*
*
* \param pNullCmd: 需要转发的指令
* \param nCmdLen: 指令长度
* \return 转发是否成功
*/
bool GatewayTask::forwardSceneBill(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	if (pUser && pUser->scene)
	{
		BYTE buf[zSocket::MAX_DATASIZE];
		Cmd::Scene::t_Bill_ForwardScene *sendCmd=(Cmd::Scene::t_Bill_ForwardScene *)buf;
		constructInPlace(sendCmd);
		sendCmd->dwUserID=pUser->id;
		sendCmd->dwSceneTempID=pUser->sceneTempID;
		sendCmd->size=nCmdLen;
		bcopy(pNullCmd,sendCmd->data,nCmdLen,sizeof(buf) - sizeof(Cmd::Scene::t_Bill_ForwardScene));
		pUser->scene->sendCmd(buf,sizeof(Cmd::Scene::t_Bill_ForwardScene)+nCmdLen);
		//Zebra::logger->debug("转发%ld的消息到%ld场景",pUser->id,pUser->sceneTempID);
		return true;
	}
	return false;
}

/**
* \brief 将消息转发到小游戏服务器
*
*
* \param pNullCmd: 需要转发的指令
* \param nCmdLen: 指令长度
* \return 转发是否成功
*/
bool GatewayTask::forwardMini(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	if (pUser)
	{
		BYTE buf[zSocket::MAX_DATASIZE];
		Cmd::Mini::t_Mini_UserForwardMini *sendCmd=(Cmd::Mini::t_Mini_UserForwardMini *)buf;
		constructInPlace(sendCmd);
		sendCmd->id=pUser->id;
		sendCmd->size=nCmdLen;
		bcopy(pNullCmd,sendCmd->data,nCmdLen,sizeof(buf) - sizeof(Cmd::Mini::t_Mini_UserForwardMini));
		miniClient->sendCmd(buf,sizeof(Cmd::Mini::t_Mini_UserForwardMini)+nCmdLen);
		Zebra::logger->debug("转发%ld的消息到Mini服务器",pUser->id);
		return true;
	}   
	return false;
}

/**
* \brief 将消息转发到场景
*
*
* \param pNullCmd: 需要转发的指令
* \param nCmdLen: 指令长度
* \return 转发是否成功
*/
bool GatewayTask::forwardScene(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	if (pUser && pUser->scene)
	{
		BYTE buf[zSocket::MAX_DATASIZE];
		Cmd::Scene::t_Scene_ForwardScene *sendCmd=(Cmd::Scene::t_Scene_ForwardScene *)buf;
		constructInPlace(sendCmd);
		sendCmd->dwUserID=pUser->id;
		sendCmd->dwSceneTempID=pUser->sceneTempID;
		sendCmd->size=nCmdLen;
		bcopy(pNullCmd,sendCmd->data,nCmdLen,sizeof(buf) - sizeof(Cmd::Scene::t_Scene_ForwardScene));
		pUser->scene->sendCmd(buf,sizeof(Cmd::Scene::t_Scene_ForwardScene)+nCmdLen);
		//Zebra::logger->debug("转发%ld的消息到%ld场景",pUser->id,pUser->sceneTempID);
		return true;
	}
	return false;
}

/**
* \brief 将消息转发到Session
*
*
* \param pNullCmd: 需要转发的指令
* \param nCmdLen: 指令长度
* \return 转发是否成功
*/
bool GatewayTask::forwardSession(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	if (pUser)
	{
		BYTE buf[zSocket::MAX_DATASIZE];
		Cmd::Session::t_Session_ForwardUser *sendCmd=(Cmd::Session::t_Session_ForwardUser *)buf;
		constructInPlace(sendCmd);
		sendCmd->dwID=pUser->id;
		sendCmd->size=nCmdLen;
		bcopy(pNullCmd,sendCmd->data,nCmdLen,sizeof(buf) - sizeof(Cmd::Session::t_Session_ForwardUser));
		sessionClient->sendCmd(buf,sizeof(Cmd::Session::t_Session_ForwardUser)+nCmdLen);
		//Zebra::logger->debug("转发%ld的消息到会话服务器",pUser->id,pUser->sceneTempID);
		return true;
	}
	return false;
}

/**
* \brief 解析选择消息
*
*
* \param pNullCmd: 需要转发的指令
* \param nCmdLen: 指令长度
* \return 解析是否成功
*/
bool GatewayTask::msgParse_Select(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	using namespace Cmd;

	switch(pNullCmd->byParam)
	{
	case CHECKNAME_SELECT_USERCMD_PARA:
		{
			stCheckNameSelectUserCmd *rev=(stCheckNameSelectUserCmd *)pNullCmd;

			Zebra::logger->info("CHECKNAME_SELECT_USERCMD_PARA");

			/*
			//TODO:唯一性验证服务器,暂时没有,不考虑
			if (GatewayService::getInstance().rolereg_verify)
			{
			using namespace Cmd::Super;
			t_Charname_Gateway cmd;
			cmd.wdServerID = GatewayService::getInstance().getServerID();
			cmd.accid = pUser->accid;
			strcpy(cmd.name,rev->name);
			GatewayService::getInstance().sendCmdToSuperServer(&cmd,sizeof(cmd));
			}
			*/
			Cmd::Record::t_CheckName_GateRecord send;
			strncpy(send.name,rev->name,MAX_NAMESIZE-1);
			send.accid = pUser->accid;
			//发送到RecordServer验证
			return recordClient->sendCmd(&send,sizeof(send));
		}
		break;
	case CREATE_SELECT_USERCMD_PARA:
		{
			stCreateSelectUserCmd *rev=(stCreateSelectUserCmd *)pNullCmd;

			Zebra::logger->info("CREATE_SELECT_USERCMD_PARA %u,%s,%u",pUser->accid,rev->strUserName,rev->charType);
			if (!pUser->charInfoFull())
			{
				if (strlen(rev->strUserName) > 16)
				{
					Zebra::logger->warn("名字长度太长(%d)",strlen(rev->strUserName));
					return true;
				}
				//角色数量没有满,还可以再建立角色
				pUser->createCharCmd.accid = pUser->accid;
				bzero(pUser->createCharCmd.name,sizeof(pUser->createCharCmd.name));
				strncpy(pUser->createCharCmd.name,rev->strUserName,MAX_NAMESIZE - 1);
				pUser->createCharCmd.type = rev->charType; // 性别
				pUser->createCharCmd.JobType = rev->JobType; // 职业
				pUser->createCharCmd.Face    = rev->Face;    // 头像
				//头发发型以及颜色
				pUser->createCharCmd.hair = rev->byHairType; // [ranqd] 简化头像，去掉光照和颜色
				//pUser->createCharCmd.hair = pUser->createCharCmd.hair << 24;
				//pUser->createCharCmd.hair = (pUser->createCharCmd.hair & HAIRTYPE_MASK) | (rev->byRGB & HAIRRGB_MASK);
				//if (rev->country >11)
				//Zebra::logger->debug("国家信息数量%u",GatewayService::getInstance().country_info.getCountrySize());
				if ( pUser->CountryID !=GatewayService::getInstance().country_info.getCountryID(pUser->CountryID))
				{
					Zebra::logger->warn("国家数据不合法");
					return true;
				}
				if (!GatewayService::getInstance().country_info.isEnableRegister(pUser->CountryID))
				{
					if (pUser)
					{
						Cmd::stDisableLoginCountryCmd send;
						send.status = Cmd::FALSE_REGISTER_COUNTRY;
						pUser->sendCmd(&send,sizeof(send));
					}
					return true;
				}
				strncpy(pUser->createCharCmd.mapName,GatewayService::getInstance().country_info.getMapName(pUser->CountryID).c_str(),MAX_NAMESIZE);
				pUser->createCharCmd.country =pUser->CountryID;
				pUser->createCharCmd.createip = this->getAddr();
				if (rev->five >=FIVE_NONE)
				{
					Zebra::logger->warn("五行主属性不合法");
					return true;
				}
				pUser->createCharCmd.five = rev->five;
				if (GatewayService::getInstance().rolereg_verify)
				{
					using namespace Cmd::Super;
					t_Charname_Gateway cmd;
					cmd.wdServerID = GatewayService::getInstance().getServerID();
					cmd.accid = pUser->accid;
					strncpy(cmd.name,pUser->createCharCmd.name,MAX_NAMESIZE);
					cmd.state = ROLEREG_STATE_TEST;
					GatewayService::getInstance().sendCmdToSuperServer(&cmd,sizeof(cmd));
				}
				else
				{
					if (!recordClient->sendCmd(&pUser->createCharCmd,sizeof(pUser->createCharCmd)))
						return true;
				}
				pUser->createState();
			}
			else
				Zebra::logger->warn("角色信息满,不能再创建角色了");

			return true;
		}
		break;
	case LOGIN_SELECT_USERCMD_PARA: 
		{
			Zebra::logger->info("LOGIN_SELECT_USERCMD_PARA");

			if (!pUser->isWaitUnregState())
			{
				stLoginSelectUserCmd *rev=(stLoginSelectUserCmd *)pNullCmd;
				SelectUserInfo *userinfo=pUser->getSelectUserInfo(rev->charNo);
				Zebra::logger->info("收到%ld的第%d个登录用户指令",pUser->accid,rev->charNo);
				if (!userinfo)
				{
					Zebra::logger->warn("收到不存在的角色,可能是恶意攻击!");
					return true;
				}
				if (userinfo->bitmask & CHARBASE_FORBID)
				{
					Zebra::logger->warn("收到已经被封号的角色,可能是恶意攻击!");
					return true;
				}

				if (!GatewayService::getInstance().country_info.isEnableLogin(userinfo->country))
				{
					Zebra::logger->warn("国家不允许登陆 %u,%u",pUser->accid,userinfo->country);
					Cmd::stDisableLoginCountryCmd send;
					send.status = Cmd::FALSE_LOGIN_COUNTRY;
					pUser->sendCmd(&send,sizeof(send));
					return true;
				}
				if (pUser->checkPassport(rev->jpegPassport))
				{
					if (pUser->tempid == 0 && userinfo && userinfo->id!=0)
					{
						//Zebra::logger->debug("%u,%u,%u",pUser->systemstate,pUser->tempid,userinfo->id);
						pUser->id=userinfo->id;
						pUser->face=userinfo->face;
						strncpy(pUser->name,userinfo->name,MAX_NAMESIZE);
						//加入角色,分配临时ID
						if (GateUserManager::getInstance()->addUser(pUser) && GateUserManager::getInstance()->addCountryUser(pUser))
						{
							Zebra::logger->info("注册 %u,%s",pUser->accid,userinfo->name);
							//注册角色
							pUser->reg(rev->charNo);
							return true;
						}
						else
						{
							Zebra::logger->warn("选择%s时添加用户失败",userinfo->name);
						}
					}
					else
						Zebra::logger->warn("用户%s(%ld)重复选择角色",userinfo->name,userinfo->id);
				}
				else
					Zebra::logger->warn("验证图形验证码失败 %u",pUser->accid);
			}
			else
			{
				Zebra::logger->warn("用户%s(%ld)正在退出状态",pUser->name,pUser->id);
			}
			return true;
		}
		break;
	case DELETE_SELECT_USERCMD_PARA:
		{
			stDeleteSelectUserCmd *rev=(stDeleteSelectUserCmd *)pNullCmd;

			Zebra::logger->info("DELETE_SELECT_USERCMD_PARA");
			SelectUserInfo *userinfo=pUser->getSelectUserInfo(rev->charNo);
			if (!userinfo)
			{
				Zebra::logger->warn("收到不存在的角色,可能是恶意攻击!");
				return true;
			}
			//debug版本去掉数字密码功能
#if 0
			//如果启用数字密码
			if (numPassword[0] && strncmp(numPassword,rev->numPassword,sizeof(numPassword)) != 0)
			{
				stDeleteErrorSelectUserCmd ret;
				sendCmd(&ret,sizeof(ret));
				return true;
			}
#endif
			if (pUser->tempid == 0 && userinfo && userinfo->id!=0)
			{
				Zebra::logger->info("用户(%s,%d)删除角色",userinfo->name,userinfo->id);
				//Cmd::Record::t_DelChar_GateRecord send;
				Cmd::Session::t_DelChar_GateSession send;
				send.accid=pUser->accid;
				send.id=userinfo->id;
				send.countryid = pUser->CountryID;
				strncpy(send.name,userinfo->name,MAX_NAMESIZE);
				sessionClient->sendCmd(&send,sizeof(send));

				Cmd::Mini::t_UserDelete_Gateway d;
				d.userID = userinfo->id;
				miniClient->sendCmd(&d,sizeof(d));

				pUser->delSelectUserInfo(rev->charNo);
				//recordClient->sendCmd(&send,sizeof(send));
			}
			else
				Zebra::logger->warn("用户%s(%ld)删除角色失败",userinfo->name,userinfo->id);
			return true;
		}
		break;
	case REQUEST_COUNTRY_SELECT_USERCMD_PARA:
		{
			Zebra::logger->info("REQUEST_COUNTRY_SELECT_USERCMD_PARA");

			if (Zebra::global["countryorder"] == "2")
			{
				Cmd::Record::t_request_Country_GateRecord reqRecord; 
				recordClient->sendCmd(&reqRecord,sizeof(reqRecord));
			}
			else  if (Zebra::global["countryorder"] == "1")
			{
				Cmd::Session::t_request_Country_GateSession reqSession;
				sessionClient->sendCmd(&reqSession,sizeof(reqSession));
			}
			/*
			#define country_info GatewayService::getInstance().country_info
			char Buf[country_info.getCountrySize()*sizeof(Cmd::Country_Info) + sizeof(Cmd::stCountryInfoUserCmd)];
			bzero(Buf,sizeof(Buf));
			Cmd::stCountryInfoUserCmd *ciu = (Cmd::stCountryInfoUserCmd*)Buf;
			constructInPlace(ciu);
			ciu->size = country_info.getAll((char *)ciu->countryinfo);
			pUser->sendCmd(ciu,sizeof(Cmd::stCountryInfoUserCmd) + ciu->size *sizeof(Cmd::Country_Info));
			// */
			return true;
		}
		break;
	}

	Zebra::logger->error("GatewayTask::msgParse_Select(%u,%u,%u)",pNullCmd->byCmd,pNullCmd->byParam,nCmdLen);
	return false;
}

/**
* \brief 解析时间消息
*
*
* \param pNullCmd: 需要转发的指令
* \param nCmdLen: 指令长度
* \return 解析是否成功
*/
bool GatewayTask::msgParse_Time(const Cmd::stNullUserCmd *pNullCmd,const DWORD nCmdLen)
{
	using namespace Cmd;

	switch(pNullCmd->byParam)
	{
	case USERGAMETIME_TIMER_USERCMD_PARA:
		{
			if (!dwTimestampServer)
			{
				dwTimestampServer=GatewayTimeTick::currentTime.msecs() - pNullCmd->dwTimestamp;
			}
			stUserGameTimeTimerUserCmd *rev=(stUserGameTimeTimerUserCmd *)pNullCmd;
			if (qwGameTime && (SQWORD)(rev->qwGameTime - (qwGameTime + 
				(GatewayTimeTick::currentTime.sec() - GameTimeSyn.sec()))) >= sampleInterval_error_sec)
			{   
				if (this->pUser)
				{
					Zebra::logger->info("客户端游戏时间太快,使用了加速器,需要断开连接(accid = %u,%u,%I64u)",this->pUser->accid,pNullCmd->dwTimestamp,rev->qwGameTime - (qwGameTime + (GatewayTimeTick::currentTime.sec() - GameTimeSyn.sec())));
				}
				Zebra::logger->debug("游戏时间:(accid=%u,%I64u,%I64u)",accid,rev->qwGameTime,qwGameTime + (GatewayTimeTick::currentTime.sec() - GameTimeSyn.sec()));
				Terminate();
			}
			if (!haveCheckTime)
				haveCheckTime = true;
			else
				Zebra::logger->warn("校对时间标记错误(accid=%u)",accid);
			return true;
		}
		break;
	case PING_TIMER_USERCMD_PARA:
		{       
			stPingTimeTimerUserCmd cmd;
			sendCmd(&cmd,sizeof(cmd));
			return true;
		}       
		break;  
	}

	Zebra::logger->error("GatewayTask::msgParse_Time(%u,%u,%u)",pNullCmd->byCmd,pNullCmd->byParam,nCmdLen);
	return false;
}

/**
* \brief 检测时间
*
* \param ct: 当前时间
* \return 是否通过检测
*/
bool GatewayTask::checkTime(const zRTime &ct)
{
	if (accountVerified == ACCOUNTVERIFY_SUCCESS)
	{
		if (ct >= lastCheckTime)
		{
			if (haveCheckTime)
			{
				//校对时间
				if (_retset_gametime(ct))
				{
					Cmd::stGameTimeTimerUserCmd cmd;
					if (qwGameTime)
					{
						cmd.qwGameTime = qwGameTime + (GatewayTimeTick::currentTime.sec() - GameTimeSyn.sec());
						sendCmd(&cmd,sizeof(cmd));
					}
					//准备重新设置同步时间
					dwTimestampServer=0;
				}
				haveCheckTime = false;
				Cmd::stRequestUserGameTimeTimerUserCmd cmd;
				sendCmd(&cmd,sizeof(cmd));
			}
			else
			{
				Zebra::logger->info("客户端指定时间内没有返回校对时间指令（accid = %u,snd_queue_size = %u）,注销",accid,mSocket->snd_queue_size());
				TerminateWait();
			}

			lastCheckTime = ct;
			lastCheckTime.addDelay(checkTimeInterval);
		}
	}

	return true;
}

/**
* \brief 对客户端发送过来的指令进行检测
* 主要检测时间戳等,对加速器等进行防止
* \param pCmd 待检测的指令
* \param ct 当前时间
* \return 检测是否成功
*/
bool GatewayTask::checkUserCmd(const Cmd::stNullUserCmd *pCmd,const zRTime &ct)
{
	if (dwTimestampServer)
	{
		if (abs((double)((dwTimestampServer + pCmd->dwTimestamp) - ct.msecs())) > sampleInterval_error_msecs)
		{
			Zebra::logger->info("客户端指令时间太快,使用了加速器,需要断开连接（accid = %u,%d,%I64u）",accid,(dwTimestampServer + pCmd->dwTimestamp) - ct.msecs(),initTime.elapse(ct));
			v_lastSampleTime = pCmd->dwTimestamp + sampleInterval;
			dwTimestampServer = ct.msecs() - pCmd->dwTimestamp;
			return false;
		}
		if (pCmd->dwTimestamp >= v_lastSampleTime)
		{
			v_lastSampleTime = pCmd->dwTimestamp + sampleInterval;
			/*
			if (v_samplePackets > 30)
			{
			Zebra::logger->debug("%d毫秒收到指令个数%d",sampleInterval,v_samplePackets);
			}
			// */
			v_samplePackets = 0;
		}
		if (pCmd->byCmd != Cmd::MAPSCREEN_USERCMD)
			v_samplePackets++;
		if (v_samplePackets > maxSamplePPS)
		{
			Zebra::logger->info("客户端指令发送过快,需要断开连接（accid = %u,%u/%u毫秒）",accid,v_samplePackets,sampleInterval);
			return false;
		}
	}
	return true;
}

/**
* \brief 解析消息
*
* \param ptNull: 需要转发的指令
* \param nCmdLen: 指令长度
* \return 解析是否成功
*/
bool GatewayTask::msgParse(const Cmd::t_NullCmd *ptNull,const DWORD nCmdLen)
{
	using namespace Cmd;
	stNullUserCmd *pNullCmd = (stNullUserCmd *)ptNull;

#ifdef _MSGPARSE_
	Zebra::logger->error("?? GatewayTask::msgParse(%u,%u,%u)",pNullCmd->byCmd,pNullCmd->byParam,nCmdLen);
#endif

	if (isTerminate()) return false;
	if (!checkUserCmd(pNullCmd,GatewayTimeTick::currentTime))
	{
		Terminate();
		return false;
	}

	if (pUser)
	{

		switch(pNullCmd->byCmd)
		{


		case LOGON_USERCMD:
			if (pNullCmd->byParam==BACKSELECT_USERCMD_PARA)
			{
				if ( pUser && ( pUser->isPlayState() ))
				{
					Zebra::logger->info("%s(%ld)小退",pUser->name,pUser->id);
					pUser->backSelect=true;
					pUser->unreg();
				}
				else if( pUser && pUser->isSelectState() )// [ranqd Add] 在选择人物界面退回到选择服务器界面
				{
					GateUserManager::getInstance()->removeUserOnlyByAccID(pUser->accid);
					pUser->SendCountryInfo();
					pUser->selectServerState();
				}
			}
			if( pNullCmd->byParam == CLIENT_SELETCT_COUNTRY )
			{
				if( pUser->isSelectServerState() )
				{
					// [ranqd] 收到用户选择服务器消息
					pUser->CountryID = ((stSelectCountryUserCmd*)pNullCmd)->id;
					pUser->initState();
					if (pUser->beginSelect())
					{
						//通知登陆服务器,网关连接数
						GatewayService::getInstance().notifyLoginServer();
					}
				}
			}
			return true;
		case SELECT_USERCMD:
			if (pUser->isSelectState())
			{
				if (msgParse_Select(pNullCmd,nCmdLen))
				{
					return true;
				}
			}
			else
				Zebra::logger->warn("%s(%ld)不再选择用户状态,发送选择用户系列指令",pUser->name,pUser->id);
			return true;
		case TIME_USERCMD:
			return msgParse_Time(pNullCmd,nCmdLen);
		case CHAT_USERCMD:
			{
				if (pUser && pUser->isPlayState())
				{
					//如果是普通聊天消息则进行词汇过滤
					if (ALL_CHAT_USERCMD_PARAMETER == pNullCmd->byParam)
					{
						Cmd::stChannelChatUserCmd *rev = (Cmd::stChannelChatUserCmd *)pNullCmd;

						if (rev->dwType==CHAT_TYPE_COUNTRY||rev->dwType==CHAT_TYPE_WORLD)
						{
							if (GatewayTimeTick::currentTime<nextCountryChatTime)
							{
								zRTime ctv;
								Cmd::stChannelChatUserCmd send;
								send.dwType=Cmd::CHAT_TYPE_SYSTEM;
								send.dwSysInfoType = Cmd::INFO_TYPE_FAIL;
								send.dwChatTime = ctv.sec();
								bzero(send.pstrName,sizeof(send.pstrName));
								bzero(send.pstrChat,sizeof(send.pstrChat));
								strncpy((char *)send.pstrChat,"对不起,该频道3秒钟可以发言一次",MAX_CHATINFO-1);

								pUser->sendCmd(&send,sizeof(send));
								return true;
							}
						}
						else
						{
							if (GatewayTimeTick::currentTime<nextChatTime)
							{
								zRTime ctv;
								Cmd::stChannelChatUserCmd send;
								send.dwType=Cmd::CHAT_TYPE_SYSTEM;
								send.dwSysInfoType = Cmd::INFO_TYPE_FAIL;
								send.dwChatTime = ctv.sec();
								bzero(send.pstrName,sizeof(send.pstrName));
								bzero(send.pstrChat,sizeof(send.pstrChat));
								strncpy((char *)send.pstrChat,"对不起,您说话太快了,请不要刷屏",MAX_CHATINFO-1);

								pUser->sendCmd(&send,sizeof(send));
								return true;
							}
							if (rev->dwType == CHAT_TYPE_BLESS_MSG)
							{
								forwardSession(pNullCmd,nCmdLen);
								return true;
							}
						}
						nextChatTime.now();
						nextChatTime.addDelay(chatInterval);
						nextCountryChatTime.now();
						nextCountryChatTime.addDelay(3000);

						/*
						if (rev->dwType==CHAT_TYPE_OVERMAN)
						{
						BYTE buf[zSocket::MAX_DATASIZE];
						Cmd::GmTool::t_Chat_GmTool *cmd=(Cmd::GmTool::t_Chat_GmTool *)buf;
						bzero(buf,sizeof(buf));
						constructInPlace(cmd);

						strncpy(cmd->userName,pUser->name,MAX_NAMESIZE);
						cmd->countryID = pUser->countryID;
						cmd->sceneID = pUser->sceneID;
						cmd->dwType = rev->dwType;
						strncpy(cmd->content,rev->pstrChat,255);
						cmd->size = rev->size;
						if (cmd->size)
						bcopy(rev->tobject_array,cmd->tobject_array,cmd->size*sizeof(Cmd::stTradeObject));
						GatewayService::getInstance().sendCmdToSuperServer(cmd,sizeof(Cmd::GmTool::t_Chat_GmTool)+cmd->size*sizeof(Cmd::stTradeObject));
						}
						*/
					}

					switch (pNullCmd->byParam)
					{
					case CREATE_CHANNEL_USERCMD_PARAMETER:
					case INVITE_CHANNEL_USERCMD_PARAMETER:
					case JOIN_CHANNEL_USERCMD_PARAMETER:
					case LEAVE_CHANNEL_USERCMD_PARAMETER:
						if (forwardSession(pNullCmd,nCmdLen))
							return true;
						break;
					case QUESTION_OBJECT_USERCMD_PARA:
						if (forwardSession(pNullCmd,nCmdLen))
						{
#ifdef _DEBUG
							Zebra::logger->debug("收到交易物品查询命令,转发到会话");
#endif              
							return true;
						}
						break;
					default:
						if (forwardScene(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
				return true;
			}
			break;
		case UNION_USERCMD: // 将消息转发到会话服务器在那里处理帮会事务。
			{
				if (pUser && pUser->isPlayState())
				{
					switch(pNullCmd->byParam)
					{
					case CREATE_UNION_PARA:      // 建立帮会
						{
							Cmd::stCreateUnionCmd * cmd = (stCreateUnionCmd *)pNullCmd;
							if (!checkNewName(cmd->UnionName))
							{
								zRTime ctv;
								Cmd::stChannelChatUserCmd send;
								send.dwType=Cmd::CHAT_TYPE_SYSTEM;
								send.dwSysInfoType = Cmd::INFO_TYPE_FAIL;
								send.dwChatTime = ctv.sec();
								bzero(send.pstrName,sizeof(send.pstrName));
								bzero(send.pstrChat,sizeof(send.pstrChat));
								strncpy((char *)send.pstrChat,"对不起,帮会名字不合法,请使用字母、数字和汉字",MAX_CHATINFO-1);

								sendCmd(&send,sizeof(send));
								return true;
							}
						}
					case UNION_STATUS_CHECK_PARA:  // 帮会状态检查判断此人是否可以建立新帮会
					case ADD_MEMBER_TO_UNION_PARA:    // 邀请玩家加入帮会
					case ENTER_UNION_CITY_AREA_PARA: // 进入帮会所得城市帮属场景
					case QUESTION_UNION_CITY_INFO_PARA:
					case CONTRIBUTE_UNION_PARA: // 帮会捐献
						{// 以上消息转发到场景服务器处理
							if (forwardScene(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					default:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;

					}
				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
				return true;
			}
			break;
		case SAFETY_USERCMD:
			{
				if (pUser==NULL || !pUser->isPlayState())
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",
						pNullCmd->byCmd,pNullCmd->byParam);
					return true;
				}

				switch (pNullCmd->byParam)
				{
				case SET_SAFETY_PARA:
					{
						Cmd::stSetSafetyUserCmd* cmd = (Cmd::stSetSafetyUserCmd*)pNullCmd;
						if (cmd->pwd == this->numPwd)
						{
							if (forwardScene(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						else
						{
							zRTime ctv;
							Cmd::stChannelChatUserCmd send;
							send.dwType=Cmd::CHAT_TYPE_SYSTEM;
							send.dwSysInfoType = Cmd::INFO_TYPE_FAIL;
							send.dwChatTime = ctv.sec();
							bzero(send.pstrName,sizeof(send.pstrName));
							bzero(send.pstrChat,sizeof(send.pstrChat));
							strncpy((char *)send.pstrChat,"对不起,数字密码不正确",MAX_CHATINFO-1);

							pUser->sendCmd(&send,sizeof(send));
						}
					}
					break;
				case SET_TEMP_UNSAFETY_PARA:
					{
						Cmd::stSetTempUnSafetyUserCmd* cmd = (Cmd::stSetTempUnSafetyUserCmd*)pNullCmd;
						if (cmd->pwd == this->numPwd)
						{
							if (forwardScene(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						else
						{
							zRTime ctv;
							Cmd::stChannelChatUserCmd send;
							send.dwType=Cmd::CHAT_TYPE_SYSTEM;
							send.dwSysInfoType = Cmd::INFO_TYPE_FAIL;
							send.dwChatTime = ctv.sec();
							bzero(send.pstrName,sizeof(send.pstrName));
							bzero(send.pstrChat,sizeof(send.pstrChat));
							strncpy((char *)send.pstrChat,"对不起,数字密码不正确",MAX_CHATINFO-1);

							pUser->sendCmd(&send,sizeof(send));
						}
					}
					break;
				case SET_SAFETY_DETAIL_PARA:
					{
						Cmd::stSetSafetyDetailUserCmd* cmd = (Cmd::stSetSafetyDetailUserCmd*)pNullCmd;
						if (cmd->pwd == this->numPwd)
						{
							if (forwardScene(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						else
						{
							zRTime ctv;
							Cmd::stChannelChatUserCmd send;
							send.dwType=Cmd::CHAT_TYPE_SYSTEM;
							send.dwSysInfoType = Cmd::INFO_TYPE_FAIL;
							send.dwChatTime = ctv.sec();
							bzero(send.pstrName,sizeof(send.pstrName));
							bzero(send.pstrChat,sizeof(send.pstrChat));
							strncpy((char *)send.pstrChat,"对不起,数字密码不正确",MAX_CHATINFO-1);

							pUser->sendCmd(&send,sizeof(send));
						}
					}
					break;
				default:
					{
						// 以上消息转发到场景服务器处理
						if (forwardScene(pNullCmd,nCmdLen))
						{
							//fprintf(stderr,"cmd:%u",pNullCmd->byCmd);
							return true;
						}
					}
					break;
				}

				return true;
			}
			break;
		case SAFETY_COWBOX:
		case TURN_USERCMD:
		case HOTSPRING_USERCMD:
		case HORSETRAINING_USERCMD:
		case SURPLUS_ATTRIBUTE_USERCMD:		//装备自由加点消息 sky
		case MAKEOBJECT_USERCMD:			//宝石镶嵌消息 sky
		case ARENA_USERCMD:					//战场-副本-竞技场相关指令 sky
		case REMAKEOBJECT_USERCMD:
			{
				if (forwardScene(pNullCmd,nCmdLen))
				{
					return true;
				}
			}
			break;
		case COUNTRY_USERCMD:
			{
				if (pUser && pUser->isPlayState())
				{
					switch(pNullCmd->byParam)
					{
					case ANSWER_COUNTRY_DARE_PARA:
					case CONTRIBUTE_COUNTRY_MATERIAL:
					case CANCEL_COUNTRY_PARA:
					case APPLY_COUNTRY_PARA:
					case CHANGE_COUNTRY_PARA:// 变更国籍
					case REQUEST_COUNTRY_POWER_PARA:
						{
							// 以上消息转发到场景服务器处理
							if (forwardScene(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					case CONFIRM_SEARCHER_PARA:
					case UP_TECH_DEGREE_PARA:
					case SET_TECH_SEARCH_PARA:
					case REQ_WAIT_OFFICIAL_PARA:
					case SELECT_TRANS_LEVEL:
					case REQ_TECH_PARA:  
					case DARE_COUNTRY_FORMAL_PARA:
					case REQUEST_DARE_COUNTRY_PARA:
					case REQUEST_DARE_RECORD_PARA:
					case TRANS_DARE_COUNTRY_PARA:
					case FORBID_TALK_COUNTRY_PARA:
					case CANCEL_TECH_SEARCH_PARA:
					case ANTI_DARE_COUNTRY_FORMAL_PARA:
					case COUNTRY_NOTE_PARA:
					case CANCEL_DIPLOMAT_PARA:
					case CANCEL_CATCHER_PARA:
					case APPOINT_CATCHER_PARA:
					case APPOINT_DIPLOMAT_PARA:
					case REQ_DAILY_EMPEROR_MONEY:
					case REQ_KING_LIST_PARA:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}

						}
						break;
					default:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					}

				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
				return true;
			}
			break;
		case VOTE_USERCMD:
			{
				if (pUser && pUser->isPlayState())
				{
					switch(pNullCmd->byParam)
					{
					case REQUEST_VOTE_LIST_PARA:
					case COMMIT_VOTE_PARA:
					case REQ_ARMY_GEN_PARA:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}

						}
						break;
					default:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					}

				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
				return true;
			}
			break;
		case ARMY_USERCMD:
			if (pUser && pUser->isPlayState())
			{
				switch (pNullCmd->byParam)
				{
				case CREATE_ARMY_PARA:
				case REQ_WAIT_GEN_PARA:
				case REQ_ARMY_SPEC_PARA:
				case REMOVE_ARMY_PARA:
				case CHANGE_ARMY_NAME_PARA:
				case EXIT_ARMY_PARA:
				case ADD_ARMY_CAPTAIN_PARA:
				case FIRE_ARMY_CAPTAIN_PARA:
					{
						if (forwardSession(pNullCmd,nCmdLen))
						{
							return true;
						}

					}
					break;
				case REQ_ARMY_LIST_PARA:
					{
						// 以上消息转发到场景服务器处理
						if (forwardScene(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					break;
				default:
					{
						if (forwardSession(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					break;
				}

			}
			else
			{
				Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
			}
			break;
		case GEM_USERCMD:
			{
				if (pUser && pUser->isPlayState())
				{
					switch (pNullCmd->byParam)
					{
					case REQUEST_DRAGON_PARA:
					case REQUEST_TIGER_PARA:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					default:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					}
				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
			}
			break;
		case DARE_USERCMD: // 战斗消息转发到会话服务器
			{
				if (pUser && pUser->isPlayState())
				{
					switch (pNullCmd->byParam)
					{
					case ACTIVE_DARE_PARA: // 发起挑战
					case QUERY_DARE_LIST_PARA:
					case NOTIFY_DARE_PARA: // 战斗相关通知
					case GET_UNION_CITY_TAX_PARA:
						{//以上消息转发到会话服务器处理
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					case ACTIVE_UNION_CITY_DARE_PARA:
						{
							if (forwardScene(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					default:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					}
				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
				return true;
			}
			break;
		case NPCDARE_USERCMD: // 战斗消息转发到会话服务器
			{
				if (pUser && pUser->isPlayState())
				{
					switch (pNullCmd->byParam)
					{
					case QUESTION_NPCDARE_INFO_PARA:
					case NPCDARE_DARE_PARA: // 发起挑战
					case NPCDARE_GETGOLD_PARA:
						{//以上消息转发到场景服务器处理
							if (forwardScene(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					default:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					}
				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
				return true;
			}
			break;
		case SEPT_USERCMD:
			{
				if (pUser && pUser->isPlayState())
				{
					switch(pNullCmd->byParam)
					{
					case CREATE_SEPT_PARA:      // 建立家族
						{
							Cmd::stCreateSeptCmd * cmd = (stCreateSeptCmd *)pNullCmd;
							if (!checkNewName(cmd->SeptName))
							{
								zRTime ctv;
								Cmd::stChannelChatUserCmd send;
								send.dwType=Cmd::CHAT_TYPE_SYSTEM;
								send.dwSysInfoType = Cmd::INFO_TYPE_FAIL;
								send.dwChatTime = ctv.sec();
								bzero(send.pstrName,sizeof(send.pstrName));
								bzero(send.pstrChat,sizeof(send.pstrChat));
								strncpy((char *)send.pstrChat,"对不起,家族名字不合法,请使用字母、数字和汉字",MAX_CHATINFO-1);

								sendCmd(&send,sizeof(send));
								return true;
							}
						}
					case SEPT_STATUS_CHECK_PARA:  // 家族状态检查判断此人是否可以建立新家族
					case ADD_MEMBER_TO_SEPT_PARA:  // 邀请玩家加入家族
						{              // 以上消息转发到场景服务器处理
							if (forwardScene(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					default:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;

					}
				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
				return true;
			}
			break;
		case RELATION_USERCMD:
			{
				if (pUser && pUser->isPlayState())
				{
					switch(pNullCmd->byParam)
					{
					case MARRY_STATUS_CHECK_PARA:
						{    // 以上消息转发到场景服务器处理
							if (forwardScene(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					default:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;

					}
				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
				return true;
			}
			break;
		case SCHOOL_USERCMD:
			{
				if (pUser && pUser->isPlayState())
				{
					switch(pNullCmd->byParam)
					{
						//            case Cmd::SCHOOL_BULLETIN__PARA:
						//            pUser->sendCmd(pNullCmd,nCmdLen);
						//            return true;
					case Cmd::SCHOOL_STATUS_CHECK_PARA:
					case Cmd::CREATE_SCHOOL_PARA:
					case Cmd::ADD_MEMBER_TO_SCHOOL_PARA:
						{    // 以上消息转发到场景服务器处理
							if (forwardScene(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					default:
						{
							if (forwardSession(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
						break;
					}
				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
				return true;
			}
			break;
		case MOVE_USERCMD:
			{
				if( strcmp(pUser->name,"test22") == 0 )
				{
					printf("收到移动消息\n");
				}
				zRTime ctv;
				if (pNullCmd->byParam == Cmd::USERMOVE_MOVE_USERCMD_PARA)
				{

					//fprintf(stderr,"gate收到移动消息\n");
					Cmd::stUserMoveMoveUserCmd send = *(Cmd::stUserMoveMoveUserCmd *)pNullCmd;
					send.dwTimestamp = ctv.msecs();
					if (pUser && pUser->isPlayState())
					{
						if (forwardScene(&send,sizeof(send)))
						{
							return true;
						}
					}
					else
					{
						Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
					}
				}
				else
				{
					if (pUser && pUser->isPlayState())
					{
						if (forwardScene(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					else
					{
						Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
					}
				}
				return true;
			}
			break;
		case MAGIC_USERCMD:
			{
#ifdef _DEBUG
				Zebra::logger->debug("攻击指令时间戳:%ld",pNullCmd->dwTimestamp);
#endif
				zRTime ctv;
				if (pNullCmd->byParam == Cmd::MAGIC_USERCMD_PARA)
				{
					Cmd::stAttackMagicUserCmd *send = (Cmd::stAttackMagicUserCmd *)pNullCmd;
					send->dwTimestamp = ctv.msecs();
					if (pUser && pUser->isPlayState())
					{
						if (forwardScene(send,nCmdLen))
						{
							return true;
						}
					}
					else
					{
						Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
					}
				}
				else
				{
					if (pUser && pUser->isPlayState())
					{
						if (forwardScene(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					else
					{
						Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
					}
				}
				return true;
			}
			break;
		case DATA_USERCMD:
			{
				switch(pNullCmd->byParam)
				{
				case LEVELDEGREE_DATA_USERCMD_PARA:
					{
						if (forwardSession(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					break;
				case LOADMAPOK_DATA_USERCMD_PARA:
					{
						if (forwardScene(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					break;
				default:
					{
						Zebra::logger->error("无效的DATA_USERCMD从客户端发送过来byParam=%u",pNullCmd->byParam);
					}
					break;
				}
				return true;
			}
			break;
		case PET_USERCMD: 
			{
				if (pNullCmd->byParam==Cmd::CHANGENAME_PET_PARA)
				{
					Cmd::stChangeNamePetCmd * cmd = (stChangeNamePetCmd *)pNullCmd;
					if (!checkNewName(cmd->name))
					{
						zRTime ctv;
						Cmd::stChannelChatUserCmd send;
						send.dwType=Cmd::CHAT_TYPE_SYSTEM;
						send.dwSysInfoType = Cmd::INFO_TYPE_FAIL;
						send.dwChatTime = ctv.sec();
						bzero(send.pstrName,sizeof(send.pstrName));
						bzero(send.pstrChat,sizeof(send.pstrChat));
						strncpy((char *)send.pstrChat,"宠物名字不合法,请使用字母、数字和汉字",MAX_CHATINFO-1);

						sendCmd(&send,sizeof(send));
						return true;
					}
				}
			}
			//注意这里没有break
		case PROPERTY_USERCMD:
#ifdef _DEBUG
			if (pNullCmd->byCmd == PROPERTY_USERCMD) Zebra::logger->error("收到人物属性操作指令！");
#endif
			//注意这里没有break
		case RELIVE_USERCMD:
			//注意这里没有break
		case TRADE_USERCMD:
			//注意这里没有break
		case MAPSCREEN_USERCMD:
			//注意这里没有break
		case TASK_USERCMD: 
			{
				if (pUser && pUser->isPlayState())
				{
					if (forwardScene(pNullCmd,nCmdLen))
					{
						return true;
					}
				}
				else
				{
					Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				}
				return true;
			}
			break;
		case GOLD_USERCMD:
			{
				if (GatewayService::service_gold)
				{
					if (pUser && pUser->isPlayState())
					{
						switch(pNullCmd->byParam)
						{
							using namespace Bill;
						case QUERY_GOLD_PARA:
							{
							}
							break;
							//请求兑换金币
						case REQUEST_REDEEM_GOLD_PARA:
							{
								using namespace Bill;
								stRequestRedeemGold* cmd = (stRequestRedeemGold*)pNullCmd;
								if ((int)cmd->dwNum < 0)
								{
									Zebra::logger->error("请求兑换金币(accid=%d),数量%d,是负数,外挂所致",this->accid,cmd->dwNum);
									return true;
								}

								t_Request_Redeem_Gold_Gateway send;
								strncpy(send.account,this->account,MAX_ACCNAMESIZE);
								send.accid = this->accid;

								if (this->pUser)
								{
									send.charid = this->pUser->id;
								}

								send.point = cmd->dwNum;
								accountClient->sendCmd(&send,sizeof(send));  
								//Zebra::logger->debug("请求兑换金币(accid=%d),数量%d",this->accid,cmd->dwNum);
							}
							break;
							//请求兑换月卡
						case REQUEST_REDEEM_MONTH_CARD_PARA:
							{
								using namespace Bill;

								t_Request_Redeem_MonthCard_Gateway send;
								strncpy(send.account,this->account,MAX_ACCNAMESIZE);
								send.accid = this->accid;

								if (this->pUser)
								{
									send.charid = this->pUser->id;
								}
								accountClient->sendCmd(&send,sizeof(send));  
								//Zebra::logger->debug("请求兑换月卡(accid=%d)",this->accid);
							}
							break;
							//请求查询点卡
						case REQUEST_POINT_PARA:
							{
								t_Request_Point_Gateway send;
								strncpy(send.account,this->account,MAX_ACCNAMESIZE);
								send.accid = this->accid;
								if (this->pUser)
								{
									send.charid = this->pUser->id;
								}
								accountClient->sendCmd(&send,sizeof(send));  
								//Zebra::logger->debug("请求查询剩余点数(accid=%d)",this->accid);
							}
							break;
						case CONSUME_CARD_PARA:
							{
								Cmd::stConSumeCardCard *rev = (Cmd::stConSumeCardCard *)pNullCmd; 
								stConSumeCardCard_Gateway send;
								send.type=rev->type;
								strncpy(send.account,this->account,MAX_ACCNAMESIZE);
								strncpy(send.cardid,rev->cardid,sizeof(send.cardid));
								send.accid = this->accid;
								accountClient->sendCmd(&send,sizeof(send));  
								Zebra::logger->debug("%s(%u)请求消费道具卡:%s,类型:%d",pUser->name,pUser->id,rev->cardid,rev->type);
								return true;
							}
							break;
						default:
							break;

						}
					}
				}
				else
				{
					stStopServiceGold ret;
					this->sendCmd(&ret,sizeof(ret));
					Zebra::logger->debug("金币系统停止维护");
				}

				return true;
			}
			break;
		case QUIZ_USERCMD:
			{//答题命令
				switch(pNullCmd->byParam)
				{
				case ANSWER_QUIZ_PARA:
					{
						if (forwardSession(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					break;
				case QUERY_QUIZ_PARA:
					{
						if (forwardSession(pNullCmd,nCmdLen) 
							&& forwardScene(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					break;
				case CREATE_QUIZ_PARA:
					{
						if (forwardScene(pNullCmd,nCmdLen))
						{
							return true;
						}  
					}
					break;
				case  QUIZ_EXIT_PARA:
					{
						if (forwardSession(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					break;
				default:
					break;
				}

				return true;
			}
			break;
		case GMTOOL_USERCMD:
		case MAIL_USERCMD:
		case AUCTION_USERCMD:
		case CARTOON_USERCMD:
		case PRISON_USERCMD:
			{
				if (forwardScene(pNullCmd,nCmdLen))
				{
					return true;
				}
				/*
				switch(pNullCmd->byParam)
				{
				case SEND_MAIL_PARA:
				if (forwardSession(pNullCmd,nCmdLen))
				{
				return true;
				}
				break;
				default:
				if (forwardScene(pNullCmd,nCmdLen))
				{
				return true;
				}
				break;
				}
				*/
				return true;
			}
			break;
		case STOCK_SCENE_USERCMD:
			{
				if (GatewayService::service_stock)
				{
					if (pUser && pUser->isPlayState())
					{
						if (forwardScene(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					else
					{
						Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
					}
				}
				else
				{
					stStopServiceStockUserCmd ret;
					this->sendCmd(&ret,sizeof(ret));
					Zebra::logger->debug("股票系统停止维护");
				}
				return true;
			}
			break;
		case STOCK_BILL_USERCMD:
			{
				if (GatewayService::service_stock)
				{
					if (pUser && pUser->isPlayState())
					{
						switch(pNullCmd->byParam)
						{
						case PARA_CANCELLISTALL_STOCKPARA:
							{
								return true;
							}
							break;
						default:
							if (pUser->forwardBill(pNullCmd,nCmdLen))
							{
								return true;
							}
						}
					}
					else
					{
						Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
					}
				}
				else
				{
					stStopServiceStockUserCmd ret;
					this->sendCmd(&ret,sizeof(ret));
					Zebra::logger->debug("股票系统停止维护");
				}
				return true;
			}
			break;
		case GIFT_USERCMD:
			{
				if (pUser && pUser->isPlayState())
					if (forwardSession(pNullCmd,nCmdLen))
						return true;
					else
						Zebra::logger->error("连接状态不正确,忽略这条指令(%d,%d)",pNullCmd->byCmd,pNullCmd->byParam);
				return true;
			}
			break;
		case ALLY_USERCMD:
			{
				switch (pNullCmd->byParam)
				{
				case REQ_COUNTRY_ALLY_INFO_PARA:
				case REQ_COUNTRY_ALLY_PARA:
				case CANCEL_COUNTRY_ALLY_PARA:
					{
						if (forwardSession(pNullCmd,nCmdLen))
						{
							return true;
						}
					}
					break;
					/*case CREATE_QUIZ_PARA:
					{
					if (forwardScene(pNullCmd,nCmdLen))
					{
					return true;
					}  
					}
					break;*/
				default:
					break;
				}

				return true;
			}
			break;
		case MINIGAME_USERCMD:
			{
				if (pUser && pUser->isPlayState())
				{
					Cmd::stMiniGameUserCmd * rev = (Cmd::stMiniGameUserCmd *)pNullCmd;
					if (COMMON_MINI_PARA==rev->byParam && LOGIN_COMMON_MINI_PARA==rev->subParam)
					{
						Cmd::Mini::t_UserLogin_Gateway send;
						send.userID = pUser->id;
						send.countryID = pUser->countryID;
						send.face = pUser->face;
						send.gateServerID = GatewayService::getInstance().getServerID();
						send.sceneServerID = pUser->scene->getServerID();
						strncpy(send.name,pUser->name,MAX_NAMESIZE-1);
						return miniClient->sendCmd(&send,sizeof(send));
					}
					if (rev->byParam>10)
						return forwardScene(pNullCmd,nCmdLen);
					else
						return forwardMini(pNullCmd,nCmdLen);
				}
			}
			break;
		case RECOMMEND_USERCMD:
			{
				if (pUser && pUser->isPlayState())
				{
					if (forwardSession(pNullCmd,nCmdLen))
					{
						return true;
					}
				}
			}
			break;
		default:
			break;
		}
	}

	Zebra::logger->error("GatewayTask::msgParse(%u,%u,%u)",pNullCmd->byCmd,pNullCmd->byParam,nCmdLen);
	return false;
}

/**
* \brief 检查新创建的人物名字是否合法
*
*
* \param newName 新名字
* \return 是否合法
*/
bool GatewayTask::checkNewName(char * newName)
{
	DWORD i = 0;
	while (newName[i]!=0 && i<strlen(newName) && i<16)
	{
		BYTE a = newName[i];
		if (a<0x80)//asc码
		{
			if (!((a>='0' && a<='9') || (a>='A' && a<='Z') || (a>='a' && a<='z')))
				return false;
		}
		else//汉字
		{
			if (!(i<strlen(newName)-1 && i<15)) return false;

			BYTE b = newName[i+1];
			if (b==0x7F || b<0x40 || b>0xFE)//尾字节在 40-FE 之间,剔除 xx7F 一条线
				return false;

			WORD val = (a<<8)|b;
			if (!((val>=0xB0A1 && val<=0xF7FE)))//GB 2312 汉字 6763 个
				//||(val>=0xB140 && val<=0xA0FE)//CJK汉字 6080 个
				//||(val>=0xAA40 && val<=0xFEA0)))//CJK 汉字和增补的汉字 8160 个
				return false;
			i++;
		}
		i++;
	}

	return true;
}

