

#ifndef _WOWCRYPT_H
#define _WOWCRYPT_H

#include <cstdlib>
#include "../Common.h"
#include "BigNumber.h"
#include <vector>
#include <openssl/sha.h>
#include <openssl/rc4.h>

class PacketCrypt
{
	public:
		PacketCrypt(uint8* K = NULL);
		~PacketCrypt();

		void Init(uint8* K);
		MNET_INLINE void DecryptRecv(uint8* pData, size_t len) { if(!m_initialized) { return; } RC4(&m_clientDecrypt, (unsigned long)len, pData, pData); }
		MNET_INLINE void EncryptSend(uint8* pData, size_t len) { if(!m_initialized) { return; } RC4(&m_serverEncrypt, (unsigned long)len, pData, pData); }
		bool IsInitialized() { return m_initialized; }

	private:
		RC4_KEY m_clientDecrypt;
		RC4_KEY m_serverEncrypt;
		bool m_initialized;
};

#endif
