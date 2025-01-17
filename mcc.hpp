#pragma once

#include <string>
#include "tokenizer.h"

class CodeContext
{
public:
    int line_number;
    string context;
};

CodeContext get_code_context(token tok, int number_of_lines=1);
