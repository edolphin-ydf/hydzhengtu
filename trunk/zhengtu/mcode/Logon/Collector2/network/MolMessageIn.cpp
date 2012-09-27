#include "stdafx.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

#include "MolMessageIn.h"

/** 
 * ��ʼ�Ĺ��캯��
 */
CMolMessageIn::CMolMessageIn() :
	mLength(0),
	mPos(0)
{

}

/**
 * ���캯��
 *
 * @param data ��Ӧ�İ�����
 * @param length �����ݵĳ���
 */
CMolMessageIn::CMolMessageIn(const char *data, int length) :
    mLength(0),
    mPos(0)
{
	if(mLength < MOL_REV_BUFFER_SIZE_TWO)
	{
		memcpy((void*)mData,data,length);
		mLength = length;
		mId = readShort();
	}
}

/** 
 * ��������
 */
CMolMessageIn::~CMolMessageIn()
{

}

/**
 * ��ȡһ��BYTE����
 */
int CMolMessageIn::readByte()
{
    int value = -1;
    if (mPos < mLength)
    {
        value = (unsigned char) mData[mPos];
    }
    mPos += 1;
    return value;
}

/**
 * ��ȡһ��short����
 */
int CMolMessageIn::readShort()
{
    int value = -1;
    if (mPos + 2 <= mLength)
    {
        u_short t;
        memcpy(&t, mData + mPos, 2);
        value = (unsigned short)(t);
    }
    mPos += 2;
    return value;
}

/**
 * ��ȡһ��long����
 */
unsigned long CMolMessageIn::readLong()
{
    unsigned long value = -1;
    if (mPos + 4 <= mLength)
    {
        u_long t;
        memcpy(&t, mData + mPos, 4);
        value = (unsigned long)(t);
    }
    mPos += 4;
    return value;
}

/// ��ȡһ��float����
float CMolMessageIn::readFloat()
{
	float value = -1;
	if (mPos + 4 <= mLength)
	{
		float t;
		memcpy(&t, mData + mPos, 4);
		value = (float)(t);
	}
	mPos += 4;
	return value;
}

/// ��ȡһ��doubel����
double CMolMessageIn::readDouble()
{
	double value = -1;
	if (mPos + 8 <= mLength)
	{
		double t;
		memcpy(&t, mData + mPos, 8);
		value = (double)(t);
	}
	mPos += 8;
	return value;
}

/**
 * ��ȡһ���ַ���
 *
 * @param length �������Ϊ-1�Ļ������Ⱦʹ洢���ַ�����ʼ�ĵ�һ��short��
 *
 * @return ���ض�ȡ���ַ���
 */
CMolString CMolMessageIn::readString(int length)
{
    if (length < 0)
    {
        length = readShort();
    }

    if (length < 0 || mPos + length > mLength)
    {
        mPos = mLength + 1;
        return CMolString();
    }

    char const *stringBeg = mData + mPos;
    char const *stringEnd = (char const *)memchr(stringBeg, '\0', length);
    CMolString readString(stringBeg,
            stringEnd ? stringEnd - stringBeg : length);
    mPos += length;

    return readString;
}
