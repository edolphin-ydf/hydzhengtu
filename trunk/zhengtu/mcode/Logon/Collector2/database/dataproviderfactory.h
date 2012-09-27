#ifndef _DATA_PROVIDER_FACTORY_H_INCLUDE
#define _DATA_PROVIDER_FACTORY_H_INCLUDE

/** 
* MolNet��������
*
* ����:���ڽ���mysql����
* ����:akinggw
* ����:2010.3.2
*/

#include "../network/MolCommon.h"

class DataProvider;

/** 
 * ���ڽ������ݿ�����һ��������
 */
class DataProviderFactory
{
public:
	/// ����һ���µ����ݿ������
	static DataProvider* createDataProvider(void);

private:
	/// ���캯��
	DataProviderFactory(void)
		throw();
	/// ��������
	~DataProviderFactory(void)
		throw();
};

#endif