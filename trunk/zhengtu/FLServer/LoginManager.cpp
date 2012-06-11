/**
* \brief ��½���ӹ�������
*
* 
*/

#include "FLServer.h"

LoginManager *LoginManager::instance = NULL;
DWORD LoginManager::maxGatewayUser=MAX_GATEWAYUSER;

/**
* \brief �����������һ������
*
* \param task һ����������
* \return ����Ƿ�ɹ�
*/
bool LoginManager::add(LoginTask *task)
{
	if( (DWORD)task == 0x00000001 )
	{
		int iii = 0;
	}
	Zebra::logger->debug("LoginManager::add");
	if (task)
	{
		task->genTempID();
		mlock.lock();
		LoginTaskHashmap_const_iterator it = loginTaskSet.find(task->getTempID());
		if (it != loginTaskSet.end())
		{
			mlock.unlock();
			return false;
		}
		loginTaskSet.insert(LoginTaskHashmap_pair(task->getTempID(),task));
		mlock.unlock();
		return true;
	}
	else
		return false;
}

/**
* \brief ��һ���������Ƴ�һ������
*
* \param task һ����������
*/
void LoginManager::remove(LoginTask *task)
{
	Zebra::logger->debug("LoginManager::remove");
	if (task)
	{
		mlock.lock();
		loginTaskSet.erase(task->getTempID());
		mlock.unlock();
	}
}

/**
* \brief �㲥ָ�ָ���ĵ�½����
*
* \param loginTempID ��½���ӵ�Ψһ���
* \param pstrCmd ��ת����ָ��
* \param nCmdLen ��ת����ָ���
* \return ת���Ƿ�ɹ�
*/
bool LoginManager::broadcast(const DWORD loginTempID,const void *pstrCmd,int nCmdLen)
{
	Zebra::logger->debug("LoginManager::broadcast");
	zMutex_scope_lock scope_lock(mlock);
	LoginTaskHashmap_iterator it = loginTaskSet.find(loginTempID);
	if (it != loginTaskSet.end())
		return it->second->sendCmd(pstrCmd,nCmdLen);
	else
		return false;
}

/**
* \brief ��½��֤����
* \param loginTempID ָ���Ŀͻ���������ʱ���
* \param retcode �����صĴ���
* \param session ��½��֤���صĻỰ��Ϣ
*/
void LoginManager::verifyReturn(const DWORD loginTempID,const BYTE retcode,const t_NewLoginSession &session)
{
	Zebra::logger->debug("LoginManager::verifyReturn");
	using namespace Cmd::DBAccess;
	zMutex_scope_lock scope_lock(mlock);

	LoginTaskHashmap_iterator it = loginTaskSet.find(loginTempID);
	if (it != loginTaskSet.end())
	{
		LoginTask *task = it->second;

		GYList *gy = NULL;
		switch(retcode)
		{
		case SESSIONCHECK_SUCCESS:
			switch(session.state)
			{
			case 0:
				Zebra::logger->error("��½�ɹ�ֱ�ӷ�������");
				//��½�ɹ�ֱ�ӷ�������
				Zebra::logger->error("gameZone = %u",session.gameZone);
				gy = GYListManager::getInstance().getAvl(session.gameZone);
				if (NULL == gy)
				{
					task->LoginReturn(Cmd::LOGIN_RETURN_GATEWAYNOTAVAILABLE);
					printf("����û�п�\n");
				}
				else if (gy->wdNumOnline >= (maxGatewayUser - 10))
				{
					task->LoginReturn(Cmd::LOGIN_RETURN_USERMAX);
					Zebra::logger->error("�û�����,��ǰ����%d",gy->wdNumOnline);
				}
				else
				{
					CEncrypt                      e;
					Cmd::FL::t_NewSession_Session tCmd;

					// [ranqd delete] δ�ɹ���½���û����������ȴ����ط���
				//	gy->wdNumOnline++;

					tCmd.session             = session;
					tCmd.session.wdGatewayID = gy->wdServerID;

					//����des������Կ                
					e.random_key_des(&tCmd.session.des_key);
					Zebra::logger->info("������Կ:%02x %02x %02x %02x %02x %02x %02x %02x",tCmd.session.des_key[0],tCmd.session.des_key[1],tCmd.session.des_key[2],tCmd.session.des_key[3],tCmd.session.des_key[4],tCmd.session.des_key[5],tCmd.session.des_key[6],tCmd.session.des_key[7]);
					/*
					for (int i=0; i<sizeof(tCmd.session.des_key); i++)
					tCmd.session.des_key[i] = (DWORD)randBetween(0,255);
					*/
					bcopy(gy->pstrIP,tCmd.session.pstrIP,sizeof(tCmd.session.pstrIP),sizeof(tCmd.session.pstrIP));
					tCmd.session.wdPort = gy->wdPort;
					ServerManager::getInstance().broadcast(session.gameZone,&tCmd,sizeof(tCmd));
				}
				break;
			case 1:
				//�ʺŴ�������״̬
				task->LoginReturn(Cmd::LOGIN_RETURN_LOCK);
				Zebra::logger->error("�ʺ�������");
				break;
			case 4:
				//�ʺŴ��ڴ�����״̬
				task->LoginReturn(Cmd::LOGIN_RETURN_WAITACTIVE);
				Zebra::logger->error("�ʺŴ�����");
				break;
			}
			break;
		case SESSIONCHECK_DB_FAILURE:
			Zebra::logger->info("���ݿ����,��½ʧ��");
			task->LoginReturn(Cmd::LOGIN_RETURN_DB);
			break;
		case SESSIONCHECK_PWD_FAILURE:
			Zebra::logger->info("�˺��������,��½ʧ��");
			task->LoginReturn(Cmd::LOGIN_RETURN_PASSWORDERROR);
			break;
		}
	}
}

/**
* \brief ���ش�����뵽ָ���Ŀͻ���
* \param loginTempID ָ���Ŀͻ���������ʱ���
* \param retcode �����صĴ���
* \param tm ������Ϣ�Ժ��Ƿ�Ͽ�����,ȱʡ�ǶϿ�����
*/
void LoginManager::loginReturn(const DWORD loginTempID,const BYTE retcode,const bool tm)
{
	Zebra::logger->debug("LoginManager::loginReturn");
	mlock.lock();
	LoginTaskHashmap_iterator it = loginTaskSet.find(loginTempID);
	if (it != loginTaskSet.end())
		it->second->LoginReturn(retcode,tm);
	mlock.unlock();
}

/**
* \brief �������е�����Ԫ�ص��ûص�����
* \param cb �ص�����ʵ��
*/
void LoginManager::execAll(LoginTaskCallback &cb)
{
	Zebra::logger->debug("LoginManager::execAll");
	zMutex_scope_lock scope_lock(mlock);
	for(LoginTaskHashmap_iterator it = loginTaskSet.begin(); it != loginTaskSet.end(); ++it)
	{
		cb.exec(it->second);
	}
}

