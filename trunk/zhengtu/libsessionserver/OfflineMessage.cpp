/**
 * \brief ������Ϣ������ʵ��
 *
 */

#include <set>
#include <sys/stat.h>
#include <sys/types.h>
//#include <dirent.h>

#include <zebra/SessionServer.h>

#define FILEPOWER 509
#define MAX_MESSAGE_NUMBER 5

struct ltstring
{
  bool operator()(const std::string s1,const std::string s2) const
  {
    return s1<s2;
  }
};

/**
 * \brief ��ʼ����������ϵͳ��ɾ������������Ϣ��
 * \return ʧ�ܷ���false,�ɹ�����true
 */
bool COfflineMessage::init()
{
/*#ifndef HAVE_STRUCT_DIRENT_D_TYPE
  struct stat si;
#endif //HAVE_STRUCT_DIRENT_D_TYPE
  struct dirent *record;

  rootpath = Zebra::global["offlineMsgPath"];

  //mkdir(rootpath.c_str());

  _mkdir(rootpath.c_str());

  DIR* tDir = opendir(rootpath.c_str());
  if (tDir == NULL) return false;

  while(NULL != (record = readdir(tDir)))
  {
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    if (record->d_type == DT_REG)
#else //!HAVE_STRUCT_DIRENT_D_TYPE
    stat((rootpath + record->d_name).c_str(),&si);
    if (S_ISREG(si.st_mode))
#endif //!HAVE_STRUCT_DIRENT_D_TYPE  
    {
      Zebra::logger->debug("unlink(%s)",(rootpath + record->d_name).c_str());
      if (0 != unlink((rootpath + record->d_name).c_str()))
      {
        Zebra::logger->error("���������ϢĿ¼[%s]ʧ��",rootpath.c_str());
        return false;
      }
    }
  }
  closedir(tDir);*/
  return true;
}

/**
 * \brief д��������Ϣ
 *
 * \param type:  ��Ϣ����
 * \param id:  ��ɫID
 * \param pNullCmd: ��Ϣ����
 * \param cmdLen:  ��Ϣ����
 *
 */
void COfflineMessage::writeOfflineMessage(const BYTE &type,const DWORD &id,const Cmd::stNullUserCmd *pNullCmd,const DWORD cmdLen)
{


  /////////////////////////////////////////////
  /*std::set<std::string,ltstring> filelist;
  char buf[MAX_NAMESIZE];
  sprintf(buf,"/%u",id);
  std::string myPath = rootpath+buf;

  if (access(myPath.c_str(),F_OK) != 0)
  {
    mkdir(myPath.c_str());
  }

  sprintf(buf,"/%u",pNullCmd->byCmd);
  myPath = myPath+buf;

  if (access(myPath.c_str(),F_OK) != 0)
  {
    mkdir(myPath.c_str());
  }

  sprintf(buf,"/%u",pNullCmd->byParam);
  myPath = myPath+buf;

  if (access(myPath.c_str(),F_OK) != 0)
  {
        mkdir(myPath.c_str());
  }

  sprintf(buf,"/%u",type);
  myPath = myPath+buf;

  if (access(myPath.c_str(),F_OK) != 0)
  {
    mkdir(myPath.c_str());
  }
/// ��ʼ����ļ���Ŀ
#ifndef HAVE_STRUCT_DIRENT_D_TYPE
    struct stat si;
    char   szName[_MAX_PATH];
#endif //HAVE_STRUCT_DIRENT_D_TYPE
  struct dirent *record;

  DIR* tDir = opendir(myPath.c_str());
  if (tDir != NULL)
  {
    while(NULL != (record = readdir(tDir)))
    {
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
      if (record->d_type == DT_REG)
#else //!HAVE_STRUCT_DIRENT_D_TYPE
      snprintf(szName,sizeof(szName),"%s/%s",myPath.c_str(),record->d_name);
      stat(szName,&si);
      if (S_ISREG(si.st_mode))
#endif //!HAVE_STRUCT_DIRENT_D_TYPE  
      {
        filelist.insert(std::string(myPath+"/"+record->d_name));
      }
    }
    closedir(tDir);
  }
  while(filelist.size()>= MAX_MESSAGE_NUMBER) /// ���������Ϣ��Ŀ����ĳ������Ϣ��Ŀ����ɾ���ϵļ�¼�Ա���ָ��������
  {
    unlink(filelist.begin()->c_str());
    filelist.erase(filelist.begin());
  }

  do {
    zRTime ctv;
    sprintf(buf,"/%lu",ctv.sec());
  }while(access(std::string(myPath+buf).c_str(),F_OK) == 0);
    
  myPath = myPath+buf;  // ���յ�·���ļ���

//  Zebra::logger->debug("д��һ���µ�������Ϣ[%s]",myPath.c_str());
  int fd;

  if ((int)-1 != (fd = open(myPath.c_str(),O_CREAT|O_WRONLY,FILEPOWER)))
  {
    write(fd,pNullCmd,cmdLen);
    close(fd);
  }
  else
  {
    Zebra::logger->error("�޷�д���ɫ[%u]��������Ϣ",id);
  }*/

}

