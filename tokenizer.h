#pragma once

#include <string>
#include <regex>
#include <deque>

using namespace std;

// these types can be bitwise ORred together to indicate that a symbol
// is of both types, e.g. "-" is of type unary and also binary
enum token_type
{
    IDENT  = 0b1,
    TYPE   = 0b1 << 1,
    NUMBER = 0b1 << 2,
    SYMBOL = 0b1 << 3,
    UNARY  = 0b1 << 4,
    BINARY = 0b1 << 5
};

// taken from stackoverflow: https://stackoverflow.com/questions/1448396/how-to-use-enums-as-flags-in-c
inline token_type operator|(token_type a, token_type b)
{
    return static_cast<token_type>(static_cast<int>(a) | static_cast<int>(b));
}

class token
{
public:
    
    token_type type;
    string val;
    token(token_type _type, string _val) : type(_type), val(_val)
    {}
};

deque<token> tokenize(string input);
