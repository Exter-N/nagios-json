#ifndef __NAGIOS_RANGE_H
#define __NAGIOS_RANGE_H

#include <cmath>

#include "json.h"

class nagios_range
{
private:
	double _minimum;
	double _maximum;
	bool _inside;

public:
	static const nagios_range empty_range;
	
	nagios_range(double minimum, double maximum, bool inside) : _minimum(minimum), _maximum(maximum), _inside(inside) { }
	
	inline double& minimum() { return _minimum; }
	inline double minimum() const { return _minimum; }
	
	inline double& maximum() { return _maximum; }
	inline double maximum() const { return _maximum; }
	
	inline bool& inside() { return _inside; }
	inline bool inside() const { return _inside; }
	
	inline bool empty() const { return !(std::isfinite(_minimum) || std::isfinite(_maximum) || _inside); }
	
	static nagios_range parse(const std::string::const_iterator& begin, const std::string::const_iterator& end);
	inline static nagios_range parse(const std::string& value)
	{
		return parse(value.begin(), value.end());
	}
	
	operator json() const;
};

#endif
