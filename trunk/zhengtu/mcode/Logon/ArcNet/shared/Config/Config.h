//////////////////////////////////////////////////
/// @file : config.h
/// @brief : 
/// @date:  2012/10/23
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __configOne_H__
#define __configOne_H__

/*
 * @defgroup 模块名 configOne 使用方法和例子
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
#注释
[section_1]
foo  = bar
water= h2o
;注释
[section_2]
foo  = foo
water= wet
four = 4.2
*/
/** @} */ // 模块结尾

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