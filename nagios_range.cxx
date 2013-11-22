#include "globals.h"

#include <cmath>
#include <map>
#include <string>

#include "json.h"
#include "lexer.h"
#include "nagios_range.h"
#include "strutil.h"

using namespace std;

namespace
{
	class range_token
	{
	public:
		virtual ~range_token() { }
		virtual bool is_eos() const { return false; }
		virtual bool is_separator() const { return false; }
		virtual bool is_inside() const { return false; }
		virtual bool is_number() const { return false; }
		virtual double number_value() const { throw logic_error("Cannot get number value of non-number token"); }
	};
	class eos_token : public range_token
	{
	public:
		virtual bool is_eos() const { return true; }
	};
	class separator_token : public range_token
	{
	public:
		virtual bool is_separator() const { return true; }
	};
	class inside_token : public range_token
	{
	public:
		virtual bool is_inside() const { return true; }
	};
	class number_token : public range_token
	{
	private:
		double _value;
	
	public:
		explicit number_token(double value) : _value(value) { }
		
		virtual bool is_number() const { return true; }
		virtual double number_value() const { return _value; }
	};
	
	class range_lexer : public lexer<range_token, string::const_iterator>
	{
	protected:
		virtual range_token* next()
		{
			double value;
			if (_pos == _end)
				return new eos_token();
			else if (*_pos == ':')
			{
				++_pos;
				return new separator_token();
			}
			else if (*_pos == '@')
			{
				++_pos;
				return new inside_token();
			}
			else if (*_pos == '~')
			{
				++_pos;
				return new number_token(-INFINITY);
			}
			else if (getnumber(_pos, _end, value))
				return new number_token(value);
			else
				throw parse_error();
		}
	
	public:
		range_lexer(const string::const_iterator& begin, const string::const_iterator& end) : lexer(begin, end) { }
	};
}

const nagios_range nagios_range::empty_range(-INFINITY, INFINITY, false);

// n => nagios_range(0, n, false)
// n: => nagios_range(n, INFINITY, false)
// ~:n => nagios_range(-INFINITY, n, false)
// n:m => nagios_range(n, m, false)
// @n:m => nagios_range(n, m, true)
nagios_range nagios_range::parse(const string::const_iterator& begin, const string::const_iterator& end)
{
	range_lexer lexer(begin, end);
	bool inside = lexer->is_inside();
	if (inside)
		++lexer;
	double first;
	if (lexer->is_number())
	{
		first = lexer->number_value();
		++lexer;
	}
	else
		throw parse_error();
	if (lexer->is_eos())
	{
		if (first < 0)
			throw parse_error();
		return nagios_range(0, first, inside);
	}
	else if (lexer->is_separator())
		++lexer;
	else
		throw parse_error();
	if (lexer->is_eos())
		return nagios_range(first, INFINITY, inside);
	else if (lexer->is_number() && lexer->number_value() >= first)
		return nagios_range(first, lexer->number_value(), inside);
	else
		throw parse_error();
}

nagios_range::operator json() const
{
	json j;
	map<string, json>& map(j.map_value());
	if (isfinite(_minimum))
		map["minimum"].number_value() = _minimum;
	if (isfinite(_maximum))
		map["maximum"].number_value() = _maximum;
	map["inside"].number_value() = _inside ? 1 : 0;
	return j;
}