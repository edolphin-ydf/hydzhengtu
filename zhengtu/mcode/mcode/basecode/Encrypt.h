/*
 文件名 : Encrypt.h
 创建时间 : 2012/9/19
 作者 : hyd
 功能 : 加密解密
*/
#ifndef __Encrypt_H__
#define __Encrypt_H__
#include "Common.h"

typedef BYTE ZES_cblock[8];
class CEncrypt
{
//public:
//	CEncrypt();
//	enum encMethod
//	{
//		ENCDEC_NONE,
//		ENCDEC_DES,
//		ENCDEC_RC5
//	};
//	void random_key_des(ZES_cblock *ret);
//	void set_key_des(ZES_cblock *key);
//	void set_key_rc5(const BYTE *data,int nLen,int rounds);
//	int encdec(void *data,DWORD nLen,bool enc);
//
//	void setEncMethod(encMethod method);
//	encMethod getEncMethod() const;
//
//private:
//	void ZES_random_key(ZES_cblock *ret);
//	void ZES_set_key(ZES_cblock *key,ZES_key_schedule *schedule);
//	void ZES_encrypt1(DWORD *data,ZES_key_schedule *ks,int enc);
//
//	void RC5_32_set_key(RC5_32_KEY *key,int len,const BYTE *data,int rounds);
//	void RC5_32_encrypt(RC5_32_INT *d,RC5_32_KEY *key);
//	void RC5_32_decrypt(RC5_32_INT *d,RC5_32_KEY *key);
//
//	int encdec_des(BYTE *data,DWORD nLen,bool enc);
//	int encdec_rc5(BYTE *data,DWORD nLen,bool enc);
//
//	ZES_key_schedule key_des;
//	RC5_32_KEY key_rc5;
//	bool haveKey_des;
//	bool haveKey_rc5;
//	encMethod method;
};
#endif
