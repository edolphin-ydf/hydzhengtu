/**
 * \brief ���ҿƼ�������
 *
 * 
 */
#include <zebra/ScenesServer.h>

void CountryAllyM::init()
{
}

CAlly* CountryAllyM::getAlly(DWORD dwCountryID1,DWORD dwCountryID2)
{
  CAlly* pAlly = NULL;
  std::vector<CAlly*>::iterator pos;

  rwlock.rdlock();

  for (pos = allies.begin(); pos!=allies.end(); pos++)
  {
    CAlly* temp = *pos;
    if (temp 
      && ((temp->dwCountryID == dwCountryID1 && temp->dwAllyCountryID == dwCountryID2)
      || (temp->dwCountryID == dwCountryID2 && temp->dwAllyCountryID == dwCountryID1))
      )
    {
      pAlly = temp;
      break;
    }
  }

  rwlock.unlock();
    
  return pAlly;
}

void   CountryAllyM::processUpdate(Cmd::Session::t_updateAlly_SceneSession *rev)
{
  rwlock.rdlock();
  
#ifdef   _DEBUG
  Zebra::logger->debug("[��������]: �յ� %d ����������",rev->dwSize);
#endif  
  for (DWORD i=0; i<rev->dwSize; i++)
  {
    CAlly* pAlly = this->getAlly(rev->data[i].dwCountryID,rev->data[i].dwAllyCountryID);
    if (pAlly)
    {//update
#ifdef   _DEBUG
  Zebra::logger->debug("[��������]: ���� %d,%d �����Ѻö�:%d",pAlly->dwCountryID,
      pAlly->dwAllyCountryID,pAlly->dwFriendDegree);
#endif  

      pAlly->dwFriendDegree = rev->data[i].dwFriendDegree;
    }
    else
    {//new
      pAlly = new CAlly();
      if (pAlly)
      {
        pAlly->dwCountryID = rev->data[i].dwCountryID;
        pAlly->dwAllyCountryID = rev->data[i].dwAllyCountryID;
        pAlly->dwFriendDegree = rev->data[i].dwFriendDegree;
        allies.push_back(pAlly);
#ifdef   _DEBUG
  Zebra::logger->debug("[��������]: �½� %d,%d �����Ѻö�:%d",pAlly->dwCountryID,
      pAlly->dwAllyCountryID,pAlly->dwFriendDegree);
#endif  
      }
    }
  }

  rwlock.unlock();
}

bool  CountryAllyM::isAlly(DWORD dwCountryID1,DWORD dwCountryID2)
{
  CAlly* pAlly = this->getAlly(dwCountryID1,dwCountryID2);

  if (pAlly && pAlly->dwFriendDegree>0)
  {
    return true;
  }

  return false;
}

DWORD  CountryAllyM::getFriendLevel(DWORD dwCountryID1,DWORD dwCountryID2)
{
  CAlly* pAlly = this->getAlly(dwCountryID1,dwCountryID2);

  if (pAlly && pAlly->dwFriendDegree>0)
  {
    return 2; // ֻҪ�Ѻöȴ���0����Э����ϵ,�˹���ϵ�ĵڶ���
/*    if (pAlly->dwFriendDegree>10000)
    {
      return 2;
    }
    else
    {
      return 1;
    }
    */
  }

  return 0;
}

