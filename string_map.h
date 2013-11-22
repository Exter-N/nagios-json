#ifndef __STRING_MAP_H
#define __STRING_MAP_H

#include <istream>
#include <map>
#include <string>

typedef std::map<std::string, std::string> string_map;

void parse_string_pair(string_map& environment, const std::string& row);

void parse_string_map(string_map& environment, char** envp);
void parse_string_map(string_map& environment, std::istream& envs);

#endif
