#ifndef _INC_LINKLIB_H_
#define _INC_LINKLIB_H_

#include <baseLib/platForm.h>
#include <baseLib/memLib.h>

#ifdef __cplusplus
extern "C"{
#endif //__cplusplus

#pragma pack(1)

typedef struct tagListNode *PListNode;
typedef struct tagListNode{
  PListNode pNext;
}NListNode;

typedef struct{
  PListNode pHead;
  PListNode pTail;
}NList,*PList;

#pragma pack()

PListNode llNodeAlloc(int sizeofData);

PListNode llListInsertData(PList pList,PListNode pBefore,PVOID pvData,int cbData,int sizeData);
PListNode llListInsertWSTR(PList pList,PListNode pBefore,PWSTR szData,int cbData,int sizeData);

PVOID llNodeData(PListNode pNode);

PListNode llNodeHandle(PVOID pData);

BOOL llListInsertNode(PList pList,PListNode pBefore,PListNode pNode);
BOOL llListRemoveNode(PList pList,PListNode pNode);
BOOL llListReplaceNode(PList pList,PListNode pOld,PListNode pNew);
BOOL llListFindNode(PList pList,PListNode pNode,PListNode *ppPrev);

void llListInsertNodeNC(PList pList,PListNode pPrev,PListNode pBefore,PListNode pNode);
void llListRemoveNodeNC(PList pList,PListNode pPrev,PListNode pNode);
void llListReplaceNodeNC(PList pList,PListNode pPrev,PListNode pOld,PListNode pNew);

typedef int (*PFNCompare)(PVOID pvL,PVOID pvR);
typedef int (*PFNCompareP)(PVOID usrData,PVOID pvL,PVOID pvR);

PListNode llListLocateNode(PList pList,int nPos,PListNode *ppPrev);
PListNode llListEnumNode(PList pList,PFNCompare fnELN,PVOID usrData,PListNode *ppPrev);
PListNode llListDetach(PList pList);

void llNodeFree(PListNode pNode,PFNFree fnFree);
void llNodeFreeP(PListNode pNode,PFNFreeP fnFree,PVOID usrData);

DWORD llListCount(PList pList);

void llListSort(PList pList,PFNCompare fnCompare);
void llListSortP(PList pList,PFNCompareP fnCompare,PVOID usrData);

void llListAttach(PList pList,PListNode pHead);

void llListFree(PList pList,PFNFree fnFree);
void llListFreeP(PList pList,PFNFreeP fnFree,PVOID usrData);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_INC_LINKLIB_H_
