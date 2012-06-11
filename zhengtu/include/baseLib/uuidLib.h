#ifndef _INC_UUIDLIB_H
#define _INC_UUIDLIB_H

#include <baseLib/platForm.h>

#ifdef HAVE_UUID
#include <uuid/uuid.h>

typedef struct{
  DWORD Data1;
  WORD  Data2;
  WORD  Data3;
  BYTE  Data4[8];
}GUID;

#endif //HAVE_UUID

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#ifdef WORDS_BIGENDIAN
//¸ß×Ö½ÚÐò(Motolora)
#define L2_INTEL(l)		(((l & 0xFF000000) >> 24) | ((l & 0x000000FF) << 24) | ((l & 0x00FF0000) >> 8) | ((l & 0x0000FF00) << 8))
#define S2_INTEL(s)		(((s & 0xFF00) >> 8) | ((s & 0x00FF) << 8))
#else //!WORDS_BIGENDIAN

//µÍ×Ö½ÚÐò(Intel)
#define L2_INTEL(l)		(l)
#define S2_INTEL(s)		(s)
#endif //!WORDS_BIGENDIAN

#ifdef _LOCAL_UUID_
#define UUIDLIB_DEFINE(vName,vl,vw1,vw2,vb1,vb2,vb3,vb4,vb5,vb6,vb7,vb8) GUID vName={L2_INTEL(vl),S2_INTEL(vw1),S2_INTEL(vw2),{vb1,vb2,vb3,vb4,vb5,vb6,vb7,vb8}}
#else //!_LOCAL_UUID_
#define UUIDLIB_DEFINE(vName,vl,vw1,vw2,vb1,vb2,vb3,vb4,vb5,vb6,vb7,vb8) extern GUID vName
#endif //!_LOCAL_UUID_

void UUID_Generate_Random(GUID *uuid);
void UUID_ToString(GUID *uuid,PSTR szUUID);

BOOL UUID_IsEqual(GUID *uuid0,GUID *uuid1);
BOOL UUID_IsEmpty(GUID *uuid);
BOOL UUID_FromString(PSTR szUUID,GUID *uuid);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_UUIDLIB_H
