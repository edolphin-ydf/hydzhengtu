#ifndef _INC_MEMLIB_H_
#define _INC_MEMLIB_H_

#include <baseLib/platForm.h>

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

PVOID MemMalloc_FL(const char *szFile,int nLine,size_t size);
PVOID MemCalloc_FL(const char *szFile,int nLine,size_t item,size_t size);
PVOID MemRealloc_FL(const char *szFile,int nLine,PVOID ptr,size_t size);

BOOL MemCheckSize_FL(const char *szFile,int nLine,PPVOID pptr,size_t size);

void MemFree_FL(const char *szFile,int nLine,PVOID ptr);

#ifdef _TRACE_MEMORY_

#define MemMalloc(size)         MemMalloc_FL(__FILE__,__LINE__,size)
#define MemCalloc(item,size)    MemCalloc_FL(__FILE__,__LINE__,item,size)
#define MemRealloc(ptr,size)    MemRealloc_FL(__FILE__,__LINE__,ptr,size)
#define MemCheckSize(pptr,size) MemCheckSize_FL(__FILE__,__LINE__,pptr,size)
#define MemFree(ptr)            MemFree_FL(__FILE__,__LINE__,ptr)

#else //_TRACE_MEMORY_

PVOID MemMalloc(size_t size);
PVOID MemCalloc(size_t item,size_t size);
PVOID MemRealloc(PVOID ptr,size_t size);
PVOID MemDup(PVOID ptr,size_t size);

BOOL MemCheckSize(PPVOID pptr,size_t size);

void MemFree(PVOID ptr);

#endif //_TRACE_MEMORY_

// Allocate a buffer (always 16 bytes aligned) 
PVOID MemMalloc16(size_t size);
PVOID MemCalloc16(size_t count,size_t size);

void MemFree16(PVOID ptr);

#ifdef WIN32
size_t MemSize(PVOID ptr);
#endif //WIN32

typedef PVOID (*PFNMalloc)(size_t size);
typedef PVOID (*PFNRealloc)(PVOID ptr,size_t size);
typedef void (*PFNFree)(PVOID ptr);
typedef void (*PFNFreeP)(PVOID usrData,PVOID ptr);

#define MemFreeF(ptr,fFree)				if (NULL != ptr) fFree(ptr);
#define MemFreeFP(ptr,fFree,usrData)	if (NULL == ptr) fFree(usrData,ptr);

#define MemFreeS(ptr)					if (NULL != ptr) { MemFree(ptr); ptr = NULL; }
#define MemFreeFS(ptr,fFree)			if (NULL != ptr) { fFree(ptr); ptr = NULL; }
#define MemFreeFPS(ptr,fFree,usrData)	if (NULL != ptr) { fFree(usrData,ptr); ptr = NULL; }

#define MemFree16S(ptr)					{ if (NULL != ptr) { MemFree16(ptr); ptr = NULL; } }

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_MEMLIB_H_

