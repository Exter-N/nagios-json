#include "globals.h"

#include <cctype>
#include <string>

#include "strutil.h"

using namespace std;

void trim(string& s)
{
	string::size_type start = 0;
	while (s.size() > start && isspace(s[start]))
		++start;
	string::size_type curend = s.size() - 1;
	string::size_type end = curend;
	while (end > start && isspace(s[end]))
		--end;
	if (start != 0 || end != curend)
		s = s.substr(start, end + 1 - start);
}
bool getnumber(string::const_iterator& begin, const string::const_iterator& end, double& value)
{
	char c;
	if (begin != end && ((c = *begin) == '.' || c == ',' || c == '-' || isdigit(c)))
	{
		string num;
		do
		{
			num.push_back((c == ',') ? '.' : c);
			++begin;
		} while (begin != end && ((c = *begin) == '.' || c == ',' || isdigit(c)));
		value = stod(num);
		return true;
	}
	return false;
}
bool starts_with(const string& haystack, const string& needle)
{
	return haystack.size() >= needle.size() && haystack.compare(0, needle.size(), needle) == 0;
}