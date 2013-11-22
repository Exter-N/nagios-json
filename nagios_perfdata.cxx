#include "globals.h"

#include <cmath>
#include <map>
#include <string>

#include "json.h"
#include "lexer.h"
#include "nagios_range.h"
#include "nagios_perfdata.h"
#include "strutil.h"

using namespace std;

namespace
{
	class perfdata_token
	{
	public:
		virtual ~perfdata_token() { }
		virtual bool is_eos() const { return false; }
		virtual bool is_space() const { return false; }
		virtual bool is_separator() const { return false; }
		virtual bool is_number() const { return false; }
		virtual bool is_string() const { return false; }
		virtual bool is_range() const { return false; }
		virtual double number_value() const { throw logic_error("Cannot get number value of non-number token"); }
		virtual const string& string_value() const { throw logic_error("Cannot get string value of non-string token"); }
		virtual const nagios_range& range_value() const { throw logic_error("Cannot get range value of non-range token"); }
	};
	class eos_token : public perfdata_token
	{
	public:
		virtual bool is_eos() const { return true; }
	};
	class space_token : public perfdata_token
	{
	public:
		virtual bool is_space() const { return true; }
	};
	class separator_token : public perfdata_token
	{
	public:
		virtual bool is_separator() const { return true; }
	};
	class number_token : public perfdata_token
	{
	private:
		double _value;
	
	public:
		explicit number_token(double value) : _value(value) { }
		
		virtual bool is_number() const { return true; }
		virtual double number_value() const { return _value; }
	};
	class string_token : public perfdata_token
	{
	private:
		string _value;
	
	public:
		explicit string_token(const string& value) : _value(value) { }
		
		virtual bool is_string() const { return true; }
		virtual const string& string_value() const { return _value; }
	};
	class range_token : public perfdata_token
	{
	private:
		nagios_range _value;
	
	public:
		explicit range_token(const nagios_range& value) : _value(value) { }
		
		virtual bool is_range() const { return true; }
		virtual const nagios_range& range_value() const { return _value; }
	};
	
	class perfdata_lexer : public lexer<perfdata_token, string::const_iterator>
	{
	private:
		enum {
			label,
			equal,
			value,
			uom,
			sep1,
			warn,
			sep2,
			crit,
			sep3,
			min,
			sep4,
			max,
			extra
		} _state;
		
		void eat_token()
		{
			char c;
			while (_pos != _end && (c = *_pos) != ';' && !isspace(c))
				++_pos;
		}
	
	protected:
		virtual perfdata_token* next()
		{
		start_over:
			if (eat_spaces() > 0)
			{
				_state = label;
				return new space_token();
			}
			if (_pos == _end)
				return new eos_token();
			switch (_state)
			{
				case label:
				{
					char c;
					string label;
					if (*_pos == '\'')
					{
						++_pos;
						for (; ; )
						{
							if (_pos == _end)
								throw parse_error();
							if ((c = *_pos) == '\'')
							{
								++_pos;
								if (_pos != _end && *_pos == '\'')
								{
									label.push_back('\'');
									++_pos;
								}
								else
									break;
							}
							else
							{
								label.push_back(c);
								++_pos;
							}
						}
						trim(label);
					}
					else
					{
						string::const_iterator start(_pos);
						while (_pos != _end && (c = *_pos) != '=' && !isspace(c))
							++_pos;
						label = string(start, _pos);
					}
					_state = equal;
					return new string_token(label);
				}
				case equal:
					if (*_pos != '=')
						throw parse_error();
					++_pos;
					_state = value;
					return new separator_token();
				case value:
				{
					double value;
					if (*_pos == 'U')
					{
						value = NAN;
						++_pos;
					}
					else if (!getnumber(_pos, _end, value))
						throw parse_error();
					_state = uom;
					return new number_token(value);
				}
				case uom:
				{
					string::const_iterator start(_pos);
					eat_token();
					_state = sep1;
					if (start != _pos)
						return new string_token(string(start, _pos));
					else // self tail-recursion
						goto start_over;
				}
				case sep1:
					if (*_pos != ';')
						throw parse_error();
					++_pos;
					_state = warn;
					return new separator_token();
				case warn:
				{
					string::const_iterator start(_pos);
					eat_token();
					_state = sep2;
					if (start != _pos)
						return new range_token(nagios_range::parse(start, _pos));
					else // self tail-recursion
						goto start_over;
				}
				case sep2:
					if (*_pos != ';')
						throw parse_error();
					++_pos;
					_state = crit;
					return new separator_token();
				case crit:
				{
					string::const_iterator start(_pos);
					eat_token();
					_state = sep3;
					if (start != _pos)
						return new range_token(nagios_range::parse(start, _pos));
					else // self tail-recursion
						goto start_over;
				}
				case sep3:
					if (*_pos != ';')
						throw parse_error();
					++_pos;
					_state = min;
					return new separator_token();
				case min:
				{
					double value;
					_state = sep4;
					if (getnumber(_pos, _end, value))
						return new number_token(value);
					else // self tail-recursion
						goto start_over;
				}
				case sep4:
					if (*_pos != ';')
						throw parse_error();
					++_pos;
					_state = max;
					return new separator_token();
				case max:
				{
					double value;
					_state = extra;
					if (getnumber(_pos, _end, value))
						return new number_token(value);
					else // self tail-recursion
						goto start_over;
				}
				case extra:
					throw parse_error();
				default:
					throw logic_error("perfdata_lexer state machine error");
			}
		}
	
