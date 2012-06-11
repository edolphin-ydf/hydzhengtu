#include <baseLib/logLib.h>
#include <baseLib/base64Lib.h>
#include <baseLib/strLib.h>
#include <baseLib/memLib.h>

static void checkLine(PBYTE *ppChar,DWORD *numCharsOnLine,DWORD dwLine);

DWORD Base64_Encode(PBYTE pbRaw,DWORD dwRawSize,PSTR szBase64,DWORD dwBase64,DWORD dwLine)
{
static char szBase64Char[]={
  'A','B','C','D','E','F','G','H','I','J','K','L','M',
  'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  'a','b','c','d','e','f','g','h','i','j','k','l','m',
  'n','o','p','q','r','s','t','u','v','w','x','y','z',
  '0','1','2','3','4','5','6','7','8','9','+','/'
};

  DWORD i,dwLength,numCharsOnLine;
  PBYTE pChar;

  if (-1 == dwRawSize) dwRawSize = strlen(pbRaw);
  if (NULL == szBase64){
    dwLength = (dwRawSize/3) * 4;
    if ((dwRawSize % 3) != 0) dwLength += 4;
    if (0 != dwLine) dwLength += (((dwLength+dwLine)/dwLine)-1)*2;  //2 for CR LF
    return dwLength;
  }
  pChar    = (PBYTE)szBase64;
  dwLength = (dwRawSize/3)*3;
  /* loop once for each 24-bit chunk,assuming an integral number of 24-bit chunks */
  for(i=numCharsOnLine=0;i<dwLength;i+=3){
    /* break up the 24-bits into four 6-bit chunks and assign characters */
    *pChar++ = szBase64Char[(pbRaw[i]>>2) & 0x3F];
	checkLine(&pChar,&numCharsOnLine,dwLine);
    
	*pChar++ = szBase64Char[((pbRaw[i]<<4) & 0x30) + ((pbRaw[i+1]>>4) & 0x0F)];
	checkLine(&pChar,&numCharsOnLine,dwLine);

    *pChar++ = szBase64Char[((pbRaw[i+1]<<2) & 0x3C) + ((pbRaw[i+2]>>6) & 0x03)];
	checkLine(&pChar,&numCharsOnLine,dwLine);

    *pChar++ = szBase64Char[pbRaw[i+2] & 0x3F];
	checkLine(&pChar,&numCharsOnLine,dwLine);
  }
  /* calculate final characters and add padding if needed */
  switch(dwRawSize % 3){
    case 1:
         *pChar++ = szBase64Char[(pbRaw[i]>>2) & 0x3F];
		 checkLine(&pChar,&numCharsOnLine,dwLine);

         *pChar++ = szBase64Char[(pbRaw[i]<<4) & 0x30];
		 checkLine(&pChar,&numCharsOnLine,dwLine);

         *pChar++ = '=';
		 checkLine(&pChar,&numCharsOnLine,dwLine);

         *pChar++ = '=';
		 checkLine(&pChar,&numCharsOnLine,dwLine);
		 break;
	case 2:
         *pChar++ = szBase64Char[(pbRaw[i]>>2) & 0x3F];
		 checkLine(&pChar,&numCharsOnLine,dwLine);

         *pChar++ = szBase64Char[((pbRaw[i]<<4) & 0x30) + ((pbRaw[i+1]>>4) & 0x0F)];
		 checkLine(&pChar,&numCharsOnLine,dwLine);

         *pChar++ = szBase64Char[(pbRaw[i+1]<<2) & 0x3C];
		 checkLine(&pChar,&numCharsOnLine,dwLine);

         *pChar++ = '='; 
		 checkLine(&pChar,&numCharsOnLine,dwLine);
		 break;
  }
  *pChar = 0;
  return pChar - (PBYTE)szBase64;
}

