/**
 * \brief ������ʽ�ඨ��
 *
 */
#include "baseLib/regex.h"
#include <zebra/srvEngine.h>
#include <boost/regex.hpp>

const int zRegex::REG_UNKNOW(78325);
const int zRegex::REG_FLAGS(78326);
const int zRegex::REG_COMP(78327);
const int zRegex::REG_MATCH(78328);
const int zRegex::REG_MULTILINE(REG_NEWLINE);
const int zRegex::REG_DEFAULT(0);


/**
 * \brief �������� 
 */


/**
 * \brief ������ʽ���뺯��
 *
 * \param regex Ҫ�����������ʽ 
 * \param flags ����ѡ��,Ŀǰ֧��#REG_MULTILINE,#REG_DEFAULT,����㲻֪����ʲô������Ĭ��ֵ 
 * \return �����Ƿ�ɹ� 
 */


/**
 * \brief ��ʼƥ���ַ���,��ƥ��ǰ�뱣֤�Ѿ���ȷ������������ʽ#compile
 *
 * \param s Ҫƥ����ַ���
 * \return ƥ���Ƿ�ɹ� 
 */


/**
 * \brief �õ�ƥ������ַ���,�ڴ�֮ǰ�뱣֤�Ѿ���ȷ�ý���ƥ��#match
 *
 * \param s �õ����ַ���������s��
 * \param sub ���ַ�����λ�á�ע��ƥ����ַ���λ��Ϊ0,�������ַ����Դ�����.���ֵΪ31
 * \return ����s 
 */

bool zRegex::match(const char * target)
	{
		if(NULL == target)
			return false;
		boost::regex reg(_exp);
		if(boost::regex_search(std::string(target),reg))
		{
		    
		    char *temp = const_cast<char*>(target);
		    char *_first = strtok(temp,",");
			if(NULL == _first)
				return false;

			char *_second = strtok(NULL,",");
			if(NULL == _second)
				return false;

			first = atoi(_first);

			second = atoi(_second);


			/*size_t len = strlen(target) + 1;
            const char *ptr1 = strstr(target,"(");
			if(NULL == ptr1)
				return false;
			const char *ptr2 = strstr(ptr1+1,",");
			if(NULL == ptr2)
				return false;
			size_t l1 = ptr2 - ptr1;
			char *f1 = new char[l1];
			strncpy(f1,ptr1+1,l1);
			f1[l1-1] = '\0';
            first = atoi(f1);
			delete[] f1;

			const char *ptr3 = strstr(ptr2+1,")");
			if(NULL == ptr3)
				return false;
			l1 = ptr3 - ptr2 - 1;
			f1 = new char[l1];
			strncpy(f1,ptr2+1,l1);
			f1[l1-1] = '\0';
            second = atoi(f1);
			delete[] f1;
            */
			return true;
		}
		return false;
	}
/**
 * \brief �õ�������Ϣ 
 * \return ������#compile��#matchʱ����false,�����ô˵õ�������Ϣ
 */
const std::string & zRegex::getError()
{
  if (errcode==REG_UNKNOW)
    errstr="unknow error";
  else if (errcode==REG_FLAGS)
    errstr="flags error";
  else if (errcode==REG_COMP)
    errstr="uncompiled error";
  else if (errcode==REG_MATCH)
    errstr="unmatched error";
  else
  {
    const char* temp = "unknow";// temp;
    //regerror(errcode,&preg,temp,1023);
	errstr=std::string(temp);
  }
  return errstr;
}
