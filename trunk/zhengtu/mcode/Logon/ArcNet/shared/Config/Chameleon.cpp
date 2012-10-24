//////////////////////////////////////////////////////////////////////////
///  Copyright(c) 1999-2012,TQ Digital Entertainment, All Right Reserved
///  Author:      hyd
///  Create:      2012/10/23
///
/// @file         Chameleon.cpp
/// @brief        
/// @version      1.0
//////////////////////////////////////////////////////////////////////////

#include "Chameleon.h"

#include <string>
#include <sstream>


Chameleon::Chameleon(std::string const& value) {
	value_=value;
}

#include <iostream>

Chameleon::Chameleon(const char* c) {
	value_=c;
}

Chameleon::Chameleon(double d) {
	std::stringstream s;
	s<<d;
	value_=s.str();
}

Chameleon::Chameleon(int d) {
	std::stringstream s;
	s<<d;
	value_=s.str();
}

Chameleon::Chameleon(float d) {
	std::stringstream s;
	s<<d;
	value_=s.str();
}

Chameleon::Chameleon(Chameleon const& other) {
	value_=other.value_;
}

Chameleon& Chameleon::operator=(Chameleon const& other) {
	value_=other.value_;
	return *this;
}

Chameleon& Chameleon::operator=(double i) {
	std::stringstream s;
	s << i;
	value_ = s.str();
	return *this;
}

Chameleon& Chameleon::operator=(int i) {
	std::stringstream s;
	s << i;
	value_ = s.str();
	return *this;
}

Chameleon& Chameleon::operator=(unsigned __int32 i) {
	std::stringstream s;
	s << i;
	value_ = s.str();
	return *this;
}

Chameleon& Chameleon::operator=(float i) {
	std::stringstream s;
	s << i;
	value_ = s.str();
	return *this;
}

Chameleon& Chameleon::operator=(std::string const& s) {
	value_=s;
	return *this;
}

Chameleon::operator std::string() const {
	return value_;
}

Chameleon::operator double() const {
	return atof(value_.c_str());
}

Chameleon::operator int() const {
	return atoi(value_.c_str());
}

Chameleon::operator unsigned __int32() const {
	return atoi(value_.c_str());
}

Chameleon::operator float() const {
	return (float)atof(value_.c_str());
}