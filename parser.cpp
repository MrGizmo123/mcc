
#include "parser.hpp"
#include "tokenizer.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <string>

ostream& operator<<(ostream& out, deque<token>& v);


int counter = 0;		// used to generate unique temp variables
int label_count = 0;		// used to generate unique labels
int variable_count = 0;

map<string, int> precedence = {
    {"+"	, 45},
    {"-"	, 45},
    {"%"	, 50},
    {"/"	, 50},
    {"*"	, 50},
    {"<"	, 35},
    {"<="	, 35},
    {">"	, 35},
    {">="	, 35},
    {"=="	, 30},
    {"!="	, 30},
    {"&&"	, 10},
    {"||"	,  5},
    {"?"	,  3},
    {"="	,  1},
};

IRVar* temp_var()
{
    string result = "tmp" + to_string(counter);
    counter++;
    return new IRVar(result);
}

string uniq_label()
{
    string result = "label" + to_string(label_count);
    label_count++;
    return result;
}

string uniq_var_name(string orig_name)
{
    return orig_name + "." + to_string(variable_count++);
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
BlockItem* parse_block_item(deque<token>& tok);
Expression* parse_expression(deque<token>& tok,int min_precedence = 0);
Expression* parse_factor(deque<token>& tok);
Statement* parse_statement(deque<token>& toks);

Function* parse_function(deque<token>& tok)
{
    token return_type = check_type(tok, TYPE, "return type");

    token func_name = check_type(tok, IDENT, "function name");

    token open_paren = check_val(tok, "(", "open paren");

    token close_paren = check_val(tok, ")", "close paren");

    token open_brace = check_val(tok, "{", "open brace");

    deque<BlockItem*> body;
    
    token next = tok.front();
    while (next.val != "}")
    {
	if (next.val == ";")	// skip empty statements
	{
	    tok.pop_front();
	    next = tok.front();
	    continue;	    
	}

	
	BlockItem* s = parse_block_item(tok);
	body.push_back(s);
	next = tok.front();
    }

    tok.pop_front();  // remove trailing

    Function* result = new Function(func_name.val, return_type.val, body);
    return result;
}

Condition* parse_if_condition(deque<token>& toks)
{
    check_val(toks, "if", "if keyword");

    check_val(toks, "(", "( before condition");
    Expression* cond = parse_expression(toks);
    check_val(toks, ")", ") after condition");
    
    Statement* then = parse_statement(toks);
    
    Statement* otherwise = 0;
    token next = toks.front();
    if (next.val == "else")
    {
	toks.pop_front();
	otherwise = parse_statement(toks);
    }

    return new Condition(cond, then, otherwise);
}


Statement* parse_statement(deque<token>& toks)
{
    token next = toks.front();
    Statement* result;
    if (next.val == "return")
    {
	check_val(toks, "return", "return keyword");
	Expression* return_val = parse_expression(toks);
	check_val(toks, ";", "semicolon"); // remove semicolon from queue
	
	return new Return(return_val);
    }
    else if (next.val == "if")
    {
	result = parse_if_condition(toks); // if condition doesnt require semicolon at the end
    }
    else
    {
	result = parse_expression(toks);
	check_val(toks, ";", "semicolon"); // remove semicolon from queue
    }

    
    
    return result;
}

Declaration* parse_declaration(deque<token>& toks)
{
    token type_specifier = check_val(toks, "int", "type specifier");

    token var_name = check_type(toks, IDENT);

    Expression* init_value = 0;
    
    token next = toks.front();
    if (next.val == "=")	// this means initializer exists
    {
	toks.pop_front();

	init_value = parse_expression(toks);
    }

    check_val(toks, ";", "semicolon"); // remove semicolon from queue

    return new Declaration(var_name.val, init_value);
}


BlockItem* parse_block_item(deque<token>& toks)
{
    token next = toks.front();
    BlockItem* result;
    if (next.val == "int")
    {
	result = parse_declaration(toks);
    }
    else
    {
	result = parse_statement(toks);
    }
    return result;
    
}

Expression* parse_expression(deque<token>& toks, int min_precedence)
{
    Expression* left = parse_factor(toks);
    token next = toks.front();
    
    while (next.type & (BINARY | TERNARY | UNARY) && precedence[next.val] >= min_precedence)
    {
	toks.pop_front();
	
	if (next.val == "=")	// check if its assignment, we want right associative
	{
	    Expression* right = parse_expression(toks, precedence[next.val]);
	    left = new Assignment(left, right);
	}
	else if (next.type & UNARY) // postfix operators ++ and --
	{
	    left = new PostfixUnary(next.val, left);
	}
	else if (next.val == "?")
	{
	    Expression* true_val = parse_expression(toks);

	    check_val(toks, ":", ": for ternary operator");
	    
	    Expression* false_val = parse_expression(toks, precedence[next.val]);

	    left = new TernaryConditional(left, true_val, false_val);
	}
	else			// else we want left associative
	{
	    string op = next.val;
	    Expression* right = parse_expression(toks, precedence[op] + 1);
	    left = new Binary(op, left, right);
	}
	
	next = toks.front();
    }

    return left;
}

Expression* parse_factor(deque<token>& toks)
{
    token tok = pop(toks);

    if (tok.type & NUMBER)
    {
	return new Constant(stoi(tok.val));
    }
    else if (tok.type & UNARY)
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
    else if (tok.type & IDENT)
    {
	return new Variable(tok.val);
    }

    return new Expression(CONST);
}


Program* parse(deque<token>& tok)
{
    Function* main = parse_function(tok);

    Program* p = new Program(main);
    return p;
}
