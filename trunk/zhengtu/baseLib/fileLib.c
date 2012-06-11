#include <baseLib/fileLib.h>
#include <baseLib/memLib.h>
#include <errno.h>
#ifdef WIN32
#include <direct.h>
#include <shlobj.h>
#endif //WIN32
#include <sys/stat.h>
#include <fcntl.h>
#include <baseLib/logLib.h>
#include <baseLib/strLib.h>
#include <baseLib/codeLib.h>

#ifdef WIN32
#define CH_OS_PATH_SPLIT    '\\'
#define SZ_OS_PATH_SPLIT    "\\"
#else //!WIN32
#define CH_OS_PATH_SPLIT    '/'
#define SZ_OS_PATH_SPLIT    "/"
#endif //WIN32

#ifdef WIN32
#pragma comment(lib,"shell32")
#pragma comment(lib,"shlwapi")
#endif //WIN32

BOOL fileIsPathSplit(char chSplit)
{
  return '\\' == chSplit || '/' == chSplit;
}

void fileCutPathSplit(PSTR szPath)
{
  int size;

  if (0 == (size=strlen(szPath))) return;
  size--;
  if (!fileIsPathSplit(szPath[size])) return;
  szPath[size] = 0;
}

void fileAdjustPathSplitEx(PSTR szPath,char chSplit)
{
  int i;

  for(i=0;0!=szPath[i];i++){
    if (fileIsPathSplit(szPath[i])) szPath[i] = chSplit;
  }
}

void fileAdjustPathSplit(PSTR szPath)
{
  int i;

  for(i=0;0!=szPath[i];i++){
    if (fileIsPathSplit(szPath[i])) szPath[i] = CH_OS_PATH_SPLIT;
  }
}

void UrlAddPathSplit(PSTR szUrl)
{
  size_t size;

  if (0 == (size=strlen(szUrl))) return;
  if ('/' == szUrl[size-1]) return;
  szUrl[size++] = '/';
  szUrl[size]   = 0;
}

#ifndef WIN32
void PathAddBackslash(PSTR szPath)
{
  int size;

  if (0 == (size=strlen(szPath))) return;
  if (fileIsPathSplit(szPath[size-1])) return;
  szPath[size++] = CH_OS_PATH_SPLIT;
  szPath[size]   = 0;
}

PSTR PathFindExtension(PSTR szPath)
{
  PSTR pName,pExt;

  pName = PathFindFileName(szPath);
  if (NULL == (pExt=strrchr(pName,'.'))) return &pName[strlen(pName)];
  return pExt;
}

void PathRenameExtension(PSTR szPath,PSTR szExt)
{
  strcpy(PathFindExtension(szPath),szExt);
}

PSTR PathFindFileName(PSTR szPath)
{
  PSTR token;

  if (NULL == (token=strrchr(szPath,'/'))){
    if (NULL == (token=strrchr(szPath,'\\'))){
	  return szPath;
	}
  }
  return &token[1];
}

BOOL PathIsRelative(PSTR szPath)
{
  return '/' != szPath[0];
}
#endif //!WIN32

