#ifndef _INC_FILELIB_H_
#define _INC_FILELIB_H_

#include <baseLib/platForm.h>
#ifdef WIN32
#include <shlwapi.h>
#endif //WIN32

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#ifdef WIN32
BOOL fileSelectPath(HWND HWindow,PSTR szTitle,PSTR szPath);
BOOL fileDeleteDirectory(PSTR szPath);

void fileSplitPathName(PSTR szFull,PSTR szPath,PSTR szName);

typedef BOOL (*PFN_WalkName)(PVOID usrData,PSTR szWalkPath,LPWIN32_FIND_DATA pWFD);

BOOL fileWalkOverPath(PSTR szWalkPath,PSTR szPattern,BOOL bSubPath,PFN_WalkName WalkName,PVOID usrData);
#endif //WIN32

#ifndef WIN32
void PathAddBackslash(PSTR szPath);
void PathRenameExtension(PSTR szPath,PSTR szExt);
PSTR PathFindExtension(PSTR szPath);
PSTR PathFindFileName(PSTR szPath);

BOOL PathIsRelative(PSTR szPath);
#endif //!WIN32

typedef enum{
  PF_FullPath,
  PF_PathName,
  PF_NameOnly
}EPathFlag;

void PathChangeIrregularChar(PSTR szPath,EPathFlag ePF,char cUse);

void fileCutPathSplit(PSTR szPath);
void fileAdjustPathSplit(PSTR szPath);
void fileAdjustPathSplitEx(PSTR szPath,char chSplit);

BOOL fileEnsurePathExist(PSTR szPath);
BOOL fileEnsureNameCanSave(PSTR szName);

int fileGetSize(FILE *fp);

BOOL fileMove(PSTR szOld,PSTR szNew);

void fileGetSamePathFileName(PSTR szArgv0,PSTR szName,PSTR szPath);
void fileGetSameNameFileName(PSTR szArgv0,PSTR szExt,PSTR szPath);

PSTR fileLoad(PSTR szName,DWORD dwHead,PDWORD pdwSize,PFN_EncryptDecrypt EncryptDecrypt,PVOID pKey);

PWSTR fileLoadW(PSTR szName);

BOOL fileSave(PSTR szName,PVOID pData,DWORD dwData);
BOOL fileSaveEx(PSTR szName,...);
BOOL fileSaveAsCode(PSTR szName,PBYTE pData,DWORD dwData,PSTR szVar);

#ifdef WIN32
BOOL fileSaveForce(PSTR szName,PSTR szTempName,PBYTE pData,DWORD cbData);
BOOL fileAutoRename(PSTR szName,DWORD dwName);

PBYTE fileBeginRead(PSTR szName,HANDLE *phFile,PDWORD pdwSize,HANDLE *phMap);
PBYTE fileBeginWrite(PSTR szName,HANDLE *phFile,HANDLE *phMap,DWORD dwMaxSize);
void fileEndAccess(PBYTE pBase,PBYTE pEnd,HANDLE hMap,HANDLE hFile);
#endif //WIN32

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_FILELIB_H_
