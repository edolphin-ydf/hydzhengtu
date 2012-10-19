
//////////////////////////////////////////////////////////////////////////
///  Copyright(c) 1999-2012,TQ Digital Entertainment, All Right Reserved
///  Author:      hyd
///  Create:      2012/10/19
///
/// @file         AccountCache.cpp
/// @brief        账号缓存管理
/// @version      1.0
//////////////////////////////////////////////////////////////////////////

#include "AccountCache.h"
#include "r_common.h"

initialiseSingleton(AccountMgr);

void AccountMgr::ReloadAccounts( bool silent )
{
	setBusy.Acquire();
	if(!silent) sLog.outString("[AccountMgr] Reloading Accounts...");

	/** @brief 加载最后登录的1000用户 */
	QueryResult* result = sCacheSQL->Query("SELECT acct, login, password, encrypted_password, gm, flags, banned, forceLanguage, muted FROM accounts");
	Field* field;
	string AccountName;
	set<string> account_list;
	Account* acct;

	if(result)
	{
		do
		{
			//field = result->Fetch();
			//AccountName = field[1].GetString();

			//// 把用户名转为大写
			//mnet_TOUPPER(AccountName);

			////Use private __GetAccount, for locks
			//acct = __GetAccount(AccountName);
			//if(acct == 0)
			//{
			//	// New account.
			//	AddAccount(field);
			//}
			//else
			//{
			//	// Update the account with possible changed details.
			//	UpdateAccount(acct, field);
			//}

			//// add to our "known" list
			//account_list.insert(AccountName);

		}
		while(result->NextRow());

		delete result;
	}

	// check for any purged/deleted accounts
//#ifdef WIN32
//	HM_NAMESPACE::hash_map<string, Account*>::iterator itr = AccountDatabase.begin();
//	HM_NAMESPACE::hash_map<string, Account*>::iterator it2;
//#else
//	std::map<string, Account*>::iterator itr = AccountDatabase.begin();
//	std::map<string, Account*>::iterator it2;
//#endif
//
//	for(; itr != AccountDatabase.end();)
//	{
//		it2 = itr;
//		++itr;
//
//		if(account_list.find(it2->first) == account_list.end())
//		{
//			delete it2->second;
//			AccountDatabase.erase(it2);
//		}
//		else
//		{
//			it2->second->UsernamePtr = (std::string*)&it2->first;
//		}
//	}
//
//	if(!silent) sLog.outString("[AccountMgr] Found %u accounts.", AccountDatabase.size());
	setBusy.Release();
}