BOOL fileEnsurePathExist(PSTR szPath)
{
  char  chTemp;
  PSTR  token;
#ifdef WIN32
  DWORD dwAttr;
#endif //!WIN32

#ifdef WIN32
  if (INVALID_FILE_ATTRIBUTES != (dwAttr=GetFileAttributes(szPath))){
    return FLAGON(dwAttr,FILE_ATTRIBUTE_DIRECTORY);
  }
#endif //!WIN32
  //Skip "?:\"
  if (strlen(szPath) <= 3){
    logMessage1("!fileEnsurePathExist(%s)",szPath);
	return FALSE;
  }
  for(token=&szPath[3];;){
	for(;0 != *token && !fileIsPathSplit(*token);token++);
	chTemp  = *token;
	*token = 0;
#ifdef WIN32
	if (0 != _mkdir(szPath)){
#else //!WIN32
    if (0 != mkdir(szPath,0x777)){
#endif //!WIN32
	  if (EEXIST != errno){
	    logMessage3("!mkdir(%s) ==> %d != %d(EEXIST)",szPath,errno,EEXIST);
	    return FALSE;
	  }
	}
	if (0 == (*(token++)=chTemp)) break;
  }
  return TRUE;
}

BOOL fileEnsureNameCanSave(PSTR szName)
{
  char szPath[_MAX_PATH];
  PSTR token;

  strncpy(szPath,szName,sizeof(szPath));
  if (NULL == (token=strrchr(szPath,CH_OS_PATH_SPLIT))) return TRUE;
  *token = 0;
  return fileEnsurePathExist(szPath);
}

#ifndef WIN32
int lock_region(HANDLE hFile,int cmd,int type,off_t offset,int whence,off_t len)
{
  struct flock lock;

  lock.l_type   = type;
  lock.l_start  = offset;
  lock.l_whence = whence;
  lock.l_len    = len;
  return fcntl(hFile,cmd,&lock);
}
#endif //!WIN32

/*
BOOL fileReadLock(HANDLE hFile,int offset,int size,BOOL bWait)
{
#ifdef WIN32
  if (0 == size) size = GetFileSize(hFile,NULL) + 1024;
  return LockFile(hFile,offset,0,size,0);
#endif //WIN32
#ifndef WIN32
  return lock_region(hFile,bWait ? F_SETLKW : F_SETLK,F_RDLCK,offset,SEEK_SET,size) >= 0;
#endif //!WIN32
}

BOOL fileWriteLock(HANDLE hFile,int offset,int size,BOOL bWait)
{
#ifdef WIN32
  if (0 == size) size = GetFileSize(hFile,NULL) + 1024;
  return LockFile(hFile,offset,0,size,0);
#endif //WIN32
#ifndef WIN32
  return lock_region(hFile,bWait ? F_SETLKW : F_SETLK,F_WRLCK,offset,SEEK_SET,size) >= 0;
#endif //!WIN32
}

BOOL fileUnlock(HANDLE hFile,int offset,int size)
{
#ifdef WIN32
  return UnlockFile(hFile,offset,0,size,0);
#endif //WIN32
#ifndef WIN32
  return lock_region(hFile,F_SETLK,F_UNLCK,offset,SEEK_SET,size) >= 0;
#endif //!WIN32
}
*/

int fileGetSize(FILE *fp)
{
  long size,curPos;

  curPos = ftell(fp);
  fseek(fp,0,SEEK_END);
  size = ftell(fp);
  fseek(fp,curPos,SEEK_SET);
  return size;
}

BOOL fileMove(PSTR szOld,PSTR szNew)
{
  int  size;
  char chData[4096];
  FILE *fr,*fw;

  if (NULL == (fr=fopen(szOld,"rb"))) return FALSE;
  if (NULL == (fw=fopen(szNew,"rw"))){
    fclose(fr);
    return FALSE;
  }
  for(;;){
    size = fread(chData,1,sizeof(chData),fr);
	fwrite(chData,1,size,fw);
	if (size < sizeof(chData)) break;
  }
  fclose(fw);
  fclose(fr);
  _unlink(szOld);
  return TRUE;
}

void fileGetSamePathFileName(PSTR szArgv0,PSTR szName,PSTR szPath)
{
  if (PathIsRelative(szName)){
    strncpy(szPath,szArgv0,sizeof(szPath));
    strcpy(PathFindFileName(szPath),szName);
  }
  else{
    strncpy(szPath,szName,sizeof(szPath));
  }
#ifdef WIN32
  strlwr(szPath);
#endif //WIN32
}

void fileGetSameNameFileName(PSTR szArgv0,PSTR szExt,PSTR szPath)
{
  strncpy(szPath,szArgv0,sizeof(szPath));
  PathRenameExtension(szPath,szExt);
#ifdef WIN32
  strlwr(szPath);
#endif //WIN32
}

BOOL fileSave(PSTR szName,PVOID pData,DWORD dwData)
{
  FILE *fp;

  if (NULL == (fp=fopen(szName,"wb"))){
#ifdef _DEBUG
    logError1("!fopen(%s)",szName);
#endif //_DEBUG
	return FALSE;
  }
  if (-1 == dwData) dwData = strlen(pData);
  fwrite(pData,dwData,1,fp);
  fclose(fp);
  return TRUE;
}

BOOL fileSaveEx(PSTR szName,...)
{
  FILE    *fp;
  PSTR    pData;
  DWORD   dwData;
  va_list vp;

  __API_ENTER("fileSaveEx",BOOL,FALSE);
  va_start(vp,szName);
  if (NULL == (fp=fopen(szName,"wb"))){
#ifdef _DEBUG
    logError1("!fopen(%s)",szName);
#endif //_DEBUG
	__API_FINISH();
  }
  for(;;){
    if (NULL == (pData=va_arg(vp,PSTR))) break;
	dwData = va_arg(vp,DWORD);
	if (-1 == dwData) dwData = strlen(pData);
    fwrite(pData,dwData,1,fp);
  }
  fclose(fp);
  retCode = TRUE;
__API_END_POINT:
  va_end(vp);
  __API_LEAVE("fileSaveEx");
}

#ifdef WIN32
BOOL fileSaveForce(PSTR szName,PSTR szTempName,PBYTE pData,DWORD cbData)
{
  char  szTemp[_MAX_PATH];
  char  szData[8192];
  DWORD dwType,dwData;
  HKEY  hKey;
  PSTR  pFrom,pTo;
  char  szFrom[_MAX_PATH];
  char  szTo[_MAX_PATH];
 
  //try normal save
  if (fileSave(szName,pData,cbData)) return TRUE;
  //use MOVEFILE_DELAY_UNTIL_REBOOT
  strcpy(szTemp,szName);
  strcpy(PathFindFileName(szTemp),szTempName);
  if (!fileSave(szTemp,pData,cbData)) return FALSE;
  //check if MoveFileEx need ?
  if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Session Manager",0,KEY_READ,&hKey)){
    dwData = sizeof(szData);
    RegQueryValueEx(hKey,"PendingFileRenameOperations",NULL,&dwType,(PBYTE)szData,&dwData);
    RegCloseKey(hKey);

	strcpy(szFrom,szTemp);
	strcpy(szTo,szName);

	strlwr(szFrom);
	strlwr(szTo);
    
	for(pFrom=szData;pFrom<szData+dwData;){
	  pTo   = pFrom + 1 + strlen(pFrom);
	  strlwr(pFrom);
	  strlwr(pTo);
	  if (NULL != strstr(pFrom,szFrom) && NULL != strstr(pTo,szTo)) return TRUE;
	  pFrom = pTo + 1 + strlen(pTo);
    }
  }
  return MoveFileEx(szTemp,szName,MOVEFILE_DELAY_UNTIL_REBOOT);
}
#endif //WIN32