	public:
		perfdata_lexer(const string::const_iterator& begin, const string::const_iterator& end) : lexer(begin, end), _state(label) { }
	};
}

// value = U => NAN <math.h>
// 'label'=value[uom][;[warn][;[crit][;[min][;[max]]]]]
void nagios_perfdata::parse_all(vector<nagios_perfdata>& dest, const string::const_iterator& begin, const string::const_iterator& end)
{
	perfdata_lexer lexer(begin, end);
	if (lexer->is_space())
		++lexer;
	while (!lexer->is_eos())
	{
		if (!lexer->is_string())
			throw parse_error();
		string label(lexer->string_value());
		++lexer;
		if (!lexer->is_separator())
			throw parse_error();
		++lexer;
		if (!lexer->is_number())
			throw parse_error();
		double value(lexer->number_value());
		++lexer;
		string uom;
		if (lexer->is_string())
		{
			uom = lexer->string_value();
			++lexer;
		}
		nagios_range warn(nagios_range::empty_range);
		nagios_range crit(nagios_range::empty_range);
		double min(-INFINITY);
		double max(INFINITY);
		if (lexer->is_separator())
		{
			++lexer;
			if (lexer->is_range())
			{
				warn = lexer->range_value();
				++lexer;
			}
			if (lexer->is_separator())
			{
				++lexer;
				if (lexer->is_range())
				{
					crit = lexer->range_value();
					++lexer;
				}
				if (lexer->is_separator())
				{
					++lexer;
					if (lexer->is_number())
					{
						min = lexer->number_value();
						++lexer;
					}
					if (lexer->is_separator())
					{
						++lexer;
						if (lexer->is_number())
						{
							max = lexer->number_value();
							++lexer;
						}
					}
				}
			}
		}
		dest.emplace_back(label, value, uom, warn, crit, min, max);
		if (lexer->is_space())
			++lexer;
	}
}

nagios_perfdata::operator json() const
{
	json j;
	map<string, json>& map(j.map_value());
	map["label"].string_value() = _label;
	if (!std::isnan(_value))
		map["value"].number_value() = _value;
	if (!_uom.empty())
		map["uom"].string_value() = _uom;
	if (!_warning.empty())
		map["warning"] = json(_warning);
	if (!_critical.empty())
		map["critical"] = json(_critical);
	if (isfinite(_minimum))
		map["minimum"].number_value() = _minimum;
	if (isfinite(_maximum))
		map["maximum"].number_value() = _maximum;
	return j;
}