#pragma once

#include <string>
#include "tokenizer.h"

class CodeContext
{
public:
    int line_number;
    string context;
    CodeContext(int _l,
		string _cxt)
	    :
		line_number(_l),
		context(_cxt)
    {}
};

CodeContext get_code_context(token tok, int number_of_lines=1);
string get_line_of_code(int);
string file_name();

string read_file(string filename);