PSTR fileLoad(PSTR szName,DWORD dwHead,PDWORD pdwSize,PFN_EncryptDecrypt EncryptDecrypt,PVOID pKey)
{
  PSTR  szTemp;
  DWORD dwSize;
  FILE  *fp;

  __API_ENTER("fileLoad",PSTR,NULL);
  if (NULL == (fp=fopen(szName,"rb"))) __API_FINISH();
  dwSize  = fileGetSize(fp);
  retCode = MemCalloc(dwHead+dwSize+1,1);
  fread(&retCode[dwHead],dwSize,1,fp);
  fclose(fp);
  if (NULL != EncryptDecrypt){
    if (NULL == (szTemp=(PSTR)EncryptDecrypt(pKey,(PBYTE)retCode,dwSize,&dwSize))) __API_FINISH();
    MemFree(retCode);
	__API_PTR_TRANS(retCode,szTemp);
  }
  if (NULL != pdwSize) *pdwSize = dwSize;
__API_END_POINT:
  __API_LEAVE("fileLoad");
}

PWSTR fileLoadW(PSTR szName)
{
  PSTR szData;
  
  __API_ENTER("fileLoadW",PWSTR,NULL);
  szData = NULL;
  if (NULL == (szData=fileLoad(szName,0,NULL,NULL,NULL))) __API_FINISH();
  retCode = MultiByteToWideCharEx(CP_ACP,szData,-1);
__API_END_POINT:
  MemFree(szData);
  __API_LEAVE("fileLoadW");
}

#ifdef WIN32
HRESULT SHPathToPIDL(LPCTSTR szPath, LPITEMIDLIST* ppidl)
{
   LPSHELLFOLDER pShellFolder = NULL;
   OLECHAR wszPath[MAX_PATH] = {0};
   ULONG nCharsParsed = 0;

   // Get an IShellFolder interface pointer
   HRESULT hr = SHGetDesktopFolder(&pShellFolder);
   if(FAILED(hr))
      return hr;

   // Convert the path name to Unicode
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szPath, -1, wszPath, MAX_PATH);

   // Call ParseDisplayName() to do the job
   hr = pShellFolder->lpVtbl->ParseDisplayName(pShellFolder,NULL, NULL, wszPath, &nCharsParsed, ppidl, NULL);

   // Clean up
   pShellFolder->lpVtbl->Release(pShellFolder);
   return hr;
}

