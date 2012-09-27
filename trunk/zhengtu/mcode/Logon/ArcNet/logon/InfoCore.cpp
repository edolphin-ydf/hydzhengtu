#include "LogonStdAfx.h"
initialiseSingleton(AccountMgr);
initialiseSingleton(IPBanner);
initialiseSingleton(InformationCore);

void AccountMgr::ReloadAccounts(bool silent)
{
	setBusy.Acquire();
	if(!silent) sLog.outString("[AccountMgr] Reloading Accounts...");

	if(!silent) sLog.outString("[AccountMgr] Found %u accounts.", AccountDatabase.size());
	setBusy.Release();

	IPBanner::getSingleton().Reload();
}

void AccountMgr::AddAccount()
{
	Account* acct = new Account;
	Sha1Hash hash;
	string Username = "test";
	string Password = "test";
	acct->AccountId				= 1;
	acct->AccountFlags			= 1;
	acct->Banned				= 1;
	if((uint32)UNIXTIME > acct->Banned && acct->Banned != 0 && acct->Banned != 1)   //1 = perm ban?
	{
		//Accounts should be unbanned once the date is past their set expiry date.
		acct->Banned = 0;
		//me go boom :(
		//printf("Account %s's ban has expired.\n",acct->UsernamePtr->c_str());
		//sLogonSQL->Execute("UPDATE accounts SET banned = 0 WHERE acct=%u", acct->AccountId);
	}
	acct->SetGMFlags("test");
	acct->Locale[0] = 'e';
	acct->Locale[1] = 'n';
	acct->Locale[2] = 'U';
	acct->Locale[3] = 'S';
	acct->forcedLocale = false;

	acct->Muted = 1;
	if((uint32)UNIXTIME > acct->Muted && acct->Muted != 0 && acct->Muted != 1)   //1 = perm ban?
	{
		//Accounts should be unbanned once the date is past their set expiry date.
		acct->Muted = 0;
		//LOG_DEBUG("Account %s's mute has expired.",acct->UsernamePtr->c_str());
		//sLogonSQL->Execute("UPDATE accounts SET muted = 0 WHERE acct=%u", acct->AccountId);
	}
	// Convert username/password to uppercase. this is needed ;)
	arcemu_TOUPPER(Username);
	arcemu_TOUPPER(Password);

	// prefer encrypted passwords over nonencrypted
	// Prehash the I value.
	hash.UpdateData((Username + ":" + Password));
	hash.Finalize();
	memcpy(acct->SrpHash, hash.GetDigest(), 20);
	AccountDatabase[Username] = acct;
}

void AccountMgr::UpdateAccount(Account* acct)
{
	
}

void AccountMgr::ReloadAccountsCallback()
{
	ReloadAccounts(true);
}
BAN_STATUS IPBanner::CalculateBanStatus(in_addr ip_address)
{
	Guard lguard(listBusy);
	list<IPBan>::iterator itr;
	list<IPBan>::iterator itr2 = banList.begin();
	for(; itr2 != banList.end();)
	{
		itr = itr2;
		++itr2;

		if(ParseCIDRBan(ip_address.s_addr, itr->Mask, itr->Bytes))
		{
			// ban hit
			if(itr->Expire == 0)
				return BAN_STATUS_PERMANENT_BAN;

			if((uint32)UNIXTIME >= itr->Expire)
			{
				//sLogonSQL->Execute("DELETE FROM ipbans WHERE expire = %u AND ip = \"%s\"", itr->Expire, sLogonSQL->EscapeString(itr->db_ip).c_str());
				banList.erase(itr);
			}
			else
			{
				return BAN_STATUS_TIME_LEFT_ON_BAN;
			}
		}
	}

	return BAN_STATUS_NOT_BANNED;
}

bool IPBanner::Add(const char* ip, uint32 dur)
{
	string sip = string(ip);

	string::size_type i = sip.find("/");
	if(i == string::npos)
		return false;

	string stmp = sip.substr(0, i);
	string smask = sip.substr(i + 1);

	unsigned int ipraw = MakeIP(stmp.c_str());
	unsigned int ipmask = atoi(smask.c_str());
	if(ipraw == 0 || ipmask == 0)
		return false;

	IPBan ipb;
	ipb.db_ip = sip;
	ipb.Bytes = static_cast<unsigned char>(ipmask);
	ipb.Mask = ipraw;
	ipb.Expire = dur;

	listBusy.Acquire();
	banList.push_back(ipb);
	listBusy.Release();

	return true;
}

