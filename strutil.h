#ifndef __STRUTIL_H
#define __STRUTIL_H

#include <string>

void trim(std::string& s);
bool getnumber(std::string::const_iterator& begin, const std::string::const_iterator& end, double& value);
bool starts_with(const std::string& haystack, const std::string& needle);

#endif