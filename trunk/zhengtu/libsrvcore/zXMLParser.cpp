/**
 * \brief zXMLParser��ʵ���ļ�
 */
#include <zebra/srvEngine.h>
#include <iconv.h>

#include <iostream>

//static void finalLibXML2() __attribute__ ((destructor));
static void finalLibXML2();
void finalLibXML2()
{
  xmlCleanupParser();
}

/**
 * \brief ����zXMLParser
 */
zXMLParser::zXMLParser()
{
  doc = NULL;
}

/**
 * \brief zXMLParser����������
 *
 * ���շ���Ŀռ�,�رմ��ļ���
 */
zXMLParser::~zXMLParser()
{
  final();
}

/**
 * \brief ��ʼ��Ҫ������xml�ļ�
 *
 * 
 * \param xmlFile �ļ�·��,���Ի�����Եġ�
 * \return �ɹ�����true,ʧ�ܷ���false�� 
 */
bool zXMLParser::initFile(const std::string &xmlFile)
{
  return initFile(xmlFile.c_str());
}

/**
 * \brief ��ʼ��Ҫ������xml�ļ�
 * \param xmlFile �ļ�·��,���Ի�����Եġ�
 * \return �ɹ�����true,ʧ�ܷ���false�� 
 */
bool zXMLParser::initFile(const char *xmlFile)
{
    Zebra::logger->info("zXMLParser::initFile(%s)",xmlFile);
  final();
  if (xmlFile==NULL) return false;
  doc = xmlParseFile(xmlFile);
  return (doc!=NULL);
}

/**
 * \brief ��ʼ��Ҫ������XML�ַ���
 *
 *
 * \param xmlStr ����ʼ����xml�ַ�����
 * \return �ɹ�����true,ʧ�ܷ���false��
 */
bool zXMLParser::initStr(const std::string &xmlStr)
{
  return initStr(xmlStr.c_str());
}

/**
 * \brief ��ʼ��Ҫ������XML�ַ���
 *
 *
 * \param xmlStr ����ʼ����xml�ַ�����
 * \return �ɹ�����true,ʧ�ܷ���false��
 */
bool zXMLParser::initStr(const char *xmlStr)
{
  final();
  if (xmlStr == NULL) return false;
  doc = xmlParseDoc((xmlChar *)xmlStr);
  return (doc!=NULL);
}

/**
 * \brief ��ʼ��XML
 * \return �ɹ�����true,ʧ�ܷ���false��
 */
bool zXMLParser::init()
{
  final();
  doc = xmlNewDoc((const xmlChar *)"1.0");
  return (doc!=NULL);
}

/**
 * \brief �ͷű�������xml�ĵ� 
 *
 *
 */
void zXMLParser::final()
{
  if (doc)
  {
    xmlFreeDoc(doc);
    doc=NULL;
  }
}

/**
 * \brief dump��XML�ĵ�
 * \param s �ĵ����λ��
 * \param format ���ʱ���Ƿ��ʽ���ĵ�
 * \return ����s
 */
std::string & zXMLParser::dump(std::string & s,bool format)
{
  if (doc)
  {
    xmlChar *out=NULL;
    int size=0;
    //xmlDocDumpMemory(doc,&out,&size);
    xmlDocDumpFormatMemory(doc,&out,&size,format?1:0);
    if (out!=NULL )
    {
      s=(char *)out;
      xmlFree(out);
    }
  }
  return s;
}

/**
 * \brief dump��ĳ���ڵ�
 * \param dumpNode Ҫ��Dump�Ľڵ�
 * \param s �ĵ����λ��
 * \param head �Ƿ����xml�ļ�ͷ.Ĭ��true
 * \return ����s
 */
std::string & zXMLParser::dump(xmlNodePtr dumpNode,std::string & s,bool head)
{
  if (dumpNode==NULL) return s;
  xmlBufferPtr out=xmlBufferCreate();
  if (xmlNodeDump(out,doc,dumpNode,1,1)!=-1)
  {    
    BYTE *cout = charConv((BYTE*)out->content,"UTF-8",(const char *)doc->encoding);
    if (cout)
    {
      if (head)
      {
        s="<?xml version=\"1.0\" encoding=\"";
        s+=(char *)doc->encoding;
        s+="\"?>";
      }
      else
        s="";
      s+=(char *)cout;      
      SAFE_DELETE_VEC(cout);
    }
  }
  xmlBufferFree(out);
  return s;
}

