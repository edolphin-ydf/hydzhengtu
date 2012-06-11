#include <baseLib/memLib.h>
#include <baseLib/logLib.h>
#include <baseLib/codeLib.h>
#include <baseLib/strLib.h>
#include <stdarg.h>
#include <stdio.h>
//#include <string.h>
#include <stdlib.h>
#include <malloc.h>

//#define _LOG_MEMORY_

#ifdef _LOG_MEMORY_
static DWORD g_dwAlloced = 0;
#endif //_LOG_MEMORY_

PVOID MemMalloc(size_t size)
{
  PVOID pvNew;
  
  if (0 == size) size = 1;
  if (NULL != (pvNew=malloc(size))){
#ifdef _LOG_MEMORY_
    logMessage("MemMalloc(%d,%d) ==> 0x%08x",g_dwAlloced,size,pvNew);
	g_dwAlloced++;
#endif //_LOG_MEMORY_
    return pvNew;
  }
  logError1("!malloc(%d)",size);
#ifdef _DEBUG
  __asm int 3;
#endif //_DEBUG
  return NULL;
}

PVOID MemCalloc(size_t item,size_t size)
{
  PVOID pvNew;

  if (0 == item) item = 1;
  if (0 == size) size = 1;
  if (NULL != (pvNew=calloc(item,size))){
#ifdef _LOG_MEMORY_
    logMessage("MemCalloc(%d,%d,%d) ==> 0x%08x",g_dwAlloced,item,size,pvNew);
	g_dwAlloced++;
#endif //_LOG_MEMORY_
    return pvNew;
  }
  logError2("!calloc(%d,%d)",item,size);
#ifdef _DEBUG
  __asm int 3;
#endif //_DEBUG
  return NULL;
}

PVOID MemRealloc(PVOID pvMem,size_t size)
{
  PVOID pvNew;

  if (0 == size) size = 1;
  if (NULL == pvMem) return MemCalloc(size,1);
#ifdef WIN32
  if (_msize(pvMem) >= size) return pvMem;
#endif //WIN32
  if (NULL != (pvNew=realloc(pvMem,size))){
#ifdef _LOG_MEMORY_
    logMessage("MemRealloc(0x%08x,%d) ==> 0x%08x",pvMem,size,pvNew);
#endif //_LOG_MEMORY_
    return pvNew;
  }
#ifdef WIN32
  logError3("!realloc(0x%08x,%d,%d)",pvMem,_msize(pvMem),size);
#else //!WIN32
  logError2("!realloc(0x%08x,%d,%d)",pvMem,size);
#endif //WIN32
  MemFree(pvMem);
#ifdef _DEBUG
  __asm int 3;
#endif //_DEBUG
  return NULL;
}

PVOID MemDup(PVOID pOld,size_t size)
{
  PVOID pNew;

  if (NULL == pOld || 0 == size) return NULL;
  if (-1 == size) size = 1 + strlen((PSTR)pOld);
  if (NULL == (pNew=MemMalloc(size))) return NULL;
  memcpy(pNew,pOld,size);
  return pNew;
}

BOOL MemCheckSize(PPVOID ppvMem,size_t size)
{
  PVOID pNew;
  
  if (NULL == *ppvMem){
    *ppvMem = pNew = MemCalloc(size,1);
  }
  else{
   if (NULL != (pNew=MemRealloc(*ppvMem,size))){
     *ppvMem = pNew;
   }
  }
  return NULL != pNew;
}

#ifdef WIN32
size_t MemSize(PVOID pvMem)
{
  return _msize(pvMem);
}
#endif //WIN32

void MemFree(PVOID pvMem)
{
  if (NULL == pvMem) return;
#ifdef _LOG_MEMORY_
  g_dwAlloced--;
  logMessage("MemFree(%d,0x%08x)",g_dwAlloced,pvMem);
#endif //_LOG_MEMORY_
  free(pvMem);
}

