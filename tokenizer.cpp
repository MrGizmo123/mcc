#include <string>
#include <regex>
#include <deque>
#include <iostream>

#include "tokenizer.h"

using namespace std;

// taken from stackoverflow https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring
// modified to include newlines as non whitespace charachters, I need them to count the line numbers for each token
inline void ltrim(string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
	return !std::isspace(ch) || (ch == '\n');
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

    regex control("if\\b|else\\b|while\\b|for\\b|break\\b|continue\\b|do\\b");
    regex ident("[a-zA-Z_]\\w*\\b");
    regex type("float\\b|int\\b");
    regex number("[0-9]+\\b");
    regex sym("=|;|\\(|\\)|\\{|\\}|:");
    regex unary("--|~|!|\\+\\+");
    regex binary("\\+|\\*|/|\\%|\\^|\\&\\&|\\&|\\|\\||!=|==|<=|>=|<|>|=");
    regex ternary("\\?");
    regex minus("-");
    regex newline("\\n");
    smatch match;

    int string_pos = 0;
    int line_number = 1;

    while (input.length() > 0)
    {
	token_type t;
	
	if (isFound(input, match, type))
	{
	    t = TYPE;
	}
	else if (isFound(input, match, control))
	{
	    t = CONTROL;
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
	else if (isFound(input, match, ternary))
	{
	    t = TERNARY;
	}
	else if (isFound(input, match, minus))
	{
	    t = UNARY | BINARY;	// minus is part of both
	}
	else if (isFound(input, match, sym))
	{
	    t = SYMBOL;
	}
	else if (isFound(input, match, newline))
	{
	    line_number++;
	    input = input.substr(1, input.length()-1); // remove one newline char
	    ltrim(input);
	    continue;
	}
	else
	{
	    cout << "Unrecognized Token: '" << input.substr(0, 20)  << "'" << endl;
	    break;
	}
		

	result.push_back(token(t, match.str(), string_pos, string_pos + match.length(), line_number));
	input = input.substr(match.length(), input.length() - match.length());
	int orig_length = input.length();
	ltrim(input);		//remove whitespace until next token
	int diff = orig_length - input.length();
	string_pos += diff;
	string_pos += match.length();
    }
    
    return result;
}


