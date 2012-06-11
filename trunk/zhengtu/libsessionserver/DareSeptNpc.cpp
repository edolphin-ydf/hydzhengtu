/**
 * \brief ����NPC����ս
 *
 * 
 */
#include <zebra/SessionServer.h>

CDareSeptNpc::CDareSeptNpc(DWORD active_time,DWORD ready_time):CDareSept(active_time,ready_time)
{
}

CDareSeptNpc::~CDareSeptNpc()
{
}

void CDareSeptNpc::setReadyOverState()
{
  Cmd::Session::t_enterWar_SceneSession exit_war; // ֪ͨ�������˳���ս״̬

  exit_war.dwWarType = this->type;
  exit_war.dwStatus = 0;

  for (DWORD i=0; i<attList.size(); i++)
  {
    this->sendCmdToAllDarePlayer(&exit_war,sizeof(exit_war),attList[i]);
  }
  
  this->sendCmdToAllDarePlayer(&exit_war,sizeof(exit_war),this->secondID);

  this->setOverState();

}