/**
 * \brief �õ�xml�ĵ��ĸ��ڵ�
 * \param rootName ���ڵ�����֡�
 * \return ���ظ��ڵ�ָ��,����NULLʧ�ܡ�
 */
xmlNodePtr zXMLParser::getRootNode(const char *rootName)
{
  if (doc == NULL) return NULL;

  //�õ����ڵ�
  xmlNodePtr cur = xmlDocGetRootElement(doc);
  //׼����ʼ�ڵ�
  if (rootName!=NULL)
    while (cur != NULL && xmlStrcmp(cur->name,(const xmlChar *) rootName))
      cur = cur->next;
  return cur;
}

/**
 * \brief �õ�ĳ���ڵ���ӽڵ�
 *
 *
 * \param parent ���ڵ�
 * \param childName �ӽڵ������,���ΪNULL,����õ���һ���ӽڵ㡣
 * \return �ӽڵ�ָ��,����NULLʧ�ܻ���û����Ӧ�Ľڵ㡣
 */
xmlNodePtr zXMLParser::getChildNode(const xmlNodePtr parent,const char *childName)
{
  if (parent==NULL) return NULL;
  xmlNodePtr retval=parent->children;
  if (childName)
    while(retval)
    {
      if (!xmlStrcmp(retval->name,(const xmlChar *) childName)) break;
      retval=retval->next;
    }
  else
    while(retval)
    {
      if (!xmlNodeIsText(retval)) break;
      retval=retval->next;
    }

  return retval;
}

/**
 * \brief �õ���һ���ڵ�
 *
 *
 * \param node ��ǰ�ڵ�
 * \param nextName ��һ���ڵ������,���ΪNULL,����õ����ڵ���һ���ڵ㡣
 * \return ��һ���ڵ�ָ��,����NULLʧ�ܻ���û����Ӧ�Ľڵ㡣
 */
xmlNodePtr zXMLParser::getNextNode(const xmlNodePtr node,const char *nextName)
{
  if (node==NULL) return NULL;
  xmlNodePtr retval=node->next;
  if (nextName)
    while(retval)
    {
      if (!xmlStrcmp(retval->name,(const xmlChar *)nextName)) break;
      retval=retval->next;
    }
  else
    while(retval)
    {
      if (!xmlNodeIsText(retval)) break;
      retval=retval->next;
    }
  return retval;
}

/**
 * \brief ͳ���ӽڵ������
 *
 *
 * \param parent ��ͳ�Ƶĸ��ڵ�
 * \param childName ��ͳ���ӽڵ������,���ΪNULL,ͳ�������ӽڵ������
 * \return �ӽڵ������
 */
DWORD zXMLParser::getChildNodeNum(const xmlNodePtr parent,const char *childName)
{
  int retval=0;
  if (parent==NULL) return retval;
  xmlNodePtr child=parent->children;
  if (childName)
    while(child)
    {
      if (!xmlStrcmp(child->name,(const xmlChar *) childName)) retval++;
      child=child->next;
    }
  else
    while(child)
    {
      if (!xmlNodeIsText(child)) retval++;
      child=child->next;
    }
  return retval;
}

/**
 * \brief Ϊxml�ĵ����һ�����ڵ�
 * \param rootName ���ڵ�����
 * \return ��ӽڵ��Ժ�,���ؽڵ�ָ��
 */
xmlNodePtr zXMLParser::newRootNode(const char *rootName)
{
  if (NULL == doc)
    return NULL;

  xmlNodePtr root_node = xmlNewNode(NULL,(const xmlChar *)rootName);
  xmlDocSetRootElement(doc,root_node);
  return root_node;
}

/**
 * \brief ��һ���ڵ���������ӽڵ�
 * \param parent ���ڵ�
 * \param childName �ӽڵ�����
 * \param content �ӽڵ�����
 * \return ������ӽڵ��ָ��
 */
xmlNodePtr zXMLParser::newChildNode(const xmlNodePtr parent,const char *childName,const char *content)
{
  if (NULL == parent)
    return NULL;

  return xmlNewChild(parent,NULL,(const xmlChar *)childName,(const xmlChar *)content);
}

bool zXMLParser::newNodeProp(const xmlNodePtr node,const char *propName,const char *prop)
{
  if (NULL == node)
    return false;

  return (NULL != xmlNewProp(node,(const xmlChar *)propName,(const xmlChar *)prop));
}

