#include "globals.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>

#include "json.h"
#include "nagios_host.h"
#include "nagios_perfdata.h"
#include "nagios_service.h"
#include "string_map.h"
#include "strutil.h"

using namespace std;

map<string, nagios_host> hosts;
string_map configuration;
string_map environment;

inline nagios_host& host(const string& host_name)
{
	nagios_host& hst = hosts[host_name];
	if (host_name.size() != 0 && hst.host_name().size() == 0)
		hst.host_name() = host_name;
	return hst;
}

void fill_status(nagios_service& svc, map<string, string>& data)
{
	svc.current_state() = stoi(data["current_state"]);
	svc.state_type() = stoi(data["state_type"]);
	svc.plugin_output() = data["plugin_output"];
	nagios_perfdata::parse_all(svc.performance_data(), data["performance_data"]);
	svc.is_flapping() = stoi(data["is_flapping"]) != 0;
}
void fill_object(nagios_host& hst, map<string, string>& data)
{
	hst.alias() = data["alias"];
	hst.display_name() = data["display_name"];
	hst.icon_image() = data["icon_image"];
}

void read_status(istream& file)
{
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
					if (stoi(object_data["active_checks_enabled"]) != 0 && object_data["check_period"] != "" && object_data["check_period"] != "none")
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
void read_objects(istream& file)
{
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

json generate_json(const string& host_prefix, const string& alias_prefix, const string& display_prefix)
{
	string::size_type host_prefix_len = host_prefix.size();
	string::size_type alias_prefix_len = alias_prefix.size();
	string::size_type display_prefix_len = display_prefix.size();
	json j;
	vector<json>& vec = j.vector_value();
	map<string, nagios_host>::iterator end = hosts.end();
	for (map<string, nagios_host>::iterator it = hosts.begin(); it != end; ++it)
	{
		nagios_host& host = it->second;
		if (host.services().size() &&
			(!host_prefix_len || starts_with(host.host_name(), host_prefix)) &&
			(!alias_prefix_len || starts_with(host.alias(), alias_prefix)) &&
			(!display_prefix_len || starts_with(host.display_name(), display_prefix))) {
			if (host_prefix_len)
				trim(host.host_name() = host.host_name().substr(host_prefix_len));
			if (alias_prefix_len)
				trim(host.alias() = host.alias().substr(alias_prefix_len));
			if (display_prefix_len)
				trim(host.display_name() = host.display_name().substr(display_prefix_len));
			vec.emplace_back(host);
		}
	}
	return j;
}
void write_json(const char* to_file, const string& host_prefix, const string& alias_prefix, const string& display_prefix)
{
	ofstream file(to_file);
	file << generate_json(host_prefix, alias_prefix, display_prefix);
}

int main(int argc, char** argv, char** envp)
{
	if (argc != 1 && argc != 2)
	{
		cerr << "Usage : " << argv[0] << " [config-file]" << endl;
		return 1;
	}
	parse_string_map(environment, envp);
	{
		string cfgfile;
		if (argc == 2)
			cfgfile = argv[1];
		else
		{
			string_map::iterator envcfg = environment.find("NAGIOS_JSON_CONFIG");
			if (envcfg != environment.end())
				cfgfile = envcfg->second;
			else
				cfgfile = "/etc/nagios-json.conf";
		}
		ifstream cfgstream(cfgfile);
		parse_string_map(configuration, cfgstream);
	}
	{
		ifstream ifs(configuration["status-file"]);
		read_status(ifs);
	}
	{
		ifstream ifs(configuration["objects-file"]);
		read_objects(ifs);
	}
	string_map::iterator SERVER_PROTOCOL = environment.find("SERVER_PROTOCOL");
	bool cgi = SERVER_PROTOCOL != environment.end();
	if (cgi)
	{
		cout << "Status: 200 OK" << endl;
		cout << "Content-Type: application/json; charset=utf-8" << endl;
		cout << "Cache-Control: no-cache, no-store, must-revalidate" << endl;
		cout << "Pragma: no-cache" << endl;
		cout << "Expires: 0" << endl;
		cout << endl;
	}
	cout << generate_json(
		configuration["users." + environment["REMOTE_USER"] + ".host-prefix"],
		configuration["users." + environment["REMOTE_USER"] + ".alias-prefix"],
		configuration["users." + environment["REMOTE_USER"] + ".display-prefix"]) << endl;
	return 1;
}