InformationCore::~InformationCore()
{
	for(map<uint32, Realm*>::iterator itr = m_realms.begin(); itr != m_realms.end(); ++itr)
		delete itr->second;
}

bool IPBanner::Remove(const char* ip)
{
	listBusy.Acquire();
	for(list<IPBan>::iterator itr = banList.begin(); itr != banList.end(); ++itr)
	{
		if(!strcmp(ip, itr->db_ip.c_str()))
		{
			banList.erase(itr);
			listBusy.Release();
			return true;
		}
	}
	listBusy.Release();
	return false;
}

void IPBanner::Reload()
{
	listBusy.Acquire();
	//TODO
	listBusy.Release();
}

Realm* InformationCore::AddRealm(uint32 realm_id, Realm* rlm)
{
	realmLock.Acquire();
	map<uint32, Realm*>::iterator itr = m_realms.find(realm_id);

	if(itr == m_realms.end())
		m_realms.insert(make_pair(realm_id, rlm));
	else
	{
		delete itr->second;
		itr->second = rlm;
	}
	realmLock.Release();
	return rlm;
}

Realm* InformationCore::GetRealm(uint32 realm_id)
{
	Realm* ret = NULL;

	realmLock.Acquire();
	map<uint32, Realm*>::iterator itr = m_realms.find(realm_id);
	if(itr != m_realms.end())
	{
		ret = itr->second;
	}
	realmLock.Release();
	return ret;
}

int32 InformationCore::GetRealmIdByName(string Name)
{
	map<uint32, Realm*>::iterator itr = m_realms.begin();
	for(; itr != m_realms.end(); ++itr)
		if(itr->second->Name == Name)
		{
			return itr->first;
		}
	return -1;
}

void InformationCore::RemoveRealm(uint32 realm_id)
{
	realmLock.Acquire();
	map<uint32, Realm*>::iterator itr = m_realms.find(realm_id);
	if(itr != m_realms.end())
	{
		delete itr->second;
		m_realms.erase(itr);
	}
	realmLock.Release();
}

void InformationCore::UpdateRealmStatus(uint32 realm_id, uint8 flags)
{
	realmLock.Acquire();
	map<uint32, Realm*>::iterator itr = m_realms.find(realm_id);
	if(itr != m_realms.end())
	{
		itr->second->flags = flags;
	}
	realmLock.Release();
}