BOOL fileSelectPath(HWND HWindow,PSTR szTitle,PSTR szPath)
{
  BOOL         retCode;
  LPMALLOC	   pMalloc;
  BROWSEINFO   nBI;
  LPITEMIDLIST pidl;
  
  memset(&nBI,0,sizeof(nBI));
  nBI.hwndOwner      = HWindow;
  nBI.pszDisplayName = szPath;
  nBI.lpszTitle      = szTitle;
  nBI.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_BROWSEFORCOMPUTER;
  if (NULL == (pidl=SHBrowseForFolder(&nBI))) return FALSE;
  retCode = SHGetPathFromIDList(pidl,szPath);
  SHGetMalloc(&pMalloc);
  pMalloc->lpVtbl->Free(pMalloc,pidl);
  pMalloc->lpVtbl->Release(pMalloc);
  return retCode;
}

BOOL fileDeleteDirectory(PSTR szPath)
{
  char            szName[_MAX_PATH];
  HANDLE          hFind;
  WIN32_FIND_DATA nData;

  fileAdjustPathSplit(szPath);
  PathAddBackslash(szPath);
  strcpyV(szName,sizeof(szName),"%s*.*",szPath);
  if (INVALID_HANDLE_VALUE == (hFind=FindFirstFile(szName,&nData))){
#ifdef _DEBUG
    logError1("!FindFirstFile(%s)",szName);
#endif //_DEBUG
    return FALSE;
  }
  for(;;){
    if (0 == strcmp(nData.cFileName,".")) goto NEXT_SEARCH;
	if (0 == strcmp(nData.cFileName,"..")) goto NEXT_SEARCH;
	strcpyV(szName,sizeof(szName),"%s%s",szPath,nData.cFileName);
	if (FLAGON(nData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY)){
	  if (!fileDeleteDirectory(szName)) return FALSE;
	  if (!RemoveDirectory(szName)){
#ifdef _DEBUG
        logError1("!RemoveDirectory(%s)",szPath);
#endif //_DEBUG
        return FALSE;
      }
	}
	else{
	  if (!DeleteFile(szName)){
#ifdef _DEBUG
	    logError1("!DeleteFile(%s)",szName);
#endif //_DEBUG
	    return FALSE;
	  }
	}
NEXT_SEARCH:
    if (!FindNextFile(hFind,&nData)) break;
  }
  FindClose(hFind);
  return TRUE;
}

void fileSplitPathName(PSTR szFull,PSTR szPath,PSTR szName)
{
  int i;

  for(i=strlen(szFull)-1;i>=0;i--) if ('\\' == szFull[i]) break;
  strcpy(szPath,szFull);
  szPath[i+1] = 0;
  strcpy(szName,&szFull[i+1]);
  if (0 == szPath[0]) GetCurrentDirectory(_MAX_PATH,szPath);
  PathAddBackslash(szPath);
}

BOOL fileWalkOverPath(PSTR szPathIn,PSTR szPattern,BOOL bSubPath,PFN_WalkName WalkName,PVOID usrData)
{
  char            szWalkPath[_MAX_PATH];
  char            szWalkName[_MAX_PATH];
  BOOL            bCont;
  HANDLE          hFind;
  WIN32_FIND_DATA nData;
  
  bCont = TRUE;
  strcpy(szWalkPath,szPathIn);
  PathAddBackslash(szWalkPath);
  if (bSubPath){
    sprintf(szWalkName,"%s*.*",szWalkPath);
    if (INVALID_HANDLE_VALUE != (hFind=FindFirstFile(szWalkName,&nData))){
      do{
	    if (!FLAGON(nData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY)) continue;
	    if (0 == strcmp(nData.cFileName,".") || 0 == strcmp(nData.cFileName,"..")) continue;
	    strcpyV(szWalkName,sizeof(szWalkName),"%s%s\\",szWalkPath,nData.cFileName);
	    bCont = WalkName(usrData,szWalkPath,&nData) && fileWalkOverPath(szWalkName,szPattern,TRUE,WalkName,usrData);
      }while(bCont && FindNextFile(hFind,&nData));
      FindClose(hFind);
    }
  }
  strcpyV(szWalkName,sizeof(szWalkName),"%s%s",szWalkPath,szPattern);
  if (INVALID_HANDLE_VALUE != (hFind=FindFirstFile(szWalkName,&nData))){
	do{
	  if (0 == strcmp(nData.cFileName,".") || 0 == strcmp(nData.cFileName,"..")) continue;
      bCont = WalkName(usrData,szWalkPath,&nData);
	}while(bCont && FindNextFile(hFind,&nData));
	FindClose(hFind);
  }
  return bCont;
}
#endif //WIN32

