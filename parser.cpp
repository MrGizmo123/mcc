
#include "parser.hpp"
#include "mcc.hpp"
#include "tokenizer.h"

#include <deque>
#include <iostream>
#include <string>

#define start_debug() int __start = toks.front().start_index;int __line_number = toks.front().line_number;
#define end_debug() int __end = toks.front().end_index; ASTDebug debug(__line_number,__start,__end);

ostream& operator<<(ostream& out, deque<token>& v);


int counter = 0;		// used to generate unique temp variables
int label_count = 0;		// used to generate unique labels
int variable_count = 0;		// used to generate unique variable names
int loop_count = 0;		// used to generate unique loop labels

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

string uniq_loop_label(string orig_name)
{
    return orig_name + "." + to_string(loop_count++);
}

void copy_ident_map(map<string, identifier>& dest, map<string, identifier> src)
{
    for (auto i=src.begin();i!=src.end();++i) {
	dest[i->first] = identifier(i->second.name, false);
    }
}



inline token pop(deque<token>& tok)
{
    token result = tok.front();
    tok.pop_front();
    return result;
}

inline void fail(token t, string expectation = "")
{
    CodeContext cxt = get_code_context(t);
    string location = file_name() + ":" + to_string(t.line_number) + ":" + to_string(t.start_index) + ": ";
    
    cout << "Syntax Error, expected " << expectation << endl
    << location << cxt.context << endl
    << string(t.start_index + location.length(), ' ') << "^" << string(t.end_index - t.start_index - 1, '~') << endl;
    
    
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


BlockItem* parse_block_item(deque<token>& tok);
Expression* parse_expression(deque<token>& tok,int min_precedence = 0);
Expression* parse_factor(deque<token>& tok);
Statement* parse_statement(deque<token>& toks);
Block* parse_block(deque<token>& toks);
For* parse_for_loop(deque<token>& toks);
While* parse_while_loop(deque<token>& toks);
DoWhile* parse_do_while_loop(deque<token>& toks);
VariableDeclaration* parse_variable_declaration(deque<token>& toks);
FunctionDeclaration* parse_function_declaration(deque<token>& toks, bool allow_definition = false); 
Declaration* parse_declaration(deque<token>& toks, bool allow_function_definion = false);

Declaration* parse_declaration(deque<token>& toks, bool allow_function_definiton)
{
    if (toks[2].val == "(") 	// its a function declaration
    {
	return parse_function_declaration(toks, allow_function_definiton);
    }

    return parse_variable_declaration(toks);
}

DoWhile* parse_do_while_loop(deque<token>& toks)
{
    start_debug();

    check_val(toks, "do", "do keyword");

    Statement* body = parse_statement(toks);

    check_val(toks, "while", "while keyword");

    check_val(toks, "(", "open parenthesis");

    Expression* condition = parse_expression(toks);

    check_val(toks, ")", "closing parenthesis");
    check_val(toks, ";", "semicolon");

    end_debug();

    return new DoWhile(condition, body, debug);
}

While* parse_while_loop(deque<token>& toks)
{
    start_debug();

    check_val(toks, "while", "while keyword");

    check_val(toks, "(", "open parenthesis");

    Expression* condition = parse_expression(toks);

    check_val(toks, ")", "closing parenthesis");

    Statement* body = parse_statement(toks);

    end_debug();

    return new While(condition, body, debug);
}

For* parse_for_loop(deque<token>& toks)
{
    start_debug();
    
    check_val(toks, "for");

    check_val(toks, "(", "open parenthesis");	// check if there is an opening ( next

    BlockItem* initializer;
    token next = toks.front();
    if (next.type & TYPE)	// if its a type then first statement is a declaration
    {
	initializer = parse_variable_declaration(toks);
    }
    else if (next.val == ";")	// empty
    {
	initializer = nullptr;
	toks.pop_front();	// remove the semicolon
    }
    else			// else its an expression
    {
	initializer = parse_expression(toks);
	check_val(toks, ";", "semicolon");
    }

    Expression* condition;
    next = toks.front();
    if (next.val == ";")	// empty condition
    {
	condition = nullptr;
	toks.pop_front();
    }
    else			// else its an expression
    {
	condition = parse_expression(toks);
	check_val(toks, ";", "semicolon"); // needs to end with a semicolon
    }

    Expression* post;
    next = toks.front();
    if (next.val == ")")	// empty condition
    {
	post = nullptr;
	toks.pop_front();
    }
    else			// else its an expression
    {
	post = parse_expression(toks);
	check_val(toks, ")", "closing parenthesis"); // needs to end with )
    }

    Statement* body = parse_statement(toks);

    end_debug();

    return new For(initializer, condition, post, body, debug);
}

Block* parse_block(deque<token>& toks)
{
    start_debug();
    
    token open_brace = check_val(toks, "{", "open brace");
    
    vector<BlockItem*> body;
    
    token next = toks.front();
    while (next.val != "}")
    {
	if (next.val == ";")	// skip empty statements
	{
	    toks.pop_front();
	    next = toks.front();
	    continue;	    
	}

	
	BlockItem* s = parse_block_item(toks);
	body.push_back(s);
	next = toks.front();
    }

    end_debug()
    toks.pop_front();  // remove trailing }
    
    return new Block(body, debug);
}

FunctionParam* parse_function_param(deque<token>& toks)
{
    string first_tok = toks.front().val;
    
    if (first_tok == "{" || first_tok == ";")
    {
	return nullptr;
    }

    token type = check_type(toks, TYPE, "parameter type");

    if (toks.front().val == "," || toks.front().val == ")")
    {
	toks.pop_front();
	return new FunctionParam(type.val, "");
    }

    token param_name = check_type(toks, IDENT, "identifier");

    if (toks.front().val != "," && toks.front().val != ")")
    {
	fail(toks.front(), "comma or closing paren");
    }

    toks.pop_front();
    return new FunctionParam(type.val, param_name.val);
    
}

FunctionDeclaration* parse_function_declaration(deque<token>& toks, bool allow_definition)
{
    start_debug();
    
    token return_type = check_type(toks, TYPE, "return type");

    token func_name = check_type(toks, IDENT, "function name");

    token open_paren = check_val(toks, "(", "open paren");

    
    vector<FunctionParam*> params;
    if (toks.front().val == ")") // empty parameter list
    {
	toks.pop_front();	// pop the closing brace
    }
    else
    {
	FunctionParam* p = parse_function_param(toks);
	while (p)
	{
	    params.push_back(p);
	    p = parse_function_param(toks);
	}
    }

    end_debug();
    
    Block* body_block = nullptr;
    if (toks.front().val == "{" && allow_definition)
    {
	body_block = parse_block(toks);
    }
    else
    {
	check_val(toks, ";", "semicolon");
    }

    FunctionDeclaration* result = new FunctionDeclaration(func_name.val, return_type.val, params, body_block, debug);
    return result;
}

Condition* parse_if_condition(deque<token>& toks)
{
    start_debug();
    
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

    end_debug();

    return new Condition(cond, then, otherwise, debug);
}


Statement* parse_statement(deque<token>& toks)
{
    start_debug();
    
    token next = toks.front();
    Statement* result;
    if (next.val == "return")
    {
	check_val(toks, "return", "return keyword");
	Expression* return_val = parse_expression(toks);
	check_val(toks, ";", "semicolon"); // remove semicolon from queue

	end_debug();
	
	return new Return(return_val, debug);
    }
    else if (next.val == "break")
    {
	toks.pop_front();
	check_val(toks, ";", "semicolon");
	
	end_debug();
	result = new Break(debug);
    }
    else if (next.val == "continue")
    {
	toks.pop_front();
	check_val(toks, ";", "semicolon");
	
	end_debug();
	result = new Continue(debug);
    }
    else if (next.val == "if")
    {
	result = parse_if_condition(toks); // if condition doesnt require semicolon at the end
    }
    else if (next.val == "{")
    {
	Block* body_block = parse_block(toks); // compound statemnt

	end_debug();
	
	result = new Compound(body_block, debug);     // does not require semicolon at the end
    }
    else if (next.val == "for")
    {
	result = parse_for_loop(toks);
    }
    else if (next.val == "while")
    {
	result = parse_while_loop(toks);
    }
    else if (next.val == "do")
    {
	result = parse_do_while_loop(toks);
    }
    else if (next.val == ";")	// null statement
    {
	toks.pop_front();	// remove semicolon
	return new Statement(NULL_AST, ASTDebug()); // empty statement that does nothing
    }
    else
    {
	result = parse_expression(toks);
	check_val(toks, ";", "semicolon"); // remove semicolon from queue
    }
    
    return result;
}

VariableDeclaration* parse_variable_declaration(deque<token>& toks)
{
    start_debug();
    
    token type_specifier = check_type(toks, TYPE, "type specifier");

    token var_name = check_type(toks, IDENT);

    Expression* init_value = 0;
    
    token next = toks.front();
    if (next.val == "=")	// this means initializer exists
    {
	toks.pop_front();

	init_value = parse_expression(toks);
    }

    end_debug();
    
    check_val(toks, ";", "semicolon"); // remove semicolon from queue

    return new VariableDeclaration(var_name.val, init_value, debug);
}


BlockItem* parse_block_item(deque<token>& toks)
{
    token next = toks.front();
    BlockItem* result;
    if (next.type & TYPE)
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
    start_debug();
    
    Expression* left = parse_factor(toks);
    token next = toks.front();
    
    while (next.type & (BINARY | TERNARY | UNARY) && precedence[next.val] >= min_precedence)
    {
	toks.pop_front();
	
	if (next.val == "=")	// check if its assignment, we want right associative
	{
	    Expression* right = parse_expression(toks, precedence[next.val]);
	    end_debug();
	    left = new Assignment(left, right, debug);
	}
	else if (next.val == "++" || next.val == "--") // postfix operators ++ and --
	{
	    end_debug();
	    left = new PostfixUnary(next.val, left, debug);
	}
	else if (next.val == "?")
	{
	    Expression* true_val = parse_expression(toks);

	    check_val(toks, ":", ": for ternary operator");
	    
	    Expression* false_val = parse_expression(toks, precedence[next.val]);

	    end_debug();
	    
	    left = new TernaryConditional(left, true_val, false_val, debug);
	}
	else			// else we want left associative
	{
	    string op = next.val;
	    Expression* right = parse_expression(toks, precedence[op] + 1);

	    end_debug();
	    
	    left = new Binary(op, left, right, debug);
	}
	
	next = toks.front();
    }

    return left;
}

Expression* parse_factor(deque<token>& toks)
{
    start_debug();
    
    token tok = pop(toks);

    if (tok.type & NUMBER)
    {
	end_debug();
	return new Constant(stoi(tok.val), debug);
    }
    else if (tok.type & UNARY)
    {
	end_debug();
	Expression* inner = parse_expression(toks);
	return new Unary(tok.val, inner, debug);
    }
    else if (tok.val == "(")
    {
	Expression* inner = parse_expression(toks);
	token close_paren = check_val(toks, ")", "correctly parenthesised expression");

	return inner;
    }
    else if (tok.type & IDENT)
    {
	if (toks.front().val == "(") // function call
	{
	    toks.pop_front();
	    vector<Expression*> args;

	    if (toks.front().val == ")") // function with now arguments
	    {
		toks.pop_front();
		end_debug();
		return new FunctionCall(tok.val, args, debug);
	    }
	    
	    while (true)
	    {
		Expression* arg = parse_expression(toks);

		if (toks.front().val == ")")
		{
		    toks.pop_front();
		    args.push_back(arg);
		    break;
		}
		
		check_val(toks, ",", "comma between arguments");
		args.push_back(arg);
	    }
	    
	    end_debug();
	    return new FunctionCall(tok.val, args, debug);
	}

	// else its a variable
	end_debug();
	return new Variable(tok.val, debug);
    }

    end_debug();
    return new Expression(CONST, debug);
}


Program* parse(deque<token>& toks)
{
    start_debug();

    vector<Declaration*> functions;
    while(toks.size() > 0)
    {
	Declaration* curr_decl = parse_declaration(toks, true);
	functions.push_back(curr_decl);
    }
    
    end_debug();
    
    Program* p = new Program(functions, debug);
    return p;
}