void InformationCore::UpdateRealmPop(uint32 realm_id, float pop)
{
	realmLock.Acquire();
	map<uint32, Realm*>::iterator itr = m_realms.find(realm_id);
	if(itr != m_realms.end())
	{
		uint8 flags;
		if(pop >= 3)
			flags =  REALM_FLAG_FULL | REALM_FLAG_INVALID; // Full
		else if(pop >= 2)
			flags = REALM_FLAG_INVALID; // Red
		else if(pop >= 0.5)
			flags = 0; // Green
		else
			flags = REALM_FLAG_NEW_PLAYERS; // recommended

		itr->second->Population = (pop > 0) ? (pop >= 1) ? (pop >= 2) ? 2.0f : 1.0f : 0.0f : 0.0f;
		itr->second->flags = flags;
	}
	realmLock.Release();
}
void InformationCore::SendRealms(AuthSocket* Socket)
{
	realmLock.Acquire();

	// packet header
	ByteBuffer data(m_realms.size() * 150 + 20);
	data << uint8(0x10);
	data << uint16(0);	  // Size Placeholder

	// dunno what this is..
	data << uint32(0);

	//sAuthLogonChallenge_C * client = Socket->GetChallenge();
	data << uint16(m_realms.size());

	// loop realms :/
	map<uint32, Realm*>::iterator itr = m_realms.begin();
	HM_NAMESPACE::hash_map<uint32, uint8>::iterator it;
	for(; itr != m_realms.end(); ++itr)
	{
//		data << uint8(itr->second->Icon);
//		data << uint8(0);				   // Locked Flag
//		data << uint8(itr->second->Colour);
		data << uint8(itr->second->Icon);
		data << uint8(itr->second->Lock);		// delete when using data << itr->second->Lock;
		data << uint8(itr->second->flags);

		// This part is the same for all.
		data << itr->second->Name;
		data << itr->second->Address;
//		data << uint32(0x3fa1cac1);
		data << float(itr->second->Population);

		/* Get our character count */
		it = itr->second->CharacterMap.find(Socket->GetAccountID());
		data << uint8((it == itr->second->CharacterMap.end()) ? 0 : it->second);
//		data << uint8(1);   // time zone
//		data << uint8(6);
		data << uint8(itr->second->TimeZone);
		data << uint8(GetRealmIdByName(itr->second->Name));        //Realm ID
	}
	data << uint8(0x17);
	data << uint8(0);

	realmLock.Release();

	// Re-calculate size.

	*(uint16*)&data.contents()[1] = uint16(data.size() - 3);

	// Send to the socket.
	Socket->Send((const uint8*)data.contents(), uint32(data.size()));

	std::list< LogonCommServerSocket* > ss;
	std::list< LogonCommServerSocket* >::iterator SSitr;

	ss.clear();

	serverSocketLock.Acquire();

	if(m_serverSockets.empty())
	{
		serverSocketLock.Release();
		return;
	}

	set<LogonCommServerSocket*>::iterator itr1;

	// We copy the sockets to a list and call RefreshRealmsPop() from there because if the socket is dead,
	//then calling the method deletes the socket and removes it from the set corrupting the iterator and causing a crash!
	for(itr1 = m_serverSockets.begin(); itr1 != m_serverSockets.end(); ++itr1)
	{
		ss.push_back(*itr1);
	}

	serverSocketLock.Release();

	for(SSitr = ss.begin(); SSitr != ss.end(); ++SSitr)
		(*SSitr)->RefreshRealmsPop();

	ss.clear();
}

void InformationCore::TimeoutSockets()
{
	if(!usepings)
		return;

	uint32 now = uint32(time(NULL));

	/* burlex: this is vulnerable to race conditions, adding a mutex to it. */
	serverSocketLock.Acquire();

	for(set< LogonCommServerSocket* >::iterator itr = m_serverSockets.begin(); itr != m_serverSockets.end();)
	{
		LogonCommServerSocket* s = *itr;
		++itr;

		uint32 last_ping = s->last_ping.GetVal();
		if(last_ping < now && ((now - last_ping) > 300))
		{
			for(set< uint32 >::iterator RealmITR = s->server_ids.begin(); RealmITR != s->server_ids.end(); ++RealmITR)
			{
				uint32 RealmID = *RealmITR;

				SetRealmOffline(RealmID);
			}

			s->removed = true;
			m_serverSockets.erase(s);
			s->Disconnect();
		}
	}

	serverSocketLock.Release();
}

void InformationCore::CheckServers()
{
	serverSocketLock.Acquire();

	set<LogonCommServerSocket*>::iterator itr, it2;
	LogonCommServerSocket* s;
	for(itr = m_serverSockets.begin(); itr != m_serverSockets.end();)
	{
		s = *itr;
		it2 = itr;
		++itr;

		/*if(!IsServerAllowed(s->GetRemoteAddress().s_addr))
		{
		LOG_DETAIL("Disconnecting socket: %s due to it no longer being on an allowed IP.", s->GetRemoteIP().c_str());
		s->Disconnect();
		}*/
	}

	serverSocketLock.Release();
}

void InformationCore::SetRealmOffline(uint32 realm_id)
{
	realmLock.Acquire();
	map<uint32, Realm*>::iterator itr = m_realms.find(realm_id);
	if(itr != m_realms.end())
	{
		itr->second->flags = REALM_FLAG_OFFLINE | REALM_FLAG_INVALID;
		itr->second->CharacterMap.clear();
		Log.Notice("InfoCore", "Realm %u is now offline (socket close).", realm_id);
	}
	realmLock.Release();
}
