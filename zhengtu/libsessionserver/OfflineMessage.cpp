/**
 * \brief 离线消息管理类实现
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
 * \brief 初始化离线聊天系统，删除所有离线信息，
 * \return 失败返回false,成功返回true
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
        Zebra::logger->error("清空离线消息目录[%s]失败",rootpath.c_str());
        return false;
      }
    }
  }
  closedir(tDir);*/
  return true;
}

/**
 * \brief 写入离线消息
 *
 * \param type:  消息类型
 * \param id:  角色ID
 * \param pNullCmd: 消息命令
 * \param cmdLen:  消息长度
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
/// 开始检查文件数目
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
  while(filelist.size()>= MAX_MESSAGE_NUMBER) /// 如果保存消息数目超过某类型消息数目，则删除老的记录以保持指定的数量
  {
    unlink(filelist.begin()->c_str());
    filelist.erase(filelist.begin());
  }

  do {
    zRTime ctv;
    sprintf(buf,"/%lu",ctv.sec());
  }while(access(std::string(myPath+buf).c_str(),F_OK) == 0);
    
  myPath = myPath+buf;  // 最终的路径文件名

//  Zebra::logger->debug("写入一个新的离线消息[%s]",myPath.c_str());
  int fd;

  if ((int)-1 != (fd = open(myPath.c_str(),O_CREAT|O_WRONLY,FILEPOWER)))
  {
    write(fd,pNullCmd,cmdLen);
    close(fd);
  }
  else
  {
    Zebra::logger->error("无法写入角色[%u]的离线消息",id);
  }*/

}

/**
 * \brief 上线的角色查找自己的离线消息
 * \param pUser:  上线的角色
 */
void COfflineMessage::getOfflineMessage(const UserSession *pUser)
{
  char buf[MAX_NAMESIZE];
  sprintf(buf,"/%u",pUser->id);
  std::string myPath = rootpath+buf;
  getOfflineMessageSetAndSend(pUser,myPath);
}

/**
 * \brief 从指定路径开始查找本层极其子目录中的离线消息并发送
 * \param pUser:  角色
 * \param path:  路径
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
      Zebra::logger->error("无法读取角色[%s]的离线消息",pUser->name);
    }
    unlink(tIterator->c_str());
  }

  rmdir(path.c_str());
  return;*/
}
