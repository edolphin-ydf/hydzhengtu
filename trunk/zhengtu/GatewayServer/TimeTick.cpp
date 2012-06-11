/**
 * \brief ʱ��ص�����
 *
 * 
 */

#include "GatewayServer.h"

zRTime GatewayTimeTick::currentTime;
GatewayTimeTick *GatewayTimeTick::instance = NULL;

struct GatewayTaskCheckTime : public GatewayTaskManager::GatewayTaskCallback
{
  bool exec(GatewayTask *gt)
  {
    return gt->checkTime(GatewayTimeTick::currentTime);
  }
};

/**
 * \brief �߳�������
 *
 */
void GatewayTimeTick::run()
{
  int nSeconds;

  nSeconds = 0;
  while(!isFinal())
  {
    zThread::sleep(1);

    //��ȡ��ǰʱ��
    currentTime.now();

    if (one_second(currentTime) ) {
      LoginSessionManager::getInstance().update(currentTime);

      GatewayTaskCheckTime gtct;
      GatewayTaskManager::getInstance().execAll(gtct);

      if (nSeconds++ > 60)
      {
        //60 seconds
        nSeconds = 0;
        fprintf(stderr,"��ǰ��������:%d\n",GatewayService::getInstance().getPoolSize());
      }
    }
  }
}

