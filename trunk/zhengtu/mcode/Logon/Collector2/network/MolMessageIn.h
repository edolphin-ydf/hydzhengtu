#ifndef MOLE2D_MESSAGE_IN_H
#define MOLE2D_MESSAGE_IN_H

/**
 * Mole2D ��Ϸ����
 *
 * ����ļ��������������һ����
 *
 * ���ߣ�akinggw
 * ����ʱ�䣺 2008.6.24
 */

#include <string>
#include <iosfwd>

#include "MolCommon.h"

//#include "VMemPool.h"

#include "MolString.h"

/**
* ���ڴ���������Ϣ
*/
class CMolMessageIn/* : public CVMemPool<CMolMessageIn>*/
{
public:
	/// ��ʼ�Ĺ��캯��
	CMolMessageIn();
	/// ���캯��
	CMolMessageIn(const char *data, int length);
	/// ��������
	~CMolMessageIn();

	/// �õ���ϢID
	int getId() const { return mId; }
	/// �õ���Ϣ���ܳ���
	int getLength() const { return mLength; }
	/// ��ȡһ��BYTE����
	int readByte();
	/// ��ȡһ��short����
	int readShort();
	/// ��ȡһ��long����
	unsigned long readLong();
	/// ��ȡһ��float����
	float readFloat();
	/// ��ȡһ��doubel����
	double readDouble();
	/// ��ȡһ���ַ������������Ϊ-1�Ļ������Ⱦʹ洢���ַ�����ʼ�ĵ�һ��short��
	CMolString readString(int length = -1);
	/// ����û�ж�ȡ�����ݵĳ���
	int getUnreadLength() const { return mLength - mPos; }

private:
	char mData[MOL_REV_BUFFER_SIZE_TWO];         /**< ���ݰ����� */
	unsigned short mLength;    /**< ���ݰ����ܳ��� */
	unsigned short mId;        /**< ���ݰ���ID */

	unsigned short mPos;       /**< ����ʵ�ʶ�ȡ��λ�� */
};

#endif
