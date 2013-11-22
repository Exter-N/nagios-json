#include "globals.h"

#include <istream>
#include <map>
#include <string>
#include <utility>

#include "string_map.h"
#include "strutil.h"

using namespace std;

void parse_string_pair(string_map& environment, const string& row)
{
	string::size_type pos = string::npos;
	if ((pos = row.find('=')) != string::npos)
	{
		string k(row.substr(0, pos));
		string v(row.substr(pos + 1));
		trim(k);
		trim(v);
		environment.insert(make_pair(k, v));
	}
	else
	{
		string s(row);
		trim(s);
		environment.insert(make_pair(s, string()));
	}
}

void parse_string_map(string_map& environment, char** envp)
{
	while (envp && *envp)
	{
		if (**envp)
			parse_string_pair(environment, *envp);
		++envp;
	}
}

void parse_string_map(string_map& environment, istream& envs)
{
	string s;
	while (getline(envs, s))
	{
		trim(s);
		if (s.size() > 0 && s[0] != '#')
			parse_string_pair(environment, s);
	}
}
