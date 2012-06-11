/**
 * \brief �����ļ�����������,
 */
#pragma once 
#include <zebra/srvEngine.h>


///�ַ���ת��д
string& str_ToUpper(string& s_Str)
{
	long i=0;
	for (i=0; i<(long)s_Str.length(); i++)
		if (isalpha(s_Str[i])) 
			s_Str[i] = (char)toupper(s_Str[i]);

	return s_Str;
}

///�ַ���תСд
string& str_ToLower(string& a_Str)
{
	long i=0;
	for (i=0; i< (long)a_Str.length(); i++)
		if (isalpha(a_Str[i])) 
			a_Str[i] = (char)tolower(a_Str[i]);

	return a_Str;
}
/**
 * \brief ���캯��
 * \param confile �����ļ�����
 */
zConfile::zConfile(const char *confile)
{
  this->confile = confile;
}

/**
 * \brief ��������
 */
zConfile::~zConfile()
{
  parser.final();
}

/**
 * \brief ȫ�ֽ�������
 * \param node ȫ�����ýڵ�
 * \return �����Ƿ�ɹ�
 */
bool zConfile::globalParse(const xmlNodePtr node)
{
  xmlNodePtr child = parser.getChildNode(node,NULL);
  string bstr((char*)child->name);
  bstr=str_ToLower(bstr);
  while(child)
  {
	bstr = std::string((char*)child->name);
	bstr=str_ToLower(bstr);
	if (strcmp(bstr.c_str(),"superserver") == 0)
	{
      parseSuperServer(child);
	}
    else
      parseNormal(child);
    child=parser.getNextNode(child,NULL);
  }
  
  return true;
}

/**
 * \brief ��ͨ��������,ֻ�Ǽ򵥵İѲ�������global������
 * \param node Ҫ�����Ľڵ�
 * \return �����Ƿ�ɹ�
 */
bool zConfile::parseNormal(const xmlNodePtr node)
{
  char buf[128];
  if (parser.getNodeContentStr(node,buf,128))
  {
    Zebra::global[(char *)node->name]=buf;
    string bstr(buf);
    bstr=str_ToLower(bstr);
    if (0 == strcmp((char *)node->name,"mysql")
        && parser.getNodePropStr(node,"encode",buf,128)
        && bstr=="yes")
    {
      std::string tmpS;
      base64_decrypt(Zebra::global[(char *)node->name],tmpS);
      Zebra::global[(char *)node->name]=tmpS;
    }
    return true;
  }
  else
    return false;
}

/**
 * \brief SuperServer��������������global�����з�����������
 *
 * server SuperServer��ַ
 *
 * port SuperServer�˿�
 * \param node SuperServer�����ڵ�
 * \return �����Ƿ�ɹ�
 */
bool zConfile::parseSuperServer(const xmlNodePtr node)
{
  char buf[64];
  if (parser.getNodeContentStr(node,buf,64))
  {
    Zebra::global["server"]=buf;
    if (parser.getNodePropStr(node,"port",buf,64))
      Zebra::global["port"]=buf;
    else
      Zebra::global["port"]="10000";
    return true;
  }
  else
    return false;
}

/**
 * \brief ��ʼ���������ļ�
 *
 * \param name ʹ�����Լ������Ķ���ڵ�����
 * \return �����Ƿ�ɹ�
 */
bool zConfile::parse(const char *name)
{
  if (parser.initFile(confile))
  {
    xmlNodePtr root=parser.getRootNode("Zebra");
    if (root)
    {
      xmlNodePtr globalNode=parser.getChildNode(root,"global");
      if (globalNode)
      {
        if (!globalParse(globalNode))
          return false;
      }
      else
        Zebra::logger->warn("��ȫ�����ö���.");
      xmlNodePtr otherNode=parser.getChildNode(root,name);
      if (otherNode)
      {
        if (!parseYour(otherNode))
          return false;
      }
      else
        Zebra::logger->warn("�� %s ���ö���.",name);
      return true;
    }
  }
  return false;
}
