#include "globals.h"

#include <map>
#include <string>

#include "json.h"
#include "nagios_host.h"

using namespace std;

nagios_host::operator json() const
{
	json j;
	map<string, json>& map(j.map_value());
	map["host_name"].string_value() = _name;
	if (_alias.size())
		map["alias"].string_value() = _alias;
	if (_display_name.size())
		map["display_name"].string_value() = _display_name;
	if (_icon_image.size())
		map["icon_image"].string_value() = _icon_image;
	vector<json>& j_services = map["services"].vector_value();
	std::map<string, nagios_service>::const_iterator end = _services.end();
	for (std::map<string, nagios_service>::const_iterator it = _services.begin(); it != end; ++it)
		j_services.emplace_back(it->second);
	return j;
}