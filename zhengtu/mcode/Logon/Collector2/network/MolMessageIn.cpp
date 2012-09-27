#include "stdafx.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

#include "MolMessageIn.h"

/** 
 * 初始的构造函数
 */
CMolMessageIn::CMolMessageIn() :
	mLength(0),
	mPos(0)
{

}

/**
 * 构造函数
 *
 * @param data 相应的包数据
 * @param length 包数据的长度
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
 * 析构函数
 */
CMolMessageIn::~CMolMessageIn()
{

}

/**
 * 读取一个BYTE数据
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
 * 读取一个short数据
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
 * 读取一个long数据
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

/// 读取一个float数据
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

/// 读取一个doubel数据
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
 * 读取一个字符串
 *
 * @param length 如果长度为-1的话，长度就存储在字符串开始的第一个short中
 *
 * @return 返回读取的字符串
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
