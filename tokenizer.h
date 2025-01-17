#pragma once

#include <string>
#include <regex>
#include <deque>

using namespace std;

// these types can be bitwise ORred together to indicate that a symbol
// is of both types, e.g. "-" is of type unary and also binary
enum token_type
{
    IDENT	= 0b1,
    TYPE	= 0b1 << 1,
    NUMBER	= 0b1 << 2,
    SYMBOL	= 0b1 << 3,
    UNARY	= 0b1 << 4,
    BINARY	= 0b1 << 5,
    CONTROL	= 0b1 << 6,
    TERNARY	= 0b1 << 7,
};

// taken from stackoverflow: https://stackoverflow.com/questions/1448396/how-to-use-enums-as-flags-in-c
inline token_type operator|(token_type a, token_type b)
{
    return static_cast<token_type>(static_cast<int>(a) | static_cast<int>(b));
}

inline token_type operator&(token_type a, token_type b)
{
    return static_cast<token_type>(static_cast<int>(a) & static_cast<int>(b));
}

class token
{
public:
    
    token_type type;
    string val;
    int start_index;
    int end_index;
    int line_number;
    
    token(
	  token_type _type,
	  string _val,
	  int _start_index,
	  int _end_index,
	  int _line_number
	  )
    :
	type(_type),
	val(_val),
	start_index(_start_index),
	end_index(_end_index),
	line_number(_line_number)
    {}
};

deque<token> tokenize(string input);
