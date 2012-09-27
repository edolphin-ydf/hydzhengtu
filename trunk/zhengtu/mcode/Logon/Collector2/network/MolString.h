#ifndef _MOL_STRING_H_INCLUDE_
#define _MOL_STRING_H_INCLUDE_

/**
 * Mole2D ��Ϸ����
 *
 * ����ļ��������������һ����
 *
 * ���ߣ�akinggw
 * ����ʱ�䣺 2010.8.13
 */

#include "MolCommon.h"

/** 
 * �Զ����ַ��������࣬��Ҫ���ڽ���ַ��������紫���б�������
 */
class CMolString
{
public:
	/// ���캯��
	CMolString()
		: count(0)
	{ }
	/// �������Ĺ��캯��
	CMolString(std::string str)
		: count(0)
	{
		memset(array,0,MOL_STR_BUFFER_SIZE);
		strcpy(array,str.c_str());
		array[(int)str.length()] = '\0';
		count = (int)str.length();
	}
	/// �������Ĺ��캯��2
	CMolString(const char* begin,int length)
		: count(0)
	{
		memset(array,0,MOL_STR_BUFFER_SIZE);
		memcpy((void*)array,begin,length);
		array[length] = '\0';
		count = length;
	}

	/// �õ��ַ���
	char* c_str(void) { return array; }
	/// ȡ��ָ��λ�õ��ַ���
	char at(int index) 
	{
		if(index < 0 || index >= count)
			return -1;

		return array[index];
	}
	/// ����ַ���
	void clear(void)
	{
		if(array) free(array);
		count = 0;
	}
	/// ����ַ����Ƿ�Ϊ��
	bool empty(void) 
	{
		return count > 0 ? false : true;
	}
	/// �õ��ַ����ĳ���
	int length(void)
	{
		return count;
	}
	/// ��ֵ����
	CMolString& operator=(CMolString& other)
	{
		if(other.empty()) return *this;

		if(array) clear();

		memset(array,0,MOL_STR_BUFFER_SIZE);
		strcpy(array,other.c_str());
		array[other.length()] = '\0';
		count = other.length();

		return *this;
	}

private:
	char array[MOL_STR_BUFFER_SIZE];
	int count;
};

#endif