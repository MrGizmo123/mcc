
#include "parser.hpp"
#include "tokenizer.h"

#include <deque>
#include <iostream>

ostream& operator<<(ostream& out, deque<token>& v);


int counter = 0;

IRVar* temp_name()
{
    string result = "tmp" + to_string(counter);
    counter++;
    return new IRVar(result);
}

inline token pop(deque<token>& tok)
{
    token result = tok.front();
    tok.pop_front();
    return result;
}

inline void fail(token t, string expectation = "")
{
    cout << "Parsing Failed at token->"
	 << t.val
	 << ", expected "
	 << expectation
	 << endl;
    
    exit(-1);
}

// pop a token and check its type
inline token check_type(deque<token>& toks, token_type type, string expectation = "")
{
    token tok = pop(toks);
    if (!(tok.type & type))
    {
	fail(tok, expectation);
    }
    return tok;
}

// pop a token and check its value
inline token check_val(deque<token>& toks, string val, string expectation = "")
{
    token tok = pop(toks);
    if (tok.val != val)
    {
	fail(tok, expectation);
    }
    return tok;
}


Function* parse_function(deque<token>& tok);
Statement* parse_statement(deque<token>& tok);
Expression* parse_expression(deque<token>& tok,int min_precedence = 0);
Factor* parse_factor(deque<token>& tok);

Function* parse_function(deque<token>& tok)
{
    token return_type = check_type(tok, TYPE, "return type");

    token func_name = check_type(tok, IDENT, "function name");

    token open_paren = check_val(tok, "(", "open paren");

    token close_paren = check_val(tok, ")", "close paren");

    token open_brace = check_val(tok, "{", "open brace");

    deque<Statement*> body;
    
    token next = tok.front();
    while (next.val != "}")
    {
	Statement* s = parse_statement(tok);
	body.push_back(s);
	next = tok.front();
    }

    tok.pop_front();  // remove trailing

    Function* result = new Function(func_name.val, return_type.val, body);
    return result;
}

Statement* parse_statement(deque<token>& tok)
{
    token ret_statement = check_val(tok, "return", "return keyword");

    Expression* ret_value = parse_expression(tok);

    token semicolon = check_val(tok, ";", "semicolon");

    Return* r = new Return(ret_value);
    return r;
}

Expression* parse_expression(deque<token>& toks, int min_precedence)
{
    Expression* left = parse_factor(toks);
    token next = toks.front();
    while (next.type == BINARY_OP)
    {
	
    }
}

Expression* parse_factor(deque<token>& toks, int min_precedence)
{
    token tok = pop(toks);

    if (tok.type == NUMBER)
    {
	return new Constant(stoi(tok.val));
    }
    else if (tok.type == UNARY)
    {
	Expression* inner = parse_expression(toks);
	return new Unary(tok.val, inner);
    }
    else if (tok.val == "(")
    {
	Expression* inner = parse_expression(toks);
	token close_paren = check_val(toks, ")", "), malformed expression");

	return inner;
    }

    return new Expression(CONST);
}


Program* parse(deque<token>& tok)
{
    Function* main = parse_function(tok);

    Program* p = new Program(main);
    return p;
}
