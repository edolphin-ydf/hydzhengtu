//////////////////////////////////////////////////
/// @file : AccountCache.h
/// @brief : 账号缓存
/// @date:  2012/10/19
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __AccountCache_H__
#define __AccountCache_H__

#include "Common.h"
using namespace std;
////////////////////////////////////////////////////////////////
/// @struct Account
/// @brief  账号结构体
////////////////////////////////////////////////////////////////
struct Account
{
	uint32 AccountId;  //账号ID
	char* GMFlags;     //GM类型
	uint8 AccountFlags;//账号类型
	uint32 Banned;     //是否被禁
	uint8 SrpHash[20]; //密码
	string* UsernamePtr;//用户名
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
/// @brief 账号缓存管理
///
/// @note
class AccountMgr : public Singleton < AccountMgr >
{
public:

	/**
	* @brief 由于玩家可能通过网站等途径修改了用户信息，需要定时器，定时加载更新用户信息
	* @param 参数一 是否后台加载
	* @return 返回值 void
	*/
	void ReloadAccounts(bool silent);

	Account* __GetAccount(string Name);
protected:
	Mutex setBusy;
};
#endif