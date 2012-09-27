#ifndef _MOL_CONNECT_POOL_H_INCLUDE_
#define _MOL_CONNECT_POOL_H_INCLUDE_

/** 
* MolNet��������
*
* ����:���ڽ���һ�����ݿ�Ķ������
* ����:akinggw
* ����:2010.8.17
*/

#include "../network/MolMutex.h"
#include "mysql.h"

//#pragma comment(lib, "../libmysql/libmysql.lib")

#include <list>
#include <string>

class ConnectPool
{
public:
	/// ���캯��
	ConnectPool();
	/// ��������
	~ConnectPool();

	/// ��ʼ�����ӳ�
	bool Init(int max,std::string host,std::string user,std::string password,std::string db,int port);
	/// �ر����ӳ�
	void Close(void);

	/// ���½������ݿ�����
	bool ResetConnet(MYSQL* pMysql);
	/// �õ����ݿ�����
	MYSQL* GetConnet();
	/// �ͷ����ݿ�����
	void PutConnet(MYSQL* pMysql);

private:
	std::list<MYSQL*> m_connlist;                /**< mysql�����Ӳ����б� */
	Mutex m_connlistlock;                        /**< ���ӳ��� */

	int m_max;                                   /**< ���ݿ����ӵ������� */
	int m_BusyCount;                             /**< ��ǰʹ�õ����ӵĸ��� */

	std::string m_host;                          /**< ���ݿ�IP��ַ */
	std::string m_user;                          /**< ���ݿ��û� */
	std::string m_password;                      /**< ���ݿ��û����� */
	std::string m_db;                            /**< Ҫ���ӵķ����������� */
	int m_port;                                  /**< ���ݿ�˿ں� */
};

#endif