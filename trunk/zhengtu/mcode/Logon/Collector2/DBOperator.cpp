#include "stdafx.h"

#include "DBOperator.h"
#include "database/dataproviderfactory.h"

initialiseSingleton(DBOperator);

/**
 * 构造函数
 */
DBOperator::DBOperator()
: m_DataProvider(NULL)
{
	m_DataProvider = DataProviderFactory::createDataProvider();
}

/**
 * 析构函数
 */
DBOperator::~DBOperator()
{
	Shutdown();

	if(m_DataProvider)
		delete m_DataProvider;
	m_DataProvider = NULL;
}

/**
 * 初始数据库
 *
 * @param host 要连接的数据库的IP地址
 * @param user 连接数据库的用户名
 * @param pass 连接数据库的用户密码
 * @param db 要连接的数据库名称
 * @param port 数据库端口号
 *
 * @return 如果数据库连接成功返回真，否则返回假
 */
bool DBOperator::Initilize(std::string host,std::string user,std::string pass,std::string db,int port)
{
	if(m_DataProvider == NULL)
		return false;

	return m_DataProvider->connect(host,user,pass,db,port);
}

/**
 * 关闭数据库连接
 */
void DBOperator::Shutdown(void)
{
	if(m_DataProvider == NULL)
		return;

	m_DataProvider->disconnect();
}

/** 
 * 根据玩家名称和密码检测这个玩家是否存在
 *
 * @param name 要检测的玩家的姓名
 * @param password 要检测的玩家的密码
 *
 * @return 如果玩家存在返回玩家的ID，不存在就返回-1
 */
unsigned int DBOperator::IsExistUser(std::string name,std::string password)
{
	if(m_DataProvider == NULL || name.empty() || password.empty()) return -1;

	std::ostringstream sqlstr;
	sqlstr << "select uid from mol_member where username='" << name << "' and password='" << password << "';";

	RecordSet pRecord = m_DataProvider->execSql(sqlstr.str());
	if(pRecord.isEmpty()) return -1;

	return atol(pRecord(0,0).c_str());
}

/** 
 * 检测指定游戏ID，指定游戏服务器名称的游戏房间是否存在
 *
 * @param gameId 要检测的游戏的ID
 * @param servername 要检测的服务器的名称
 *
 * @return 如果这个游戏房间存在返回真，否则返回假
 */
bool DBOperator::IsExistGameServer(unsigned int gameId,std::string servername)
{
	if(gameId < 0 || servername.empty())
		return false;

	std::ostringstream sqlstr;
	sqlstr << "select * from mol_room where gameId=" << gameId << " and name='" << servername << "';";

	RecordSet pRecord = m_DataProvider->execSql(sqlstr.str());
	if(pRecord.isEmpty()) return false;

	return true;
}

/** 
 * 根据用户ID得到用户的游戏数据
 *
 * @param UserId 要取得的用户的ID
 * @param UserData 如果取得用户数据成功，这里用于存储取得的用户数据
 *
 * @return 如果取得用户数据成功返回真，否则返回假
 */
bool DBOperator::GetUserData(unsigned int UserId,UserDataStru &UserData)
{
	if(m_DataProvider == NULL || UserId <= 0) return false;

	std::ostringstream sqlstr;
	sqlstr << "select * from mol_userdata where UserId=" << UserId << ";";

	RecordSet pRecord = m_DataProvider->execSql(sqlstr.str());
	if(pRecord.isEmpty()) return false;

	UserData.UserId = atol(pRecord(0,0).c_str());
	UserData.Money = _atoi64(pRecord(0,1).c_str());
	UserData.BankMoney = _atoi64(pRecord(0,2).c_str());
	UserData.Level = atoi(pRecord(0,3).c_str());
	UserData.Experience = atoi(pRecord(0,4).c_str());
	strcpy(UserData.UserAvatar , pRecord(0,5).c_str());
	UserData.TotalBureau = atoi(pRecord(0,6).c_str());
	UserData.SBureau = atoi(pRecord(0,7).c_str());
	UserData.FailBureau = atoi(pRecord(0,8).c_str());
	UserData.SuccessRate = atof(pRecord(0,9).c_str());
	UserData.RunawayRate = atof(pRecord(0,10).c_str());

	return true;
}