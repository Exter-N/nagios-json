#ifndef __NAGIOS_HOST_H
#define __NAGIOS_HOST_H

#include <map>
#include <string>

#include "json.h"
#include "nagios_service.h"

class nagios_host
{
private:
	std::string _name;
	std::string _alias;
	std::string _display_name;
	std::string _icon_image;
	std::map<std::string, nagios_service> _services;

public:
	nagios_host() : _name(), _services() { }
	explicit nagios_host(const std::string& host_name) : _name(host_name), _services() { }

	inline std::string& host_name() { return _name; }
	inline const std::string& host_name() const { return _name; }

	inline std::string& alias() { return _alias; }
	inline const std::string& alias() const { return _alias; }

	inline std::string& display_name() { return _display_name; }
	inline const std::string& display_name() const { return _display_name; }

	inline std::string& icon_image() { return _icon_image; }
	inline const std::string& icon_image() const { return _icon_image; }

	inline std::map<std::string, nagios_service>& services() { return _services; }
	inline const std::map<std::string, nagios_service>& services() const { return _services; }

	inline nagios_service& service(const std::string& service_description)
	{
		nagios_service& svc = _services[service_description];
		if (service_description.size() != 0 && svc.service_description().size() == 0)
			svc.service_description() = service_description;
		return svc;
	}
	
	operator json() const;
};

#endif
