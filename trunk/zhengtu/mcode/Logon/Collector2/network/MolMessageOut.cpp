#include "stdafx.h"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

#include "MolMessageOut.h"

/// ��ʼ����Ϣ����������
const unsigned int INITIAL_DATA_CAPACITY = 16;
/// ������������
const unsigned int CAPACITY_GROW_FACTOR = 2;

/**
 * ���캯��
 */
CMolMessageOut::CMolMessageOut() :
     mPos(0)
{
    mData = (char*) malloc(INITIAL_DATA_CAPACITY);
    mDataSize = INITIAL_DATA_CAPACITY;
}

/**
 * ����ϢID�Ĺ��캯��
 *
 * @param id ��ϢID
 */
CMolMessageOut::CMolMessageOut(int id):
    mPos(0)
{
    mData = (char*) malloc(INITIAL_DATA_CAPACITY);
    mDataSize = INITIAL_DATA_CAPACITY;

    writeShort(id);
}

/**
 * ��������
 */
CMolMessageOut::~CMolMessageOut()
{
    if(mData) free(mData);
}

/**
 * �����ǰ��Ϣ
 */
void CMolMessageOut::clear()
{
    mData = (char *) realloc(mData, INITIAL_DATA_CAPACITY);
    mDataSize = INITIAL_DATA_CAPACITY;
    mPos = 0;
}

/**
 * дһ��BYTE��ȥ
 *
 * @param value Ҫд���BYTEֵ
 */
void CMolMessageOut::writeByte(int value)
{
    expand(mPos + 1);
    mData[mPos] = value;
    mPos += 1;
}

/** 
 * д��һ��BYTE��ȥ
 *
 * @param data Ҫд�������
 * @param length Ҫд������ݳ���
 */
void CMolMessageOut::writeBytes(char *data,unsigned long length)
{
	if(data == NULL || length <= 0) return;

	expand(mPos + length);
	memcpy(mData+mPos,data,length);
	mPos += length;
}

/**
 * ȷ���������������ܴ�
 *
 * @param size Ҫ����Ļ�������С
 */
void CMolMessageOut::expand(size_t bytes)
{
    if (bytes > mDataSize)
    {
        do
        {
            mDataSize *= CAPACITY_GROW_FACTOR;
        }
        while (bytes > mDataSize);

        mData = (char*) realloc(mData, mDataSize);
    }
}

/**
 * дһ��SHORT��ȥ
 *
 * @param value Ҫд���SHORTֵ
 */
void CMolMessageOut::writeShort(int value)
{
    expand(mPos + 2);
    u_short t = (u_short)(value);
    memcpy(mData + mPos, &t, 2);
    mPos += 2;
}

/**
 * дһ��LONG��ȥ
 *
 * @param value Ҫд���LONGֵ
 */
void CMolMessageOut::writeLong(unsigned long value)
{
    expand(mPos + 4);
    u_long t = (u_long)(value);
    memcpy(mData + mPos, &t, 4);
    mPos += 4;
}

/**  
 * дһ��FLOAT��ȥ
 *
 * @param value Ҫд���ֵ
 */
void CMolMessageOut::writeFloat(float value)
{
	expand(mPos + 4);
	memcpy(mData + mPos, &value, 4);
	mPos += 4;
}

/** 
 * дһ��DOUBLE��ȥ
 *
 * @param value Ҫд���ֵ
 */
void CMolMessageOut::writeDouble(double value)
{
	expand(mPos + 8);
	memcpy(mData + mPos, &value, 8);
	mPos += 8;
}

/**
 * дһ���ַ�����ȥ
 *
 * @param string Ҫд����ַ���
 * @param length �ַ�������
 */
void CMolMessageOut::writeString(const std::string &str, int length)
{
    int stringLength = (int)str.length();
    if (length < 0)
    {
        writeShort(stringLength);
        length = stringLength;
    }
    else if (length < stringLength)
    {
        stringLength = length;
    }
    expand(mPos + length);

    memcpy(mData + mPos, str.c_str(), stringLength);

    if (length > stringLength)
    {
        memset(mData + mPos + stringLength, '\0', length - stringLength);
    }
    mPos += length;
}

