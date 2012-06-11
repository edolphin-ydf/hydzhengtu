/**
 * \brief 实现zProperties属性关联类
 *
 * 
 */
#include <zebra/srvEngine.h>

#include <iostream>
#include <fstream>
//#include <ext/hash_map>

DWORD zProperties::parseCmdLine(const std::string &cmdLine)
{
  std::vector<std::string> sv;
  stringtok(sv,cmdLine);
  for(std::vector<std::string>::const_iterator it = sv.begin(); it != sv.end(); it++)
  {
    std::vector<std::string> ssv;
    stringtok(ssv,*it,"=",1);
    if (ssv.size() == 2)
    {
      properties[ssv[0]] = ssv[1];
    }
  }
  return properties.size();
}

DWORD zProperties::parseCmdLine(const char *cmdLine)
{
  std::vector<std::string> sv;
  stringtok(sv,cmdLine);
  for(std::vector<std::string>::const_iterator it = sv.begin(); it != sv.end(); it++)
  {
    std::vector<std::string> ssv;
    stringtok(ssv,*it,"=",1);
    if (ssv.size() == 2)
    {
      properties[ssv[0]] = ssv[1];
    }
  }
  return properties.size();
}

