/**
 * \brief 时间回调函数
 *
 * 
 */

#include "BillServer.h"


BillTimeTick *BillTimeTick::instance = NULL;
zRTime BillTimeTick::currentTime;
Timer BillTimeTick::_one_min(60); 
Timer BillTimeTick::_one_sec(1); 
void BillTimeTick::run()
{
  while(!isFinal())
  {
    zThread::msleep(50);
    currentTime.now();
    if (_one_sec(currentTime))
    {
      ::Bill_timeAction();

      BillUserManager::getInstance()->update();
      if (_one_min(currentTime))
      {
        ConsignHistoryManager::getInstance()->update();
      }
    }
    BillTaskManager::getInstance().execEvery();
    BillClientManager::getInstance().execEvery();
  }
}

