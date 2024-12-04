#include <string>
#include <regex>
#include <deque>
#include <iostream>

#include "tokenizer.h"

using namespace std;

// taken from stackoverflow https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring
inline void ltrim(string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
	return !std::isspace(ch);
    }));
}

bool isFound(string& input, smatch& match, regex& re)
{
    regex_search(input, match, re);
	
    if (match.position() != 0)	// if match position is not 0
    {
	return false;
    }
    return true;
}

deque<token> tokenize(string input)
{
    ltrim(input);		// remove whitespace from beginning
    
    deque<token> result;

    regex ident("[a-zA-Z_]\\w*\\b");
    regex type("float\\b|int\\b");
    regex number("[0-9]+\\b");
    regex sym("=|;|\\(|\\)|\\{|\\}");
    regex unary("--|~");
    regex binary("+|*|/");
    regex minus("-");
    smatch match;

    while (input.length() > 0)
    {
	token_type t;
	
	if (isFound(input, match, type))
	{
	    t = TYPE;
	}
	else if (isFound(input, match, ident))
	{
	    t = IDENT;
	}
	else if (isFound(input, match, number))
	{
	    t = NUMBER;
	}
	else if (isFound(input, match, unary))
	{
	    t = UNARY;
	}
	else if (isFound(input, match, binary))
	{
	    t = BINARY;
	}
	else if (isFound(input, match, minus))
	{
	    t = UNARY | BINARY;	// minus is part of both
	}
	else if (isFound(input, match, sym))
	{
	    t = SYMBOL;
	}
	else
	{
	    cout << "Unrecognized Token: " << input.substr(0, 20) << endl;
	    break;
	}
		

	result.push_back(token(t, match.str()));
	input = input.substr(match.length(), input.length() - match.length());
	ltrim(input);		//remove whitespace until next token
    }
    
    return result;
}


