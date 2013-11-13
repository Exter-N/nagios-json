#include "globals.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>
#include "nagios_host.h"
#include "json.h"

using namespace std;

map<string, nagios_host> hosts;

inline nagios_host& host(const string& host_name)
{
	nagios_host& hst = hosts[host_name];
	if (host_name.size() != 0 && hst.host_name().size() == 0)
		hst.host_name() = host_name;
	return hst;
}

void trim(string& s)
{
	string::size_type start = 0;
	while (s.size() > start && (s[start] == ' ' || s[start] == '\t'))
		++start;
	string::size_type curend = s.size() - 1;
	string::size_type end = curend;
	while (end > start && (s[end] == ' ' || s[end] == '\t'))
		--end;
	if (start != 0 || end != curend)
		s = s.substr(start, end + 1 - start);
}

inline int atoi(const string& s) { return atoi(s.c_str()); }

void fill_status(nagios_service& svc, map<string, string>& data)
{
	svc.current_state() = atoi(data["current_state"]);
	svc.state_type() = atoi(data["state_type"]);
	svc.plugin_output() = data["plugin_output"];
	svc.performance_data() = data["performance_data"];
	svc.is_flapping() = atoi(data["is_flapping"]) != 0;
}
void fill_object(nagios_host& hst, map<string, string>& data)
{
	hst.alias() = data["alias"];
	hst.display_name() = data["display_name"];
	hst.icon_image() = data["icon_image"];
}

void read_status(const char* from_file)
{
	ifstream file(from_file);
	bool in_object = false, shall_store = false;
	map<string, string> object_data;
	string object_type;
	string s, k, v;
	string::size_type pos = string::npos;
	while (getline(file, s))
	{
		trim(s);
		if (!s.size())
			continue;
		if (!in_object)
		{
			if (s.size() > 2 && s.substr(pos = s.size() - 2) == " {")
			{
				in_object = true;
				object_type = s.substr(0, pos);
				shall_store = object_type == "hoststatus" || object_type == "servicestatus";
			}
		}
		else
		{
			if (s.size() == 1 && s[0] == '}')
			{
				if (object_type == "hoststatus")
				{
					if (atoi(object_data["active_checks_enabled"]) != 0 && object_data["check_period"] != "" && object_data["check_period"] != "none")
						fill_status(host(object_data["host_name"]).service("Ping"), object_data);
				}
				else if (object_type == "servicestatus")
					fill_status(host(object_data["host_name"]).service(object_data["service_description"]), object_data);
				object_data.clear();
				in_object = false;
			}
			else if (shall_store && (pos = s.find('=')) != string::npos)
			{
				k = s.substr(0, pos);
				v = s.substr(pos + 1);
				trim(k);
				trim(v);
				object_data[k] = v;
			}
		}
	}
}
void read_objects(const char* from_file)
{
	ifstream file(from_file);
	bool in_object = false, shall_store = false;
	map<string, string> object_data;
	string object_type;
	string s, k, v;
	string::size_type pos = string::npos;
	while (getline(file, s))
	{
		trim(s);
		if (!s.size())
			continue;
		if (!in_object)
		{
			if (s.size() > 9 && s.substr(0, 7) == "define " && s.substr(pos = s.size() - 2) == " {")
			{
				in_object = true;
				object_type = s.substr(7, pos - 7);
				shall_store = object_type == "host";
			}
		}
		else
		{
			if (s.size() == 1 && s[0] == '}')
			{
				if (object_type == "host")
					fill_object(host(object_data["host_name"]), object_data);
				object_data.clear();
				in_object = false;
			}
			else if (shall_store && (pos = s.find('\t')) != string::npos)
			{
				k = s.substr(0, pos);
				v = s.substr(pos + 1);
				trim(k);
				trim(v);
				object_data[k] = v;
			}
		}
	}
}
void write_json(const char* to_file)
{
	ofstream file(to_file);
	json root;
	vector<json>& j_hosts = root.vector_value();
	map<string, nagios_host>::iterator end = hosts.end();
	for (map<string, nagios_host>::iterator it = hosts.begin(); it != end; ++it)
	{
		nagios_host& host = it->second;
		if (host.services().size())
		{
			json j_host;
			map<string, json>& j_hmap = j_host.map_value();
			j_hmap["host_name"].string_value() = host.host_name();
			if (host.alias().size())
				j_hmap["alias"].string_value() = host.alias();
			if (host.display_name().size())
				j_hmap["display_name"].string_value() = host.display_name();
			if (host.icon_image().size())
				j_hmap["icon_image"].string_value() = host.icon_image();
			vector<json>& j_services = j_hmap["services"].vector_value();
			map<string, nagios_service>::iterator end2 = host.services().end();
			for (map<string, nagios_service>::iterator it2 = host.services().begin(); it2 != end2; ++it2)
			{
				nagios_service& service = it2->second;
				json j_service;
				map<string, json>& j_smap = j_service.map_value();
				j_smap["service_description"].string_value() = service.service_description();
				j_smap["current_state"].int_value() = service.current_state();
				j_smap["state_type"].int_value() = service.state_type();
				if (service.plugin_output().size())
					j_smap["plugin_output"].string_value() = service.plugin_output();
				if (service.performance_data().size())
					j_smap["performance_data"].string_value() = service.performance_data();
				j_smap["is_flapping"].int_value() = service.is_flapping() ? 1 : 0;
				j_services.push_back(j_service);
			}
			j_hosts.push_back(j_host);
		}
	}
	file << root;
}

int main(int argc, char** argv, char** envp)
{
	if (argc != 1 && argc != 4)
	{
		cerr << "Usage : " << argv[0] << " [status-file objects-file json-file]" << endl;
		return 1;
	}
	read_status((argc == 4) ? argv[1] : "/var/cache/nagios3/status.dat");
	read_objects((argc == 4) ? argv[2] : "/var/cache/nagios3/objects.cache");
	write_json((argc == 4) ? argv[3] : "/var/www/nagios.json");
	return 0;
}
