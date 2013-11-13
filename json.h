#ifndef __JSON_H
#define __JSON_H

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <map>

#define JSON_TYPE_NULL (-1)
#define JSON_TYPE_INT 0
#define JSON_TYPE_STRING 1
#define JSON_TYPE_VECTOR 2
#define JSON_TYPE_MAP 3

class json
{
public :
	typedef int int_type;
	typedef std::string string_type;
	typedef std::vector<json> vector_type;
	typedef std::map<std::string, json> map_type;
	
private :
	mutable int _type;
	int_type _int_value;
	mutable string_type* _str_value;
	mutable vector_type* _vec_value;
	mutable map_type* _map_value;

public :
	json() : _type(JSON_TYPE_NULL), _int_value(0), _str_value(nullptr), _vec_value(nullptr), _map_value(nullptr) { }
	explicit json(int int_value) : _type(JSON_TYPE_INT), _int_value(int_value), _str_value(nullptr), _vec_value(nullptr), _map_value(nullptr) { }
	explicit json(const string_type& string_value) : _type(JSON_TYPE_STRING), _int_value(0), _str_value(new string_type(string_value)), _vec_value(nullptr), _map_value(nullptr) { }
	explicit json(const vector_type& vector_value) : _type(JSON_TYPE_VECTOR), _int_value(0), _str_value(nullptr), _vec_value(new vector_type(vector_value)), _map_value(nullptr) { }
	explicit json(const map_type& map_value) : _type(JSON_TYPE_MAP), _int_value(0), _str_value(nullptr), _vec_value(nullptr), _map_value(new map_type(map_value)) { }
	json(const json& other) : _type(other._type), _int_value(other._int_value), _str_value(other._str_value ? new string_type(*other._str_value) : nullptr), _vec_value(other._vec_value ? new vector_type(*other._vec_value) : nullptr), _map_value(other._map_value ? new map_type(*other._map_value) : nullptr) { }
	virtual ~json() { clear(); }

	inline int type() const { return _type; }
	inline bool has_type() const { return _type != JSON_TYPE_NULL; }
	inline bool is_int() const { return _type == JSON_TYPE_INT; }
	inline bool is_string() const { return _type == JSON_TYPE_STRING; }
	inline bool is_vector() const { return _type == JSON_TYPE_VECTOR; }
	inline bool is_map() const { return _type == JSON_TYPE_MAP; }
	
	json& operator =(const json& other)
	{
		clear();
		_type = other._type;
		_int_value = other._int_value;
		if (other._str_value)
			_str_value = new string_type(*other._str_value);
		if (other._vec_value)
			_vec_value = new vector_type(*other._vec_value);
		if (other._map_value)
			_map_value = new map_type(*other._map_value);
		return *this;
	}

	inline void clear()
	{
		_type = JSON_TYPE_NULL;
		_int_value = 0;
		if (_str_value)
			delete _str_value;
		_str_value = nullptr;
		if (_vec_value)
			delete _vec_value;
		_vec_value = nullptr;
		if (_map_value)
			delete _map_value;
		_map_value = nullptr;
	}

	inline int_type& int_value()
	{
		if (_type == JSON_TYPE_NULL)
			_type = JSON_TYPE_INT;
		else if (_type != JSON_TYPE_INT)
			throw std::logic_error("Trying to access non-int json as int");
		return _int_value;
	}
	inline int_type int_value() const
	{
		if (_type == JSON_TYPE_NULL)
			_type = JSON_TYPE_INT;
		else if (_type != JSON_TYPE_INT)
			throw std::logic_error("Trying to access non-int json as int");
		return _int_value;
	}

	inline string_type& string_value()
	{
		if (_type == JSON_TYPE_NULL)
		{
			_type = JSON_TYPE_STRING;
			_str_value = new string_type();
		}
		else if (_type != JSON_TYPE_STRING)
			throw std::logic_error("Trying to access non-string json as string");
		return *_str_value;
	}
	inline const string_type& string_value() const
	{
		if (_type == JSON_TYPE_NULL)
		{
			_type = JSON_TYPE_STRING;
			_str_value = new string_type();
		}
		else if (_type != JSON_TYPE_STRING)
			throw std::logic_error("Trying to access non-string json as string");
		return *_str_value;
	}

	inline vector_type& vector_value()
	{
		if (_type == JSON_TYPE_NULL)
		{
			_type = JSON_TYPE_VECTOR;
			_vec_value = new vector_type();
		}
		else if (_type != JSON_TYPE_VECTOR)
			throw std::logic_error("Trying to access non-vector json as vector");
		return *_vec_value;
	}
	inline const vector_type& vector_value() const
	{
		if (_type == JSON_TYPE_NULL)
		{
			_type = JSON_TYPE_VECTOR;
			_vec_value = new vector_type();
		}
		else if (_type != JSON_TYPE_VECTOR)
			throw std::logic_error("Trying to access non-vector json as vector");
		return *_vec_value;
	}

	inline map_type& map_value()
	{
		if (_type == JSON_TYPE_NULL)
		{
			_type = JSON_TYPE_MAP;
			_map_value = new map_type();
		}
		else if (_type != JSON_TYPE_MAP)
			throw std::logic_error("Trying to access non-map json as map");
		return *_map_value;
	}
	inline const map_type& map_value() const
	{
		if (_type == JSON_TYPE_NULL)
		{
			_type = JSON_TYPE_MAP;
			_map_value = new map_type();
		}
		else if (_type != JSON_TYPE_MAP)
			throw std::logic_error("Trying to access non-map json as map");
		return *_map_value;
	}

private :
	static void put_string(std::ostream& os, const string_type& s)
	{
		os << '"';
		string_type::const_iterator end = s.end();
		for (string_type::const_iterator it = s.begin(); it != end; ++it)
		{
			char c = *it;
			switch (c)
			{
				case '\\' :
				case '"' :
				case '/' :
					os << '\\' << c;
				case '\b' :
					os << '\\' << 'b';
					break;
				case '\f' :
					os << '\\' << 'f';
					break;
				case '\n' :
					os << '\\' << 'n';
					break;
				case '\r' :
					os << '\\' << 'r';
					break;
				case '\t' :
					os << '\\' << 't';
					break;
				default :
					os << c;
					break;
			}
		}
		os << '"';
	}

public :
	friend std::ostream& operator <<(std::ostream& os, const json& value)
	{
		switch (value._type)
		{
			case JSON_TYPE_INT :
				os << value._int_value;
				break;
			case JSON_TYPE_STRING :
				put_string(os, *value._str_value);
				break;
			case JSON_TYPE_VECTOR :
				{
					os << '[';
					bool first = true;
					vector_type& vec = *value._vec_value;
					vector_type::iterator vend = vec.end();
					for (vector_type::iterator it = vec.begin(); it != vend; ++it)
					{
						if (first)
							first = false;
						else
							os << ',';
						os << *it;
					}
					os << ']';
				}
				break;
			case JSON_TYPE_MAP :
				{
					os << '{';
					bool first = true;
					map_type& map = *value._map_value;
					map_type::iterator mend = map.end();
					for (map_type::iterator it = map.begin(); it != mend; ++it)
					{
						if (first)
							first = false;
						else
							os << ',';
						put_string(os, it->first);
						os << ':' << it->second;
					}
					os << '}';
				}
				break;
			default :
				os << "null";
				break;
		}
		return os;
	}
};

#endif