/**
 * \brief �õ��ڵ�����,��ת��������
 *
 *
 * \param node ����ڵ�
 * \param propName Ҫ�õ�����������
 * \param prop ���ؽ���Ĵ洢λ��ָ��
 * \param propSize prop�Ŀռ��С
 * \return  �ɹ�����true,���򷵻�false
 */
bool zXMLParser::getNodePropNum(const xmlNodePtr node,const char *propName,void *prop,const int propSize)
{
  char *temp=NULL;
  bool ret=true;
  if (node==NULL || prop==NULL || propName==NULL) return false;
  temp = (char *)xmlGetProp(node,(const xmlChar *) propName);
  if (temp ==NULL ) return false;
  switch(propSize)
  {
    case sizeof(BYTE):
      *(BYTE*)prop=(BYTE)atoi(temp);
      break;
    case sizeof(WORD):
      *(WORD *)prop=(WORD)atoi(temp);
      break;
    case sizeof(DWORD):
      *(DWORD *)prop=atoi(temp);
      break;
    case sizeof(QWORD):
      *(QWORD *)prop=atol(temp);
      break;
    default:
      ret=false;
  }
  if (temp) xmlFree(temp);
  return ret;
}

/**
 * \brief �õ��ڵ�����
 *
 *
 * \param node ����ڵ�
 * \param propName Ҫ�õ�����������
 * \param prop ���ؽ���Ĵ洢λ��ָ��
 * \param propSize prop�Ŀռ��С
 * \return  �ɹ�����true,���򷵻�false
 */
bool zXMLParser::getNodePropStr(const xmlNodePtr node,const char *propName,void *prop,int propSize)
{
  char *temp=NULL;
  bool ret=true;
  if (node==NULL || prop==NULL || propName==NULL) return false;
  temp =(char *)xmlGetProp(node,(const xmlChar *) propName);
  if (temp ==NULL ) return false;
  BYTE *out =charConv((BYTE*)temp,"UTF-8",(const char *)doc->encoding);
  if (out)
  {
    bzero(prop,propSize);
    strncpy((char *)prop,(const char*)out,propSize - 1);
    SAFE_DELETE_VEC(out);
  }
  if (temp) xmlFree(temp);
  return ret;
}

/**
 * \brief �õ��ڵ�����
 *
 *
 * \param node ����ڵ�
 * \param propName Ҫ�õ�����������
 * \param prop ���ؽ���Ĵ洢λ��
 * \return  �ɹ�����true,���򷵻�false
 */
bool zXMLParser::getNodePropStr(const xmlNodePtr node,const char *propName,std::string &prop)
{
  char *temp=NULL;
  bool ret=true;
  if (node==NULL || propName==NULL) return false;
  temp =(char *)xmlGetProp(node,(const xmlChar *) propName);
  if (temp ==NULL ) return false;
  BYTE *out =charConv((BYTE*)temp,"UTF-8",(const char *)doc->encoding);
  if (out)
  {
    prop = (char *)out;
    SAFE_DELETE_VEC(out);
  }
  if (temp) xmlFree(temp);
  return ret;
}

/**
 * \brief �õ��ڵ�����,��ת��������
 *
 *
 * \param node ����ڵ�
 * \param content ���ؽ���Ĵ洢λ��ָ��
 * \param contentSize content�Ŀռ��С
 * \return  �ɹ�����true,���򷵻�false
 */
bool zXMLParser::getNodeContentNum(const xmlNodePtr node,void *content,int contentSize)
{
  char *temp=NULL;
  bool ret=true;
  if (node==NULL || content==NULL) return false;
  xmlNodePtr text=node->children;
  while(text!=NULL)
  {
    if (!xmlStrcmp(text->name,(const xmlChar *) "text"))
    {
      temp =(char *)text->content;
      break;
    }
    text=text->next;
  }
  if (temp ==NULL ) return false;
  switch(contentSize)
  {
    case sizeof(BYTE):
      *(BYTE*)content=(BYTE)atoi(temp);
      break;
    case sizeof(WORD):
      *(WORD *)content=(WORD)atoi(temp);
      break;
    case sizeof(DWORD):
      *(DWORD *)content=atoi(temp);
      break;
    case sizeof(QWORD):
      *(QWORD *)content=atol(temp);
      break;
    default:
      ret=false;
  }
  return ret;
}