/**
 * \brief ���ߵĽ�ɫ�����Լ���������Ϣ
 * \param pUser:  ���ߵĽ�ɫ
 */
void COfflineMessage::getOfflineMessage(const UserSession *pUser)
{
  char buf[MAX_NAMESIZE];
  sprintf(buf,"/%u",pUser->id);
  std::string myPath = rootpath+buf;
  getOfflineMessageSetAndSend(pUser,myPath);
}

/**
 * \brief ��ָ��·����ʼ���ұ��㼫����Ŀ¼�е�������Ϣ������
 * \param pUser:  ��ɫ
 * \param path:  ·��
 */
void COfflineMessage::getOfflineMessageSetAndSend(const UserSession *pUser,std::string path)
{
///////////////////////

/*
#ifndef HAVE_STRUCT_DIRENT_D_TYPE
    struct stat si;
    char   szName[_MAX_PATH];
#endif //HAVE_STRUCT_DIRENT_D_TYPE
  struct dirent *record;
  std::set<std::string,ltstring> filelist;
  std::set<std::string,ltstring>::iterator tIterator;

  DIR* tDir = opendir(path.c_str());
  if (tDir == NULL) return;
  while(NULL != (record = readdir(tDir)))
  {
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    if (record->d_type == DT_DIR)
#else //!HAVE_STRUCT_DIRENT_D_TYPE
    snprintf(szName,sizeof(szName),"%s/%s",path.c_str(),record->d_name);
    stat(szName,&si);
    if (S_ISDIR(si.st_mode))
#endif //!HAVE_STRUCT_DIRENT_D_TYPE  
    {
      if (strcmp(record->d_name,".") == 0 ||  strcmp(record->d_name,"..") == 0) continue;

      getOfflineMessageSetAndSend(pUser,(path+"/"+record->d_name).c_str());
    }
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    else if (record->d_type == DT_REG)
#else //!HAVE_STRUCT_DIRENT_D_TYPE
        else if (S_ISREG(si.st_mode))
#endif //!HAVE_STRUCT_DIRENT_D_TYPE  
    {
      filelist.insert(std::string(path+"/"+record->d_name));
    }
  }
  closedir(tDir);

  for (tIterator = filelist.begin(); tIterator != filelist.end(); tIterator++)
  {
    int fd;
    DWORD cmdLen;
    BYTE buf[zSocket::MAX_DATASIZE];

    bzero(buf,zSocket::MAX_DATASIZE);
    if ((int)-1 != (fd = open((*tIterator).c_str(),O_RDONLY)))
    {
      cmdLen = read(fd,buf,zSocket::MAX_DATASIZE);
      pUser->sendCmdToMe(buf,cmdLen);
      close(fd);
    }
    else
    {
      Zebra::logger->error("�޷���ȡ��ɫ[%s]��������Ϣ",pUser->name);
    }
    unlink(tIterator->c_str());
  }

  rmdir(path.c_str());
  return;*/
}
