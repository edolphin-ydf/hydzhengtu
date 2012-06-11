#include <zebra/srvEngine.h>

/**
 * \brief 得到系统时区设置字符串
 *
 * \param s 时区将放入此字符串中
 * \return 返回参数s
 */
std::string & zRTime::getLocalTZ(std::string & s)
{
  long tz;

  std::ostringstream so;
  tzset();
  tz = _timezone/3600;
  //so << _tzname[0];
  so << tz;
  s= so.str();
  return s;
}
