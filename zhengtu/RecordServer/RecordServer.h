/**
 * \brief zebra项目档案服务器，用于创建、储存和读取档案
 *
 */
#pragma once

//#include <iostream>
//#include <set>

#include <zebra/srvEngine.h>
#include <mysql.h>


#pragma comment(lib,"ws2_32.lib")

/**
 * \brief 定义读档连接任务类
 *
 */
class RecordTask : public zTCPTask
{

  public:

    /**
     * \brief 构造函数
     * 因为档案数据已经压缩过,在通过底层传送的时候就不需要压缩了
     * \param pool 所属连接池指针
     * \param sock TCP/IP套接口
     * \param addr 地址
     */
    RecordTask(
        zTCPTaskPool *pool,
        const SOCKET sock,
        const struct sockaddr_in *addr = NULL) : zTCPTask(pool,sock,addr,false)
    {
      wdServerID = 0;
      wdServerType = UNKNOWNSERVER;
    }

    /**
     * \brief 虚析构函数
     *
     */
    virtual ~RecordTask() {};

    int verifyConn();
    int recycleConn();
    bool msgParse(const Cmd::t_NullCmd *,const DWORD);

    /**
     * \brief 获取服务器编号
     *
     * \return 服务器编号
     */
    const WORD getID() const
    {
      return wdServerID;
    }

    /**
     * \brief 获取服务器类型
     *
     * \return 服务器类型
     */
    const WORD getType() const
    {
      return wdServerType;
    }

  private:

    WORD wdServerID;          /**< 服务器编号,一个区唯一的 */
    WORD wdServerType;          /**< 服务器类型 */

    bool verifyLogin(const Cmd::Record::t_LoginRecord *ptCmd);
    bool msgParse_Gateway(const Cmd::t_NullCmd *,const DWORD);
    bool getSelectInfo(DWORD accid, DWORD countryid);
    bool msgParse_Scene(const Cmd::t_NullCmd *,const DWORD);
    bool msgParse_Session(const Cmd::t_NullCmd*,const DWORD);

    bool readCharBase(const Cmd::Record::t_ReadUser_SceneRecord *rev);
    bool saveCharBase(const Cmd::Record::t_WriteUser_SceneRecord *rev);

    static const dbCol charbase_define[];

#ifdef _TEST_DATA_LOG
    static const dbCol chartest_define[];
    bool readCharTest(Cmd::Record::t_Read_CharTest_SceneRecord *rev);
    bool insertCharTest(Cmd::Record::t_Insert_CharTest_SceneRecord *rev);
    bool updateCharTest(Cmd::Record::t_Update_CharTest_SceneRecord *rev);
    bool deleteCharTest(Cmd::Record::t_Delete_CharTest_SceneRecord *rev);
#endif
};

/**
 * \brief 定义档案服务类
 *
 * 项目档案服务器，用于创建、储存和读取档案<br>
 * 这个类使用了Singleton设计模式，保证了一个进程中只有一个类的实例
 *
 */
class RecordService : public zSubNetService
{

  public:

    bool msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

    /**
     * \brief 虚析构函数
     *
     */
    ~RecordService()
    {
      instance = NULL;

      //关闭线程池
      if (taskPool)
      {
        taskPool->final();
        SAFE_DELETE(taskPool);
      }
    }

    const int getPoolSize() const
    {
      if (taskPool)
      {
        return taskPool->getSize();
      }
      else
      {
        return 0;
      }
    }

    /**
     * \brief 返回唯一的类实例
     *
     * \return 唯一的类实例
     */
    static RecordService &getInstance()
    {
      if (NULL == instance)
        instance = new RecordService();

      return *instance;
    }

    /**
     * \brief 释放类的唯一实例
     *
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    void reloadConfig();

    /**
     * \brief 指向数据库连接池实例的指针
     *
     */
    static zDBConnPool *dbConnPool;

  private:

    /**
     * \brief 类的唯一实例指针
     *
     */
    static RecordService *instance;

    zTCPTaskPool *taskPool;        /**< TCP连接池的指针 */

    /**
     * \brief 构造函数
     *
     */
    RecordService() : zSubNetService("档案服务器",RECORDSERVER)
    {
      taskPool = NULL;
    }

    bool init();
    void newTCPTask(const SOCKET sock,const struct sockaddr_in *addr);
    void final();
};

/**
 * \brief 角色档案读取写入的会话记录
 *
 */
struct RecordSession
{
  DWORD accid;      /// 帐号
  DWORD id;        /// 角色编号
  WORD  wdServerID;    /// 服务器编号
  zTime lastsavetime;    /// 最后一次存档时间

  /**
   * \brief 缺省构造函数
   *
   */
  RecordSession(const DWORD accid,const DWORD id,const WORD wdServerID) : lastsavetime()
  {
    this->accid = accid;
    this->id = id;
    this->wdServerID = wdServerID;
  }

  /**
   * \brief 拷贝构造函数
   *
   */
  RecordSession(const RecordSession& rs)
  {
    accid = rs.accid;
    id = rs.id;
    wdServerID = rs.wdServerID;
    lastsavetime = rs.lastsavetime;
  }

  /**
   * \brief 赋值操作符号，没有实现，禁用掉了
   *
   */
  RecordSession & operator= (const RecordSession &rs);

  const bool operator== (const RecordSession &rs);
  /*{
    return (accid == rs.accid && id == rs.id);
  }*/

};

class RecordSessionManager
{

  public:

    /**
     * \brief 默认析构函数
     *
     */
    ~RecordSessionManager()
    {
      sessionMap.clear();
    }

    /**
     * \brief 返回类的唯一实例
     *
     * 实现了Singleton设计模式，保证了一个进程中只有一个类的实例
     *
     */
    static RecordSessionManager &getInstance()
    {
      if (NULL == instance)
        instance = new RecordSessionManager;

      return *instance;
    }

    /**
     * \brief 释放类的唯一实例
     *
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    bool add(const DWORD accid,const DWORD id,const WORD wdServerID);
    bool verify(const DWORD accid,const DWORD id,const WORD wdServerID);
    bool remove(const DWORD accid,const DWORD id,const WORD wdServerID);
    void removeAllByServerID(const WORD wdServerID);
    bool empty()
    {
      return sessionMap.empty(); 
    }

  private:

    /**
     * \brief 类的唯一实例指针
     *
     */
    static RecordSessionManager *instance;

    /**
     * \brief 默认构造函数
     *
     */
    RecordSessionManager() {};

    /**
     * \brief 定义容器类型
     *
     */
    typedef hash_map<DWORD,RecordSession> RecordSessionHashmap;
    /**
     * \brief 定义容器迭代器类型
     *
     */
    typedef RecordSessionHashmap::iterator RecordSessionHashmap_iterator;
    /**
     * \brief 定义容器键值对类型
     *
     */
    typedef RecordSessionHashmap::value_type RecordSessionHashmap_pair;
    /**
     * \brief 存储在线帐号列表信息的容器
     *
     */
    RecordSessionHashmap sessionMap;
    /**
     * \brief 互斥变量
     *
     */
    zMutex mlock;

};

#define  MAX_UZLIB_CHAR   (200 * 1024) 
#define MAX_NAMESIZE 32
#define MAX_SIZE 300
const DWORD PACKET_MASK           =   0x0000ffff;                 
const DWORD MAX_DATABUFFERSIZE    =   (PACKET_MASK + 1);          
const DWORD MAX_DATASIZE          =   (MAX_DATABUFFERSIZE - 4);

struct Data{
    DWORD size;
    char data[0];
};
