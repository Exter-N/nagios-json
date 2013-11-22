#ifndef __JSON_H
#define __JSON_H

#include <cctype>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#define JSON_TYPE_NULL (-1)
#define JSON_TYPE_NUMBER 0
#define JSON_TYPE_STRING 1
#define JSON_TYPE_VECTOR 2
#define JSON_TYPE_MAP 3

class json
{
public :
	typedef double number_type;
	typedef std::string string_type;
	typedef std::vector<json> vector_type;
	typedef std::map<std::string, json> map_type;
	
private :
	mutable int _type;
	number_type _num_value;
	mutable string_type* _str_value;
	mutable vector_type* _vec_value;
	mutable map_type* _map_value;

public :
	inline json() : _type(JSON_TYPE_NULL), _num_value(0), _str_value(nullptr), _vec_value(nullptr), _map_value(nullptr) { }
	explicit inline json(number_type number_value) : _type(JSON_TYPE_NUMBER), _num_value(number_value), _str_value(nullptr), _vec_value(nullptr), _map_value(nullptr) { }
	explicit inline json(const string_type& string_value) : _type(JSON_TYPE_STRING), _num_value(0), _str_value(new string_type(string_value)), _vec_value(nullptr), _map_value(nullptr) { }
	explicit inline json(const vector_type& vector_value) : _type(JSON_TYPE_VECTOR), _num_value(0), _str_value(nullptr), _vec_value(new vector_type(vector_value)), _map_value(nullptr) { }
	explicit inline json(const map_type& map_value) : _type(JSON_TYPE_MAP), _num_value(0), _str_value(nullptr), _vec_value(nullptr), _map_value(new map_type(map_value)) { }
	template<typename T>
	explicit json(const std::vector<T>& data) : _type(JSON_TYPE_VECTOR), _num_value(0), _str_value(nullptr), _vec_value(new vector_type(data.size())), _map_value(nullptr)
	{
		vector_type::size_type size = _vec_value->size();
		for (vector_type::size_type i(0); i < size; ++i)
			(*_vec_value)[i] = json(data[i]);
	}
	inline json(const json& other) : _type(other._type), _num_value(other._num_value), _str_value(other._str_value ? new string_type(*other._str_value) : nullptr), _vec_value(other._vec_value ? new vector_type(*other._vec_value) : nullptr), _map_value(other._map_value ? new map_type(*other._map_value) : nullptr) { }
	inline json(json&& other) : _type(other._type), _num_value(other._num_value), _str_value(other._str_value), _vec_value(other._vec_value), _map_value(other._map_value)
	{
		other._type = JSON_TYPE_NULL;
		other._num_value = 0;
		other._str_value = nullptr;
		other._vec_value = nullptr;
		other._map_value = nullptr;
	}
	virtual ~json() { clear(); }

	inline int type() const { return _type; }
	inline bool has_type() const { return _type != JSON_TYPE_NULL; }
	inline bool is_number() const { return _type == JSON_TYPE_NUMBER; }
	inline bool is_string() const { return _type == JSON_TYPE_STRING; }
	inline bool is_vector() const { return _type == JSON_TYPE_VECTOR; }
	inline bool is_map() const { return _type == JSON_TYPE_MAP; }
	
	json& operator =(const json& other)
	{
		clear();
		_type = other._type;
		_num_value = other._num_value;
		if (other._str_value)
			_str_value = new string_type(*other._str_value);
		if (other._vec_value)
			_vec_value = new vector_type(*other._vec_value);
		if (other._map_value)
			_map_value = new map_type(*other._map_value);
		return *this;
	}
	json& operator =(json&& other)
	{
		clear();
		_type = other._type;
		_num_value = other._num_value;
		_str_value = other._str_value;
		_vec_value = other._vec_value;
		_map_value = other._map_value;
		other._type = JSON_TYPE_NULL;
		other._num_value = 0;
		other._str_value = nullptr;
		other._vec_value = nullptr;
		other._map_value = nullptr;
		return *this;
	}

	inline void clear()
	{
		_type = JSON_TYPE_NULL;
		_num_value = 0;
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

	inline number_type& number_value()
	{
		if (_type == JSON_TYPE_NULL)
			_type = JSON_TYPE_NUMBER;
		else if (_type != JSON_TYPE_NUMBER)
			throw std::logic_error("Trying to access non-number json as number");
		return _num_value;
	}
	inline number_type number_value() const
	{
		if (_type == JSON_TYPE_NULL)
			_type = JSON_TYPE_NUMBER;
		else if (_type != JSON_TYPE_NUMBER)
			throw std::logic_error("Trying to access non-number json as number");
		return _num_value;
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
			case JSON_TYPE_NUMBER :
				{
					long long llvalue(value._num_value);
					if (llvalue == value._num_value)
						os << llvalue;
					else
						os << std::to_string(value._num_value);
				}
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
