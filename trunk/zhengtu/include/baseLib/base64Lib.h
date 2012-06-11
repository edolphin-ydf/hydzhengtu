#ifndef _INC_BASE64LIB_H_
#define _INC_BASE64LIB_H_

#include <baseLib/platForm.h>

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

DWORD Base64_Encode(PBYTE pbRaw,DWORD dwRawSize,PSTR szBase64,DWORD dwBase64,DWORD dwLine);
DWORD Base64_Decode(PSTR szBase64,DWORD dwBase64,PBYTE pbRaw,DWORD dwRawSize);

PSTR Base64_Encode_Data(PBYTE szData,DWORD cbData,DWORD dwLine);

PSTR Base64_Encode_DataAuto(PBYTE szData,DWORD cbData,DWORD dwLine,PSTR *pszAlloc);

PBYTE Base64_Decode_Data(PSTR szB64,DWORD cbB64,PDWORD pcbData);

BOOL Base64_CheckData(PSTR szB64,DWORD cbB64);

#define BASE64_BAD_CHAR		-1
#define BASE64_SPACE_CHAR	-2
#define BASE64_END_CHAR		-3

int Base64_DecodeChar(BYTE cChar);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_BASE64LIB_H_

