#include <baseLib/logLib.h>
#include <baseLib/linkLib.h>

PListNode llNodeAlloc(int cbData)
{
  return (PListNode)MemCalloc(sizeof(NListNode) + cbData,1); 
}

PListNode llListInsertData(PList pList,PListNode pBefore,PVOID pData,int cbData,int exData)
{
  PListNode pNode;

  if (-1 == cbData) cbData = 1 + strlen(pData);
  if (NULL == (pNode=llNodeAlloc(cbData+exData))) return NULL;
  if (NULL != pData){
    memcpy(&pNode[1],pData,cbData);
  }
  llListInsertNode(pList,pBefore,pNode);
  return pNode;
}

PListNode llListInsertWSTR(PList pList,PListNode pBefore,PWSTR szData,int size,int exData)
{
  if (-1 == size) size = 1 + wcslen(szData);
  size *= sizeof(WCHAR);
  return llListInsertData(pList,pBefore,szData,size,exData);
}

PVOID llNodeData(PListNode pNode)
{
  return NULL == pNode ? NULL : &pNode[1];
}

PListNode llNodeHandle(PVOID pData)
{
  return NULL == pData ? NULL : &((PListNode)pData)[-1];
}

BOOL llListInsertNode(PList pList,PListNode pBefore,PListNode pNode)
{
  PListNode pPrev;

  if (llListFindNode(pList,pNode,NULL)) return FALSE;
  if (NULL == pBefore){
    //append
	pNode->pNext = NULL;
	if (NULL != pList->pTail){
	  pList->pTail->pNext = pNode;
	}
	pList->pTail = pNode;
	if (NULL == pList->pHead){
	  pList->pHead = pNode;
    }
  }
  else{
    if (!llListFindNode(pList,pBefore,&pPrev)) return FALSE;
	llListInsertNodeNC(pList,pPrev,pBefore,pNode);
  }
  return TRUE;
}

void llListInsertNodeNC(PList pList,PListNode pPrev,PListNode pBefore,PListNode pNode)
{
  pNode->pNext = pBefore;
  if (NULL == pPrev){
    pList->pHead = pNode;
  }
  else{
    pPrev->pNext = pNode;
  }
}

BOOL llListFindNode(PList pList,PListNode pLookup,PListNode *ppPrev)
{
  PListNode pPrev,pNode;

  for(pPrev=NULL,pNode=pList->pHead;NULL!=pNode;pPrev=pNode,pNode=pNode->pNext){
    if (pNode == pLookup){
	  if (NULL != ppPrev) *ppPrev = pPrev;
	  return TRUE;
	}
  }
  return FALSE;
}

BOOL llListRemoveNode(PList pList,PListNode pNode)
{
  PListNode pPrev;

  if (!llListFindNode(pList,pNode,&pPrev)) return FALSE;
  llListRemoveNodeNC(pList,pPrev,pNode);
  return TRUE;
}

void llListRemoveNodeNC(PList pList,PListNode pPrev,PListNode pNode)
{
  //是首?
  if (NULL == pPrev){
    pList->pHead = pNode->pNext;
  }
  else{
    pPrev->pNext = pNode->pNext;
  }
  //是尾?
  if (pNode == pList->pTail){
    pList->pTail = pPrev;
  }
}

BOOL llListReplaceNode(PList pList,PListNode pOld,PListNode pNew)
{
  PListNode pPrev;
 
  if (llListFindNode(pList,pNew,NULL)) return FALSE;
  if (!llListFindNode(pList,pOld,&pPrev)) return FALSE;
  llListReplaceNodeNC(pList,pPrev,pOld,pNew);
  return TRUE;
}

void llListReplaceNodeNC(PList pList,PListNode pPrev,PListNode pOld,PListNode pNew)
{
  pNew->pNext = pOld->pNext;
  //在首?
  if (NULL == pPrev){
    pList->pHead = pNew;
  }
  else{
    pPrev->pNext = pNew;
  }
  //在尾?
  if (pOld == pList->pTail){
    pList->pTail = pNew;
  }
}

