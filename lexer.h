#ifndef __LEXER_H
#define __LEXER_H

#include <cctype>
#include <deque>
#include <string>
#include <stdexcept>

template<typename Token, typename InputIterator>
class lexer
{
public:
	typedef Token token_type;
	typedef InputIterator iterator_type;
	typedef Token& reference;
	typedef const Token& const_reference;
	typedef Token* pointer;
	typedef const Token* const_pointer;
	typedef typename std::deque<Token*>::size_type count_type;
	typedef typename std::iterator_traits<InputIterator>::difference_type difference_type;

protected:
	virtual pointer next() = 0;

private:
	std::deque<pointer> _queue;
	
	inline void ensure_lookahead()
	{
		if (_queue.empty())
			_queue.push_back(next());
	}
	void ensure_lookahead(count_type n)
	{
		while (_queue.size() <= n)
			_queue.push_back(next());
	}

protected:
	InputIterator _pos;
	const InputIterator _end;
	
	count_type eat_spaces()
	{
		count_type num(0);
		while (_pos != _end && isspace(*_pos))
		{
			++_pos;
			++num;
		}
		return num;
	}

public:
	lexer(const InputIterator& pos, const InputIterator& end) : _queue(), _pos(pos), _end(end) { }
	template<typename Sequence>
	explicit lexer(Sequence& source) : _queue(), _pos(source.begin()), _end(source.end()) { }
	virtual ~lexer()
	{
		count_type n(_queue.size());
		while (n--)
		{
			delete _queue.front();
			_queue.pop_front();
		}
	}
	
	inline lexer& operator ++()
	{
		ensure_lookahead();
		delete _queue.front();
		_queue.pop_front();
		return *this;
	}
	lexer& operator +=(count_type n)
	{
		ensure_lookahead(n);
		while (n--)
		{
			delete _queue.front();
			_queue.pop_front();
		}
		return *this;
	}
	inline const_reference operator *()
	{
		ensure_lookahead();
		return *_queue.front();
	}
	inline const_pointer operator ->()
	{
		ensure_lookahead();
		return _queue.front();
	}
	inline const_reference operator [](count_type n)
	{
		ensure_lookahead(n);
		return _queue[n];
	}
};

class parse_error : public std::runtime_error
{
public:
	parse_error() : std::runtime_error("Parse error") { }
	explicit parse_error(const std::string& message) : std::runtime_error(message) { }
};

#endif