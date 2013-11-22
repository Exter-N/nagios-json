#include "globals.h"

#include <map>
#include <string>

#include "json.h"
#include "nagios_service.h"

using namespace std;

nagios_service::operator json() const
{
	json j;
	map<string, json>& map(j.map_value());
	map["service_description"].string_value() = _description;
	map["current_state"].number_value() = _cur_state;
	map["state_type"].number_value() = _state_type;
	if (_output.size())
		map["plugin_output"].string_value() = _output;
	if (_performance.size())
		map["performance_data"] = json(_performance);
	map["is_flapping"].number_value() = _flapping ? 1 : 0;
	return j;
}