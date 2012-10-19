//////////////////////////////////////////////////
/// @file : AccountCache.h
/// @brief : �˺Ż���
/// @date:  2012/10/19
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __AccountCache_H__
#define __AccountCache_H__

#include "Common.h"
using namespace std;
////////////////////////////////////////////////////////////////
/// @struct Account
/// @brief  �˺Žṹ��
////////////////////////////////////////////////////////////////
struct Account
{
	uint32 AccountId;  //�˺�ID
	char* GMFlags;     //GM����
	uint8 AccountFlags;//�˺�����
	uint32 Banned;     //�Ƿ񱻽�
	uint8 SrpHash[20]; //����
	string* UsernamePtr;//�û���
	uint32 Muted;

	Account()
	{
		GMFlags = NULL;
	}

	~Account()
	{
		delete [] GMFlags;
	}

	void SetGMFlags(const char* flags)
	{
		delete [] GMFlags;

		size_t len = strlen(flags);
		if(len == 0 || (len == 1 && flags[0] == '0'))
		{
			// no flags
			GMFlags = NULL;
			return;
		}

		GMFlags = new char[len + 1];
		memcpy(GMFlags, flags, len);
		GMFlags[len] = 0;
	}

	char Locale[4];
	bool forcedLocale;

};

////////////////////////////////////////////////////////////////
/// @class AccountMgr
/// @brief �˺Ż������
///
/// @note
class AccountMgr : public Singleton < AccountMgr >
{
public:

	/**
	* @brief ������ҿ���ͨ����վ��;���޸����û���Ϣ����Ҫ��ʱ������ʱ���ظ����û���Ϣ
	* @param ����һ �Ƿ��̨����
	* @return ����ֵ void
	*/
	void ReloadAccounts(bool silent);

	Account* __GetAccount(string Name);
protected:
	Mutex setBusy;
};
#endif