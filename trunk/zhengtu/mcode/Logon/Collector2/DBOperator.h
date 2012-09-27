#ifndef _DB_OPERATOR_H_INCLUDE_
#define _DB_OPERATOR_H_INCLUDE_

#include "network/MolSingleton.h"
#include "database/dataprovider.h"

#include <string>

/** 
 * ����û���Ϣ
 */
struct UserDataStru
{
	int UserId;                   // ���ID
	__int64 Money;                // ��ҽ�Ǯ
	__int64 BankMoney;            // ���н�Ǯ
	int Level;                    // ��ҵȼ�
	int Experience;               // ��Ҿ���ֵ
	char UserAvatar[256];         // ���Avatar
	int TotalBureau;              // ����ܵľ���
	int SBureau;                  // ���ʤ������
	int FailBureau;               // ���ʧ�ܾ���
	float SuccessRate;            // ���ʤ������
	float RunawayRate;            // ������ܸ���  
};

/**
 * �����������Ϸ�����е����ݿ����
 */
class DBOperator : public Singleton<DBOperator>
{
public:
	/// ���캯��
	DBOperator();
	/// ��������
	~DBOperator();

	/// ��ʼ���ݿ�
	bool Initilize(std::string host,std::string user,std::string pass,std::string db,int port);
	/// �ر����ݿ�����
	void Shutdown(void);

	/// ����������ƺ��������������Ƿ����
	unsigned int IsExistUser(std::string name,std::string password);
	/// �����û�ID�õ��û�����Ϸ����
	bool GetUserData(unsigned int UserId,UserDataStru &UserData);
	/// ���ָ����ϷID��ָ����Ϸ���������Ƶ���Ϸ�����Ƿ����
	bool IsExistGameServer(unsigned int gameId,std::string servername);

private:
	DataProvider *m_DataProvider;                               /**< ���ڷ��ʱ������ݿ� */
};

#define ServerDBOperator DBOperator::getSingleton()

#endif
