/**
 * \brief 正则表达式类定义
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
 * \brief 析构函数 
 */


/**
 * \brief 正则表达式编译函数
 *
 * \param regex 要编译的正则表达式 
 * \param flags 编译选项,目前支持#REG_MULTILINE,#REG_DEFAULT,如果你不知道用什么建议用默认值 
 * \return 编译是否成功 
 */


/**
 * \brief 开始匹配字符串,在匹配前请保证已经正确编译了正则表达式#compile
 *
 * \param s 要匹配的字符串
 * \return 匹配是否成功 
 */


/**
 * \brief 得到匹配的子字符串,在此之前请保证已经正确得进行匹配#match
 *
 * \param s 得到的字符串将放入s中
 * \param sub 子字符串的位置。注意匹配的字符串位置为0,其他子字符串以此类推.最大值为31
 * \return 返回s 
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
 * \brief 得到错误信息 
 * \return 当进行#compile或#match时返回false,可以用此得到错误信息
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
