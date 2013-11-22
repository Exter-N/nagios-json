#ifndef __NAGIOS_PERFDATA_H
#define __NAGIOS_PERFDATA_H

#include <string>

#include "json.h"
#include "nagios_range.h"

class nagios_perfdata
{
private:
	std::string _label;
	double _value;
	std::string _uom;
	nagios_range _warning;
	nagios_range _critical;
	double _minimum;
	double _maximum;

public:
	nagios_perfdata(const std::string& label, double value, const std::string& uom, const nagios_range& warning, const nagios_range& critical, double minimum, double maximum) : _label(label), _value(value), _uom(uom), _warning(warning), _critical(critical), _minimum(minimum), _maximum(maximum) { }

	inline std::string& label() { return _label; }
	inline const std::string& label() const { return _label; }
	
	inline double& value() { return _value; }
	inline double value() const { return _value; }

	inline std::string& uom() { return _uom; }
	inline const std::string& uom() const { return _uom; }

	inline nagios_range& warning() { return _warning; }
	inline const nagios_range& warning() const { return _warning; }

	inline nagios_range& critical() { return _critical; }
	inline const nagios_range& critical() const { return _critical; }
	
	inline double& minimum() { return _minimum; }
	inline double minimum() const { return _minimum; }
	
	inline double& maximum() { return _maximum; }
	inline double maximum() const { return _maximum; }
	
	static void parse_all(std::vector<nagios_perfdata>& destination, const std::string::const_iterator& begin, const std::string::const_iterator& end);
	inline static void parse_all(std::vector<nagios_perfdata>& destination, const std::string& values)
	{
		parse_all(destination, values.begin(), values.end());
	}
	
	operator json() const;
};

#endif
