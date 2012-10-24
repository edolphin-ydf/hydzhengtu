//////////////////////////////////////////////////
/// @file : config.h
/// @brief : 
/// @date:  2012/10/23
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __configOne_H__
#define __configOne_H__

/*
 * @defgroup ģ���� configOne ʹ�÷���������
 * @{
 */
/*
ConfigFile cf("config.txt");

std::string foo;
std::string water;
double      four;

foo   = cf.Value("section_1","foo"  );
water = cf.Value("section_2","water");
four  = cf.Value("section_2","four" );
#ע��
[section_1]
foo  = bar
water= h2o
;ע��
[section_2]
foo  = foo
water= wet
four = 4.2
*/
/** @} */ // ģ���β

#include "Chameleon.h"
#include <map>

class configOne
{
	std::map<std::string,Chameleon> content_;

public:
	configOne();
	bool SetSource(std::string const& configFile);

	Chameleon const& Value(std::string const& section, std::string const& entry) const;

	Chameleon const& Value(std::string const& section, std::string const& entry, double value);
	Chameleon const& Value(std::string const& section, std::string const& entry, std::string const& value);

};

extern configOne Config;
#endif