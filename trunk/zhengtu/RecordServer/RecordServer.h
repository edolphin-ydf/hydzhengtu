/**
 * \brief zebra��Ŀ���������������ڴ���������Ͷ�ȡ����
 *
 */
#pragma once

//#include <iostream>
//#include <set>

#include <zebra/srvEngine.h>
#include <mysql.h>


#pragma comment(lib,"ws2_32.lib")

/**
 * \brief �����������������
 *
 */
class RecordTask : public zTCPTask
{

  public:

    /**
     * \brief ���캯��
     * ��Ϊ���������Ѿ�ѹ����,��ͨ���ײ㴫�͵�ʱ��Ͳ���Ҫѹ����
     * \param pool �������ӳ�ָ��
     * \param sock TCP/IP�׽ӿ�
     * \param addr ��ַ
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
     * \brief ����������
     *
     */
    virtual ~RecordTask() {};

    int verifyConn();
    int recycleConn();
    bool msgParse(const Cmd::t_NullCmd *,const DWORD);

    /**
     * \brief ��ȡ���������
     *
     * \return ���������
     */
    const WORD getID() const
    {
      return wdServerID;
    }

    /**
     * \brief ��ȡ����������
     *
     * \return ����������
     */
    const WORD getType() const
    {
      return wdServerType;
    }

  private:

    WORD wdServerID;          /**< ���������,һ����Ψһ�� */
    WORD wdServerType;          /**< ���������� */

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
 * \brief ���嵵��������
 *
 * ��Ŀ���������������ڴ���������Ͷ�ȡ����<br>
 * �����ʹ����Singleton���ģʽ����֤��һ��������ֻ��һ�����ʵ��
 *
 */
class RecordService : public zSubNetService
{

  public:

    bool msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd,const DWORD nCmdLen);

    /**
     * \brief ����������
     *
     */
    ~RecordService()
    {
      instance = NULL;

      //�ر��̳߳�
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
     * \brief ����Ψһ����ʵ��
     *
     * \return Ψһ����ʵ��
     */
    static RecordService &getInstance()
    {
      if (NULL == instance)
        instance = new RecordService();

      return *instance;
    }

    /**
     * \brief �ͷ����Ψһʵ��
     *
     */
    static void delInstance()
    {
      SAFE_DELETE(instance);
    }

    void reloadConfig();

    /**
     * \brief ָ�����ݿ����ӳ�ʵ����ָ��
     *
     */
    static zDBConnPool *dbConnPool;

  private:

    /**
     * \brief ���Ψһʵ��ָ��
     *
     */
    static RecordService *instance;

    zTCPTaskPool *taskPool;        /**< TCP���ӳص�ָ�� */

    /**
     * \brief ���캯��
     *
     */
    RecordService() : zSubNetService("����������",RECORDSERVER)
    {
      taskPool = NULL;
    }

    bool init();
    void newTCPTask(const SOCKET sock,const struct sockaddr_in *addr);
    void final();
};

/**
 * \brief ��ɫ������ȡд��ĻỰ��¼
 *
 */
struct RecordSession
{
  DWORD accid;      /// �ʺ�
  DWORD id;        /// ��ɫ���
  WORD  wdServerID;    /// ���������
  zTime lastsavetime;    /// ���һ�δ浵ʱ��

  /**
   * \brief ȱʡ���캯��
   *
   */
  RecordSession(const DWORD accid,const DWORD id,const WORD wdServerID) : lastsavetime()
  {
    this->accid = accid;
    this->id = id;
    this->wdServerID = wdServerID;
  }

  /**
   * \brief �������캯��
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
   * \brief ��ֵ�������ţ�û��ʵ�֣����õ���
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
     * \brief Ĭ����������
     *
     */
    ~RecordSessionManager()
    {
      sessionMap.clear();
    }

    /**
     * \brief �������Ψһʵ��
     *
     * ʵ����Singleton���ģʽ����֤��һ��������ֻ��һ�����ʵ��
     *
     */
    static RecordSessionManager &getInstance()
    {
      if (NULL == instance)
        instance = new RecordSessionManager;

      return *instance;
    }

    /**
     * \brief �ͷ����Ψһʵ��
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
     * \brief ���Ψһʵ��ָ��
     *
     */
    static RecordSessionManager *instance;

    /**
     * \brief Ĭ�Ϲ��캯��
     *
     */
    RecordSessionManager() {};

    /**
     * \brief ������������
     *
     */
    typedef hash_map<DWORD,RecordSession> RecordSessionHashmap;
    /**
     * \brief ������������������
     *
     */
    typedef RecordSessionHashmap::iterator RecordSessionHashmap_iterator;
    /**
     * \brief ����������ֵ������
     *
     */
    typedef RecordSessionHashmap::value_type RecordSessionHashmap_pair;
    /**
     * \brief �洢�����ʺ��б���Ϣ������
     *
     */
    RecordSessionHashmap sessionMap;
    /**
     * \brief �������
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