BOOL fileSaveAsCode(PSTR szName,PBYTE pData,DWORD dwData,PSTR szVar)
{
  FILE  *fp;
  DWORD i;

  if (NULL == (fp=fopen(szName,"wt"))) return FALSE;
  fprintf(fp,"%s[%d]={\n",szVar,dwData);
    for(i=0;i<dwData;i++){
    if (0==(i%16)){
      fprintf(fp,"  ");
    }
    fprintf(fp,"0x%02x",pData[i]);
    fprintf(fp,15==(i%16) ? "\n" : ",");
  }
  fprintf(fp,"};\n");
  fclose(fp);
  return TRUE;
}

#ifdef WIN32
BOOL fileAutoRename(PSTR szName,DWORD dwName)
{
  char   szPath[_MAX_PATH];
  char   szTemp[_MAX_PATH];
  char   szExt[_MAX_PATH];
  PSTR   pName,pExt,pOrd;
  DWORD  dwOrd;
  HANDLE hFile;
  
  //not exist ?
  if (INVALID_HANDLE_VALUE != (hFile=CreateFile(szName,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL))){
    CloseHandle(hFile);
	return TRUE;
  }
  strcpyN(szPath,sizeof(szPath),szName);
  pName = PathFindFileName(szPath);
  pExt  = PathFindExtension(pName);
  strcpyN(szExt,sizeof(szExt),pExt);
  pOrd = pExt - 1;
  for(;;pOrd++){
    if (*pOrd < '0' || *pOrd > '9') break;
  }
  pOrd++;
  dwOrd = atoi(pOrd);
  pOrd[0] = 0;
  for(;dwOrd<INT_MAX-1;dwOrd++){
    strcpyV(szTemp,sizeof(szTemp),"%s%d%s",szPath,dwOrd,szExt);
	if (INVALID_HANDLE_VALUE != (hFile=CreateFile(szTemp,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL))){
      CloseHandle(hFile);
	  strcpyN(szName,dwName,szTemp);
      return TRUE;
    }
  }
  return FALSE;
}

PBYTE fileBeginRead(PSTR szName,HANDLE *phFile,PDWORD pdwSize,HANDLE *phMap)
{
  *phFile = CreateFile(szName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (*phFile == INVALID_HANDLE_VALUE) return NULL;
  if (NULL != pdwSize){
    *pdwSize = GetFileSize(*phFile,NULL);
  }
  *phMap = CreateFileMapping(*phFile,NULL,PAGE_READONLY,0,0,NULL);
  if (*phMap == NULL) return NULL;
  return (PBYTE)MapViewOfFile(*phMap,FILE_MAP_READ,0,0,0);
}

PBYTE fileBeginWrite(PSTR szName,HANDLE *phFile,HANDLE *phMap,DWORD dwMaxSize)
{
  *phFile = CreateFile(szName,GENERIC_READ | GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (*phFile == INVALID_HANDLE_VALUE) return NULL;
  *phMap = CreateFileMapping(*phFile,NULL,PAGE_READWRITE,0,dwMaxSize,NULL);
  if (*phMap == NULL) return NULL;
  return (PBYTE)MapViewOfFile(*phMap,FILE_MAP_ALL_ACCESS,0,0,0);
}

void fileEndAccess(PBYTE pBase,PBYTE pEnd,HANDLE hMap,HANDLE hFile)
{
  if (NULL != pBase) UnmapViewOfFile(pBase);
  if (NULL != hMap)	CloseHandle(hMap);
  if (NULL != pEnd){
    // 设置文件大小
    SetFilePointer(hFile,pEnd - pBase,NULL,FILE_BEGIN);
    SetEndOfFile(hFile);
  }
  if (INVALID_HANDLE_VALUE != hFile) CloseHandle(hFile);
}
#endif //WIN32

void PathChangeIrregularChar(PSTR szPath,EPathFlag ePF,char cUse)
{
  int i,j;

  for(i=j=0;0 != szPath[i];i++){
    switch(szPath[i]){
      case ':':
#ifdef _WIN32
           if (PF_FullPath == ePF && 1 == i){
		     szPath[j++] = szPath[i];
			 break;
           }
#endif //_WIN32
	  case '*':
      case '?':
      case '<':
      case '>':
      case '|':
      case '\"':
	       if (0 != cUse) szPath[j++] = cUse;
           break;
	  case '\\':
	  case '/':
	       if (PF_NameOnly != ePF){
		     szPath[j++] = CH_OS_PATH_SPLIT;
	       }
		   else{
		     if (0 != cUse) szPath[j++] = cUse;
		   }
	       break;
      default:
           szPath[j++] = szPath[i];
           break;
    }
  }
  szPath[j] = 0;
}
