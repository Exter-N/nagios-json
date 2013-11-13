#ifndef __NAGIOS_SERVICE_H
#define __NAGIOS_SERVICE_H

#include <string>

class nagios_service
{
private:
	std::string _description;
	int _cur_state;
	int _state_type;
	std::string _output;
	std::string _performance;
	bool _flapping;

public:
	nagios_service() : _description(), _cur_state(-1), _state_type(-1), _output(), _performance(), _flapping(false) { }
	explicit nagios_service(const std::string& service_description) : _description(service_description), _cur_state(-1), _state_type(-1), _output(), _performance(), _flapping(false) { }

	inline std::string& service_description() { return _description; }
	inline const std::string& service_description() const { return _description; }
	inline int& current_state() { return _cur_state; }
	inline int current_state() const { return _cur_state; }
	inline int& state_type() { return _state_type; }
	inline int state_type() const { return _state_type; }
	inline std::string& plugin_output() { return _output; }
	inline const std::string& plugin_output() const { return _output; }
	inline std::string& performance_data() { return _performance; }
	inline const std::string& performance_data() const { return _performance; }
	inline bool& is_flapping() { return _flapping; }
	inline bool is_flapping() const { return _flapping; }
};

#endif