void llNodeFree(PListNode pNode,PFNFree fnFree)
{
  if (NULL == pNode) return;
  if (NULL != fnFree) fnFree(&pNode[1]);
  MemFree(pNode);
}

void llNodeFreeP(PListNode pNode,PFNFreeP fnFree,PVOID pFree)
{
  if (NULL == pNode) return;
  fnFree(pFree,&pNode[1]);
  MemFree(pNode);
}

PListNode llListLocateNode(PList pList,int nPos,PListNode *ppPrev)
{
  PListNode pPrev,pNode;

  if (nPos < 0) return NULL;
  for(pPrev=NULL,pNode=pList->pHead;NULL!=pNode && 0 != nPos;pPrev=pNode,pNode=pNode->pNext,nPos--);
  if (NULL != ppPrev) *ppPrev = pPrev;
  return pNode;
}

PListNode llListEnumNode(PList pList,PFNCompare fnCompare,PVOID pvParam,PListNode *ppPrev)
{
  PListNode pPrev,pNode;

  for(pPrev=NULL,pNode=pList->pHead;NULL!=pNode;pPrev=pNode,pNode=pNode->pNext){
    if (0 == fnCompare(pvParam,&pNode[1])){
	  if (NULL != ppPrev) *ppPrev = pPrev;
	  return pNode;
	}
  }
  return NULL;
}

PListNode llListDetach(PList pList)
{
  PListNode pHead;

  pHead = pList->pHead;
  pList->pHead = NULL;
  pList->pTail = NULL;
  return pHead;
}

DWORD llListCount(PList pList)
{
  DWORD     dwCount;
  PListNode pNode;

  if (NULL == pList) return 0;
  for(dwCount=0,pNode=pList->pHead;NULL!=pNode;pNode=pNode->pNext,dwCount++);
  return dwCount;
}

void llListSort(PList pList,PFNCompare fnCompare)
{
  NList     nNew;
  PListNode pOld,pNext,pNew;

  memset(&nNew,0,sizeof(nNew));
  for(pOld=pList->pHead;NULL!=pOld;pOld=pNext){
    pNext = pOld->pNext;
    for(pNew=nNew.pHead;NULL!=pNew;pNew=pNew->pNext){
	  if (fnCompare(&pOld[1],&pNew[1]) < 0) break;
    }
	llListInsertNode(&nNew,pNew,pOld);
  }
  *pList = nNew;
}

void llListSortP(PList pList,PFNCompareP fnCompare,PVOID usrData)
{
  NList     nNew;
  PListNode pOld,pNext,pNew;

  memset(&nNew,0,sizeof(nNew));
  for(pOld=pList->pHead;NULL!=pOld;pOld=pNext){
    pNext = pOld->pNext;
    for(pNew=nNew.pHead;NULL!=pNew;pNew=pNew->pNext){
	  if (fnCompare(usrData,&pOld[1],&pNew[1]) < 0) break;
    }
	llListInsertNode(&nNew,pNew,pOld);
  }
  *pList = nNew;
}

void llListAttach(PList pList,PListNode pHead)
{
  PListNode pTail;

  for(pTail=pHead;NULL!=pTail;pTail=pTail->pNext){
    if (NULL == pTail->pNext) break;
  }
  pList->pHead = pHead;
  pList->pTail = pTail;
}

void llListFree(PList pList,PFNFree fnFree)
{
  PListNode pNode,pNext;

  for(pNode=pList->pHead;NULL!=pNode;pNode=pNext){
    pNext = pNode->pNext;
	if (NULL != fnFree) fnFree(&pNode[1]);
    MemFree(pNode);
  }
  memset(pList,0,sizeof(NList));
}

void llListFreeP(PList pList,PFNFreeP fnFree,PVOID pFree)
{
  PListNode pNode,pNext;

  for(pNode=pList->pHead;NULL!=pNode;pNode=pNext){
    pNext = pNode->pNext;
	if (NULL != fnFree) fnFree(pFree,&pNode[1]);
    MemFree(pNode);
  }
  memset(pList,0,sizeof(NList));
}
