/**
 * \brief 时间回调函数
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
 * \brief 线程主函数
 *
 */
void GatewayTimeTick::run()
{
  int nSeconds;

  nSeconds = 0;
  while(!isFinal())
  {
    zThread::sleep(1);

    //获取当前时间
    currentTime.now();

    if (one_second(currentTime) ) {
      LoginSessionManager::getInstance().update(currentTime);

      GatewayTaskCheckTime gtct;
      GatewayTaskManager::getInstance().execAll(gtct);

      if (nSeconds++ > 60)
      {
        //60 seconds
        nSeconds = 0;
        fprintf(stderr,"当前在线人数:%d\n",GatewayService::getInstance().getPoolSize());
      }
    }
  }
}