PVOID MemMalloc_FL(const char *szFile,int nLine,size_t size)
{
  PVOID pvNew;
  
  if (0 == size) size = 1;
  if (NULL != (pvNew=malloc(size))){
#ifdef _LOG_MEMORY_
    logMessage_FL(szFile,nLine,"MemMalloc(%d,%d) ==> 0x%08x",g_dwAlloced,size,pvNew);
	g_dwAlloced++;
#endif //_LOG_MEMORY_
    return pvNew;
  }
  logError_FL(szFile,nLine,"!malloc(%d)",size);
#ifdef _DEBUG
  __asm int 3;
#endif //_DEBUG
  return NULL;
}

PVOID MemCalloc_FL(const char *szFile,int nLine,size_t item,size_t size)
{
  PVOID pvNew;

  if (0 == item) item = 1;
  if (0 == size) size = 1;
  if (NULL != (pvNew=calloc(item,size))){
#ifdef _LOG_MEMORY_
    logMessage_FL(szFile,nLine,"MemCalloc(%d,%d,%d) ==> 0x%08x",g_dwAlloced,item,size,pvNew);
	g_dwAlloced++;
#endif //_LOG_MEMORY_
    return pvNew;
  }
  logError_FL(szFile,nLine,"!calloc(%d,%d)",item,size);
#ifdef _DEBUG
  __asm int 3;
#endif //_DEBUG
  return NULL;
}

PVOID MemRealloc_FL(const char *szFile,int nLine,PVOID pvMem,size_t size)
{
  PVOID pvNew;

  if (0 == size) size = 1;
  if (NULL == pvMem) return MemCalloc_FL(szFile,nLine,size,1);
#ifdef WIN32
  if (_msize(pvMem) >= size) return pvMem;
#endif //WIN32
  if (NULL != (pvNew=realloc(pvMem,size))){
    logMessage_FL(szFile,nLine,"MemRealloc(0x%08x,%d) ==> 0x%08x",pvMem,size,pvNew);
    return pvNew;
  }
#ifdef WIN32
  logError_FL(szFile,nLine,"!realloc(0x%08x,%d,%d)",pvMem,_msize(pvMem),size);
#else //!WIN32
  logError_FL(szFile,nLine,"!realloc(0x%08x,%d,%d)",pvMem,size);
#endif //WIN32
  MemFree(pvMem);
#ifdef _DEBUG
  __asm int 3;
#endif //_DEBUG
  return NULL;
}

BOOL MemCheckSize_FL(const char *szFile,int nLine,PPVOID ppvMem,size_t size)
{
  if (NULL == *ppvMem){
    *ppvMem = MemCalloc_FL(szFile,nLine,size,1);
  }
  else{
   *ppvMem = MemRealloc_FL(szFile,nLine,*ppvMem,size);
  }
  return NULL != *ppvMem;
}

void MemFree_FL(const char *szFile,int nLine,PVOID pvMem)
{
  if (NULL == pvMem) return;
#ifdef _LOG_MEMORY_
  g_dwAlloced--;
  logMessage_FL(szFile,nLine,"MemFree(%d,0x%08x)",g_dwAlloced,pvMem);
#else
  logMessage_FL(szFile,nLine,"MemFree(%0x%08x)",pvMem);
#endif //_LOG_MEMORY_
  free(pvMem);
}

//16 byte aligned
PVOID MemMalloc16(size_t size)
{
  PVOID mptr;
  PBYTE nptr;

  if (NULL == (mptr=malloc(size+0x10))) return NULL;
  // Aligns memory block on 16byte
  nptr = (PBYTE)(((long)mptr + 0x10) & ~0x0F);
  nptr[-1] = nptr - (PBYTE)mptr;
  return (PVOID)nptr;
}

// Allocate a buffer (always 16 bytes aligned) 
PVOID MemCalloc16(size_t count,size_t size)
{
  PVOID mptr;

  if (NULL == (mptr=MemMalloc16(count*size))) return NULL;
  // Initialise memory to zero
  memset(mptr,0,count * size);
  return mptr;
}

// Free a buffer
void MemFree16(PVOID mptr)
{
  PBYTE nptr;
 
  if (NULL == mptr) return;
  nptr  = (PBYTE)mptr;
  nptr -= nptr[-1];
  free(nptr);
}

/*
size_t _msize_16(PVOID mptr)
{
  char *nptr,cext;
 
  nptr  = (char*)mptr;
  cext  = nptr[-1];
  nptr -= cext;
  return _msize(nptr) - cext;
}
*/