//////////////////////////////////////////////////
/// @file : Chameleon.h
/// @brief : 
/// @date:  2012/10/23
/// @author : hyd
//////////////////////////////////////////////////
#ifndef __Chameleon_H__
#define __Chameleon_H__

#include <string>
class Chameleon
{
public:
	Chameleon() {};
	explicit Chameleon(const std::string&);
	explicit Chameleon(double);
	explicit Chameleon(int);
	explicit Chameleon(float);
	explicit Chameleon(const char*);

	Chameleon(const Chameleon&);
	Chameleon& operator=(Chameleon const&);

	Chameleon& operator=(double);
	Chameleon& operator=(int);
	Chameleon& operator=(unsigned __int32);
	Chameleon& operator=(float);
	Chameleon& operator=(std::string const&);

public:
	operator std::string() const;
	operator double     () const;
	operator int        () const;
	operator unsigned __int32 () const;
	operator float      () const;
private:
	std::string value_;
	
};
#endif