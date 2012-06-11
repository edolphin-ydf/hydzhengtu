#include <zebra/srvEngine.h>

/**
 * \brief �õ�ϵͳʱ�������ַ���
 *
 * \param s ʱ����������ַ�����
 * \return ���ز���s
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
