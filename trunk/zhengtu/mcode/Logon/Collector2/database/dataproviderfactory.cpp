#include "stdafx.h"
#include "dataproviderfactory.h"

#include "mysqldataprovider.h"

/** 
 * ���캯��
 */
DataProviderFactory::DataProviderFactory(void)
	throw()
{

}

/** 
 * ��������
 */
DataProviderFactory::~DataProviderFactory(void)
	throw()
{

}

/** 
 * ����һ���µ����ݿ������
 *
 * @return �������ǽ��������ݿ������
 */
DataProvider* DataProviderFactory::createDataProvider(void)
{
	MySqlDataProvider *provider = new MySqlDataProvider();
	return provider;
}