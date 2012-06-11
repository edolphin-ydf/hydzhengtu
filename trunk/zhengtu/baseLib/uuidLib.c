#include <baseLib/uuidLib.h>

#ifdef WIN32
#pragma comment(lib,"ws2_32")
#pragma comment(lib,"rpcrt4")
#endif //WIN32

void UUID_Generate_Random(GUID *uuid)
{
#ifdef WIN32
  UuidCreate(uuid);
#endif //WIN32
  //uuid->Data1 = htonl(uuid->Data1);
  //uuid->Data2 = htons(uuid->Data2);
  //uuid->Data3 = htons(uuid->Data3);
#ifdef HAVE_UUID_GENERATE
  uuid_generate(uuid);
#endif //HAVE_UUID_GENERATE
}

BOOL UUID_IsEqual(GUID *uuid0,GUID *uuid1)
{  
  return 0 == memcmp(uuid0,uuid1,sizeof(GUID));
}

BOOL UUID_IsEmpty(GUID *uuid)
{
  GUID uuid0;

  if (NULL == uuid) return TRUE;
  memset(&uuid0,0,sizeof(uuid0));
  return 0 == memcmp(&uuid0,uuid,sizeof(GUID));
}

void UUID_ToString(GUID *uuid,PSTR szUUID)
{
#ifdef WIN32
  PBYTE StringUuid;

  UuidToString(uuid,&StringUuid);
  strcpy(szUUID,StringUuid);
  RpcStringFree(&StringUuid);
#endif //WIN32
#ifdef HAVE_UUID_UNPARSE
  uuid_unparse(*uuid,szUUID);
#endif //HAVE_UUID_UNPARSE
  strupr(szUUID);
}

BOOL UUID_FromString(PSTR szUUID,GUID *uuid)
{
#ifdef WIN32
  return RPC_S_OK == UuidFromString(szUUID,uuid);
#endif //WIN32
#ifdef HAVE_UUID_PARSE
  return uuid_parse(szUUID,uuid);
#endif //HAVE_UUID_PARSE
}