/**
 * \brief �õ��ڵ�����
 *
 *
 * \param node ����ڵ�
 * \param content ���ؽ���Ĵ洢λ��ָ��
 * \param contentSize content�Ŀռ��С
 * \return  �ɹ�����true,���򷵻�false
 */
bool zXMLParser::getNodeContentStr(const xmlNodePtr node,void *content,const int contentSize)
{
  char *temp=NULL;
  bool ret=true;
  if (node==NULL || content==NULL) return false;
  xmlNodePtr text=node->children;
  while(text!=NULL)
  {
    if (!xmlStrcmp(text->name,(const xmlChar *) "text"))
    {
      temp =(char *)text->content;
      break;
    }
    text=text->next;
  }
  if (temp ==NULL ) return false;
  BYTE *out = charConv((BYTE*)temp,"UTF-8",(const char *)doc->encoding);
  if (out)
  {
    bzero(content,contentSize);
    strncpy((char *)content,(const char *)out,contentSize - 1);
    SAFE_DELETE_VEC(out);
  }
  return ret;
}

/**
 * \brief �õ��ڵ�����
 *
 *
 * \param node ����ڵ�
 * \param content ���ؽ���Ĵ洢λ��
 * \return  �ɹ�����true,���򷵻�false
 */
bool zXMLParser::getNodeContentStr(const xmlNodePtr node,std::string &content)
{
  char *temp=NULL;
  bool ret=true;
  if (node==NULL) return false;
  xmlNodePtr text=node->children;
  while(text!=NULL)
  {
    if (!xmlStrcmp(text->name,(const xmlChar *) "text"))
    {
      temp =(char *)text->content;
      break;
    }
    text=text->next;
  }
  if (temp ==NULL ) return false;
  BYTE *out = charConv((BYTE*)temp,"UTF-8",(const char *)doc->encoding);
  if (out)
  {
    content = (char *)out;
    SAFE_DELETE_VEC(out);
  }
  return ret;
}

bool zXMLParser::getNodeContentStr(const xmlNodePtr node,std::string &content,bool head)
{
  if (node==NULL) return false;
  xmlBufferPtr out=xmlBufferCreate();
  if (xmlNodeDump(out,doc,node,1,1)!=-1)
  {    
    BYTE *cout = charConv((BYTE*)out->content,"UTF-8",(const char *)doc->encoding);
    if (cout)
    {
      if (head)
      {
        content="<?xml version=\"1.0\" encoding=\"";
        content+=(char *)doc->encoding;
        content+="\"?>";
      }
      else
        content="";
      content+=(char *)cout;      
      SAFE_DELETE_VEC(cout);
    }
  }
  xmlBufferFree(out);
  return true;

  #if 0
  char *temp=NULL;
  bool ret=true;
  if (node==NULL) return false;
  temp = (char *) xmlNodeGetContent(node);  
  if (temp ==NULL ) return false;
  BYTE *out = charConv((BYTE*)temp,"UTF-8",(const char *)doc->encoding);
  if (out)
  {  if (head)
    {
      content="<?xml version=\"1.0\" encoding=\"";
      content+=(char *)doc->encoding;
      content+="\"?>";
    }
    else
      content="";
    content+=(char *)out;
    SAFE_DELETE_VEC(out);
  }  
  
  return ret;
        #endif
}

//׼������xml�ַ���
//�����ڴ�ָ������ֹ��ͷ�
BYTE* zXMLParser::charConv(BYTE *in,const char *fromEncoding,const char * toEncoding)
{
  BYTE *out;
  size_t ret,size,out_size;

  size = strlen((char *)in); 
  out_size = size * 2 + 1; 
  out = new BYTE[out_size]; 
  bzero(out,out_size);
  if (out)
  {
    if (fromEncoding!=NULL && toEncoding!=NULL)
    {
      iconv_t icv_in = iconv_open(toEncoding,fromEncoding);
      if ((iconv_t)-1 == icv_in)
      {
        SAFE_DELETE_VEC(out);
        out = NULL;
      }
      else
      {
        const char *fromtemp = (char *)in;
        char *totemp =(char *)out;
        size_t tempout = out_size-1;
        ret =iconv(icv_in,&fromtemp,&size,&totemp,&tempout);
        if ((size_t)-1 == ret)
        {
          SAFE_DELETE_VEC(out);
          out = NULL;
        }
        iconv_close(icv_in);
      }
    }
    else
      strncpy((char *)out,(char *)in,size);
  }
  return (out);
}