/* This function performs base64 decoding */
DWORD Base64_Decode(PSTR szBase64,DWORD dwBase64,PBYTE pbRaw,DWORD dwRawSize)
{
  int   v;
  DWORD i,j,k,n;
  DWORD dw;

  if (-1 == dwBase64) dwBase64 = strlen(szBase64);
  //calculate how many valid charcaters are there in the encoded data
  for(i=n=0;i<dwBase64;i++){
	if (BASE64_BAD_CHAR == (v=Base64_DecodeChar(szBase64[i]))) return -1;
	if (BASE64_END_CHAR == v) break;
	if (BASE64_SPACE_CHAR == v) continue;
	n++;
  }
  if (NULL == pbRaw) return n * 3 / 4;
  //decode input data
  for(dw=i=n=k=0;i<dwBase64 && n<dwRawSize;i++){
    v = Base64_DecodeChar(szBase64[i]);
	if (BASE64_SPACE_CHAR == v) continue;
	if (BASE64_END_CHAR != v){
	  dw <<= 6;
      dw  |= v;
	  k++;
    }
	if (4 == k || BASE64_END_CHAR == v){
	  /* if less than 4 characters were decoded,shift the value to the
      left up to the 2nd byte of DWORD and pad the rest bits with zero's */
      dw <<= (24 - k * 6);
      k    = ((k*3)/4);
      /* copy decoded value to the output buffer starting
      from the 2nd bytes of decoded value */
      for(j=0;j<k && n<dwRawSize;j++,n++){
        pbRaw[n] = (BYTE)((dw >> (16-j*8)) & 0xFF);
      }
	  if (BASE64_END_CHAR == v) break;
	  k  = 0;
	  dw = 0;
    }
  }
  return n;
}

PSTR Base64_Encode_Data(PBYTE szData,DWORD cbData,DWORD dwLine)
{
  PSTR szB64;
  int  cbB64;

  __API_ENTER("Base64_Encode_Data",PSTR,NULL);
  szB64 = NULL;
  if (-1 == (cbB64=Base64_Encode(szData,cbData,NULL,0,dwLine))) __API_FINISH();
  if (NULL == (szB64=MemCalloc(++cbB64,1))) __API_FINISH();
  if (-1 == Base64_Encode(szData,cbData,szB64,cbB64,dwLine)) __API_FINISH()
  __API_PTR_TRANS(retCode,szB64);
__API_END_POINT:
  MemFree(szB64);
  __API_LEAVE("Base64_Encode_Data");
}

PBYTE Base64_Decode_Data(PSTR szB64,DWORD cbB64,PDWORD pcbData)
{
  int   cbData;
  PBYTE pbData;

  __API_ENTER("Base64_Decode_Data",PBYTE,NULL);
  pbData = NULL;
  if (-1 == (cbData=Base64_Decode(szB64,cbB64,NULL,0))) __API_FINISH();
  if (NULL == (pbData=MemCalloc(++cbData,1))) __API_FINISH();
  if (-1 == (cbData=Base64_Decode(szB64,cbB64,pbData,cbData))) __API_FINISH();
  __API_PTR_TRANS(retCode,pbData);
  if (NULL != pcbData) *pcbData = cbData;
__API_END_POINT:
  MemFree(pbData);
  __API_LEAVE("Base64_Decode_Data");
}

BOOL Base64_CheckData(PSTR szB64,DWORD cbB64)
{
  DWORD i,v;

  if (-1 == cbB64) cbB64 = strlen(szB64);
  for(i=0;i<cbB64;i++){
  	if (0 == szB64[i]){
	  for(;i<cbB64;i++){
	    if (0 != szB64[i]) break;
	  }
	  if (i != cbB64) return FALSE;
	  break;
	}
	if (BASE64_BAD_CHAR == (v=Base64_DecodeChar(szB64[i]))) return FALSE;
	if (BASE64_SPACE_CHAR == v) continue;
  }
  return TRUE;
}

static void checkLine(PBYTE *ppChar,DWORD *numCharsOnLine,DWORD dwLine)
{
  if (0 == dwLine) return;
  (*numCharsOnLine)++;
  if (*numCharsOnLine < dwLine) return;
  *(*ppChar)++ = '\r';
  *(*ppChar)++ = '\n';
  *numCharsOnLine = 0;
}

int Base64_DecodeChar(BYTE cData) 
{
static char szBase64Char[]={
  BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_SPACE_CHAR,BASE64_SPACE_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_SPACE_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,
  BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,
  BASE64_SPACE_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,0x3e,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,0x3f,
  0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_END_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,
  BASE64_BAD_CHAR,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
  0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,
  BASE64_BAD_CHAR,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
  0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR,BASE64_BAD_CHAR
};

  if (cData >= SIZEOF_ARRAY(szBase64Char)) return BASE64_BAD_CHAR;
  return szBase64Char[cData];
}

PSTR Base64_Encode_DataAuto(PBYTE pData,DWORD cbData,DWORD dwLine,PSTR *pszAlloc)
{
  *pszAlloc = NULL;
  if (Base64_CheckData((PSTR)pData,cbData)) return (PSTR)pData;
  *pszAlloc = Base64_Encode_Data(pData,cbData,dwLine);
  return *pszAlloc;
}
