#pragma once

#include <deque>
#include <ostream>
#include <string>
#include <sstream>


#include <vector>

#include "asm.hpp"
#include "tokenizer.h"
#include "tacky.hpp"
#include "mcc.hpp"

#define indent() out<<string(indentation,'\t');

using namespace std;

IRVar* temp_var();
string uniq_label();
string uniq_var_name(string);
string uniq_loop_label(string orig);

enum ASTType{
    FUNC,
    FUNCTION_CALL,
    FUNCTION_PARAM,
    PROG,
    CONST,
    VAR,
    RET,
    UNARY_AST,
    BINARY_AST,
    TERNARY_AST,
    ASSIGNMENT,
    VARIABLE_DECLARATION,
    FUNCTION_DECLARATION,
    CONDITIONAL,
    BLOCK,
    COMPOUND,
    FOR,
    WHILE,
    DO_WHILE,
    BREAK,
    CONTINUE,
    NULL_AST
};

// enum Type {
//     INT,
//     FUNCTION
// };

class Type
{
public:
    virtual bool equals(Type* other)
    {	
	return (this->get_typename() == other->get_typename());
    }
    
    virtual string get_typename() { return "type"; }
};

class PrimitiveType : public Type
{
public:
    string name;

    PrimitiveType(string _name)
    :
	name(_name)
    {}

    virtual string get_typename() { return "primitive " + name; }
};

class FunctionType : public Type
{
public:
    Type* return_type;
    vector<Type*> args;

    FunctionType(Type* _return_type, vector<Type*> _args)
    :
	return_type(_return_type),
	args(_args)
    {}

    virtual bool equals(Type* other)
    {
	if (!Type::equals(other)) // if other is not a function
	    return false;

	FunctionType* func = (FunctionType*) other;

	if (args.size() != func->args.size()) // if they dont have same number of arguments
	    return false;
	
	for (int i=0;i<args.size();i++) { // if the type of respective args does not match
	    if (!(args[i]->equals(func->args[i])))
		return false;
	}

	return true;
    }

    virtual string get_typename() { return "function"; }
};

class identifier
{
public:
    string name;
    bool declared_in_this_scope;
    bool external_linkage;

    identifier(string _name = "", bool _decl = true, bool _extern = false) : name(_name), declared_in_this_scope(_decl), external_linkage(_extern) {}
};

class symbol
{
public:
    string name;
    Type* type;
    bool defined;

    symbol(string _name="", Type* _type=new PrimitiveType("int"), bool _defined=false)
    :
	name(_name),
	type(_type),
	defined(_defined)
    {}
};

void copy_ident_map(map<string, identifier>& dest, map<string, identifier> src);

// the position of the text code which made an AST Node
class ASTDebug
{
public:
    int line_number;
    int start;
    int end;
    ASTDebug(int _line=0,
	     int _start=0,
	     int _end=0)
    :
	line_number(_line),
	start(_start),
	end(_end)
    {}

    ASTDebug(const ASTDebug& src)
    {
	line_number = src.line_number;
	start = src.start;
	end = src.end;
    }
};

inline void fail(string message, ASTDebug debug)
{
    string line = get_line_of_code(debug.line_number);
    string location = file_name() + ":" + to_string(debug.line_number) + ":" + to_string(debug.start) + ": ";
    
    cout
    << "Semantic Analysis failed. " << message << endl
    << location << line << endl
    << string(location.length() + debug.start, ' ') << "^" << string(debug.end - debug.start-1, '~') << endl;
    exit(-1);
}

// abstract syntax tree (actually just a node in the AST)
class AST
{
public:
    ASTType type;
    ASTDebug debug_info;

    AST(ASTType _type, ASTDebug _debug_info=ASTDebug())
    :
	type(_type),
	debug_info(_debug_info)
    {}
    
    virtual IRNode* emit(vector<IRNode*>& result) {
	result.push_back(new IRNode());
	return new IRNode();
    }

    virtual void resolve_identifiers(map<string, identifier>& ident_map) {}

    virtual void label_loops(string curr_loop_label) {}

    virtual Type* do_type_checking(map<string, symbol>& symbol_table) { return nullptr; }
    
    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	out << "Empty AST" << endl;
	return out;
    } 
};

class BlockItem : public AST
{
public:
    BlockItem(ASTType _type, ASTDebug debug)
    : AST(_type, debug)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	result.push_back(new IRStatement());
	return new IRNode();
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	out << "Block Item" << endl;
	return out;
    } 
};

class Block : public AST
{
public:
    vector<BlockItem*> items;
    
    Block(vector<BlockItem*> _items, ASTDebug debug)
    :
	AST(BLOCK, debug),
	items(_items)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	for (BlockItem* s : items) {
	    s->emit(result);
	}

	return new IRNode();
    }

    virtual void label_loops(string curr_loop_label)
    {
	for (BlockItem* b : items) {
	    b->label_loops(curr_loop_label);
	}
    }

    virtual void resolve_identifiers(map<string, identifier>& ident_map, bool new_scope = true)
    {
	map<string, identifier> ident_map_copy;

	if (new_scope)		// sometimes, like in a function definition, we dont want to make a new scope here
	    copy_ident_map(ident_map_copy, ident_map);
	else
	    ident_map_copy = ident_map;
	
	for (BlockItem* b : items) {
	    b->resolve_identifiers(ident_map_copy);
	}
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	for (BlockItem* b : items) {
	    b->do_type_checking(symbol_table);
	}

	return nullptr;
    }
    
    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	for(BlockItem* b : items) {
	    b->pretty_print(out, indentation);
	    out << endl;
	}
	
	return out;
    } 
};

class Statement : public BlockItem
{
public:
    Statement(ASTType _type, ASTDebug debug)
    : BlockItem(_type, debug)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	result.push_back(new IRStatement());
	return new IRNode();
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "Generic Statement" << endl;
	return out;
    } 
};

class Expression : public Statement
{
public:
    Expression(ASTType _type, ASTDebug debug)
    :
	Statement(_type, debug)
    {}

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	result.push_back(new IRExpr());
	return new IROperand();
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	out << " Empty Expression ";
	return out;
    } 
};

class Declaration : public BlockItem
{
public:
    Declaration(ASTType _type, ASTDebug _debug)
    :
	BlockItem(_type, _debug)
    {}
};

class VariableDeclaration : public Declaration
{
    string name;		// identifier
    Expression* val;

public:
    VariableDeclaration(string _name, Expression* _val, ASTDebug debug)
    :
	Declaration(VARIABLE_DECLARATION, debug),
	name(_name),
	val(_val)
    {}

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	if (ident_map.count(name) > 0)
	{
	    if (ident_map[name].declared_in_this_scope)
	    {
		fail("Redeclaration of variable " + name, debug_info);
	    }
	}

	string unique_name = uniq_var_name(name);

	ident_map[name] = identifier(unique_name);

	if (val)		// check if initializer value exists
	{
	    val->resolve_identifiers(ident_map);
	}

	name = unique_name;
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	symbol_table[name] = symbol(name, new PrimitiveType("int"));

	if (val)
	    val->do_type_checking(symbol_table);

	return nullptr;
    }

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	if (val)
	{
	    IRVar* dest_var = new IRVar(name);
	    IROperand* value = val->emit(result);
	    IRLoad* result_op = new IRLoad(dest_var, value);
	    result.push_back(result_op);
	    return result_op;
	}

	return new IRNode();
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "DECLARE " << name;
	if (val)
	{
	    out << " = ";
	    val->pretty_print(out, 0);
	}
	return out;
    }
};

class FunctionParam : public AST
{
public:
    string type;
    string name;

    FunctionParam(string _t, string _name)
    :
	AST(FUNCTION_PARAM),
	type(_t),
	name(_name)
    {}

    void resolve_identifiers(map<string, identifier>& ident_map)
    {
	if (name == "")		// unnamed variable
	    return;
	
	if (ident_map.count(name) > 0)
	{
	    if (ident_map[name].declared_in_this_scope)
	    {
		fail("Redeclaration of variable " + name, ASTDebug());
	    }
	}

	string unique_name = uniq_var_name(name);

	ident_map[name] = identifier(unique_name);

	name = unique_name;
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	symbol_table[name] = symbol(name, new PrimitiveType("int"));
	return symbol_table[name].type;
    }
};

class FunctionDeclaration : public Declaration
{
public:
    string name;		// identifier
    string return_type;
    vector<FunctionParam*> params;
    Block* body;


    FunctionDeclaration(string _name, string _return_type, vector<FunctionParam*>& _params, Block* _body, ASTDebug debug)
    :
	Declaration(FUNCTION_DECLARATION, debug),
	name(_name),
	return_type(_return_type),
	params(_params),
	body(_body)
    {}

    virtual void label_loops(string curr_loop_name)
    {
	if (body)
	    body->label_loops(curr_loop_name);
    }

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	if (ident_map.count(name) > 0)
	{
	    identifier prev_entry = ident_map[name];
	    if (prev_entry.declared_in_this_scope && !prev_entry.external_linkage)
	    {
		fail("function and variable cannot have same name", debug_info);
	    }
	}

	ident_map[name] = identifier(name, true, true);
	
	map<string, identifier> ident_map_copy;
	copy_ident_map(ident_map_copy, ident_map);
	for (FunctionParam* param : params) {
	    param->resolve_identifiers(ident_map_copy);
	}

	if (body)
	    body->resolve_identifiers(ident_map_copy, false);
    }

    FunctionType* construct_type()
    {
	PrimitiveType* return_type = new PrimitiveType("int");

	vector<Type*> param_types;
	for (FunctionParam* p : params) {
	    param_types.push_back(new PrimitiveType("int"));
	}

	return new FunctionType(return_type, param_types);
    }
    
    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	FunctionType* self_type = construct_type();
	bool has_body = body ? true : false;
	bool already_defined = false;

	if (symbol_table.count(name) > 0)
	{
	    symbol old_decl = symbol_table[name];
	    
	    already_defined = old_decl.defined;
	    if (already_defined && has_body)
	    {
		fail("cannot redefine function", debug_info);
	    }

	    if (!old_decl.type->equals(self_type))
	    {
		fail("Incompatible function redeclaration", debug_info);
	    }
	}

	symbol_table[name] = symbol(name, self_type, already_defined || has_body);

	if (body)
	{
	    for (FunctionParam* param : params) {
		param->do_type_checking(symbol_table);
	    }
	    body->do_type_checking(symbol_table);
	}
	
	return self_type;
    }

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	if(!body)
	    return nullptr;
	
	result.push_back(new IRLabel(name));

	vector<string> param_list;
	for (FunctionParam* p : params) {
	    param_list.push_back(p->name);
	}
	
	vector<IRNode*> ir_body;
	body->emit(ir_body);
	
	return new IRFunction(name, param_list, ir_body);
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "INT FUNCTION " << name << ":" << endl;
	++indentation;
	for (const FunctionParam* param : params) {
	    indent();
	    out << "INT " << param->name << endl;
	}

	if (body)
	    body->pretty_print(out, indentation+1);

	return out;
    }
};

class Program : public AST
{
public:

    vector<Declaration*> declarations;
    
    Program(vector<Declaration*>& _functions, ASTDebug debug)
    :
	AST(PROG, debug),
	declarations(_functions)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {

	vector<IRFunction*> functions;

	for (Declaration* decl : declarations) {
	    
	    IRNode* return_val = decl->emit(result);

	    if (return_val && decl->type == FUNCTION_DECLARATION)
	    {
		IRFunction* func_ptr = (IRFunction*)return_val;
		functions.push_back(func_ptr);
	    }
	}

	return new IRProgram(functions);
    }

    virtual void label_loops(string curr_loop_label)
    {
	for (Declaration* decl : declarations) {
	    decl->label_loops(curr_loop_label);
	}
    }

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	for (Declaration* decl : declarations) {
	    decl->resolve_identifiers(ident_map);
	}
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	for (Declaration* decl : declarations) {
	    decl->do_type_checking(symbol_table);
	}
	return nullptr;
    }
    
    virtual ostream& pretty_print(ostream& out, int indentation=0)
    {

	for (Declaration* decl : declarations) {
	    decl->pretty_print(out, indentation);
	    out << endl;
	}
	
	return out;
    } 
};

class FunctionCall : public Expression
{
public:
    string name;
    vector<Expression*> args;

    FunctionCall(string _name, vector<Expression*>& _args, ASTDebug _debug)
    :
	Expression(FUNCTION_CALL, _debug),
	name(_name),
	args(_args)
    {}

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	if (ident_map.count(name) == 0)
	{
	    fail("Undeclared Function", debug_info);
	}

	name = ident_map[name].name;

	for(Expression* arg : args) {
	    arg->resolve_identifiers(ident_map);
	}
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	FunctionType* func_type = (FunctionType*)symbol_table[name].type;

	if (func_type->get_typename() != "function")
	    fail("cannot call variable like a function", debug_info);

	if (func_type->args.size() != args.size())
	    fail("wrong number of arguments", debug_info);

	for (int i=0;i<args.size();i++)
	{
	    Type* arg_type = args[i]->do_type_checking(symbol_table);
	    if (!(arg_type->equals(func_type->args[i])))
		fail("Wrong argument type at argument number " + to_string(i+1), debug_info);
	}
	
	return func_type;
    }

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	vector<IROperand*> ir_args;
	for(Expression* e : args) {
	    ir_args.push_back(e->emit(result));
	}

	IRVar* dest = temp_var();

	result.push_back(new IRFunctionCall(name, dest, ir_args));
	
	return dest;
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "FUNCALL " << name << "(";

	if (args.size() == 0)
	{
	    out << ")";
	    return out;
	}
	
	for (Expression* expr : args) {
	    expr->pretty_print(out, 0);
	    out << ", ";
	}
	out << "\b\b)";
	return out;
    } 
    
};

class Binary : public Expression
{
public:
    Expression* first;
    Expression* second;
    string op;

    Binary(string _op, Expression* _f, Expression* _s, ASTDebug debug)
    : Expression(BINARY_AST, debug),
    op(_op),
    first(_f),
    second(_s)
    {}

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	first->resolve_identifiers(ident_map);
	second->resolve_identifiers(ident_map);
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	first->do_type_checking(symbol_table);
	second->do_type_checking(symbol_table);
	return new PrimitiveType("int");
    }

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* src1 = first->emit(result);
	IROperand* src2 = second->emit(result);
	IRVar* dest = temp_var();

	if(op == "+")
	{
	    result.push_back(new IRAdd(dest, src1, src2));
	}
	else if (op == "-")
	{
	    result.push_back(new IRSub(dest, src1, src2));
	}
	else if (op == "*")
	{
	    result.push_back(new IRMul(dest, src1, src2));
	}
	else if (op == "/")
	{
	    result.push_back(new IRDiv(dest, src1, src2));
	}
	else if (op == "%")
	{
	    result.push_back(new IRMod(dest, src1, src2));
	}
	else if (op == "&")
	{
	    result.push_back(new IRBitAnd(dest, src1, src2));
	}
	else if (op == "&&")
	{
	    string fail_label = uniq_label();
	    string end_label = uniq_label();
	    result.push_back(new IRJumpZero(src1, fail_label));
	    result.push_back(new IRJumpZero(src2, fail_label));
	    result.push_back(new IRLoad(dest, new IRConst(1)));
	    result.push_back(new IRJump(end_label));
	    result.push_back(new IRLabel(fail_label));
	    result.push_back(new IRLoad(dest, new IRConst(0)));
	    result.push_back(new IRLabel(end_label));
	}
	else if (op == "||")
	{
	    string success_label = uniq_label();
	    string end_label = uniq_label();
	    result.push_back(new IRJumpNotZero(src1, success_label));
	    result.push_back(new IRJumpNotZero(src2, success_label));
	    result.push_back(new IRLoad(dest, new IRConst(0)));
	    result.push_back(new IRJump(end_label));
	    result.push_back(new IRLabel(success_label));
	    result.push_back(new IRLoad(dest, new IRConst(1)));
	    result.push_back(new IRLabel(end_label));
	}
	else if (op == "==")
	{
	    result.push_back(new IREqual(dest, src1, src2));
	}
	else if (op == "!=")
	{
	    result.push_back(new IRUnequal(dest, src1, src2));
	}
	else if (op == ">=")
	{
	    result.push_back(new IRGreaterEqual(dest, src1, src2));
	}
	else if (op == "<=")
	{
	    result.push_back(new IRLessEqual(dest, src1, src2));
	}
	else if (op == ">")
	{
	    result.push_back(new IRGreater(dest, src1, src2));
	}
	else if (op == "<")
	{
	    result.push_back(new IRLess(dest, src1, src2));
	}
	
	return dest;
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "(";
	first->pretty_print(out, 0);
	out << " " << op << " ";
	second->pretty_print(out, 0);
	out << ") ";
	return out;
    } 

};

class Factor : public Expression
{
public:
    Factor(ASTType _type, ASTDebug debug)
    : Expression(_type, debug)
    {}

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	result.push_back(new IRExpr());
	return new IROperand();
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	out << " Empty Factor ";
	return out;
    } 
};

class Unary : public Factor
{
public:
    Expression* inner;
    string op;
    
    Unary(string _op, Expression* _inner, ASTDebug debug)
    :
	Factor(UNARY_AST, debug),
	inner(_inner),
	op(_op)
    {}

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	inner->resolve_identifiers(ident_map);
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	inner->do_type_checking(symbol_table);
	return new PrimitiveType("int");
    }

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* src = inner->emit(result);
	IRVar* dest = temp_var();

	if(op == "-")
	{
	    result.push_back(new IRNeg(dest, src));
	}
	else if (op == "~")
	{
	    result.push_back(new IRNot(dest, src));
	}
	else if (op == "!")	// !x is the same as x == 0
	{
	    result.push_back(new IREqual(dest, src, new IRConst(0)));
	}
	else if (op == "++")
	{
	    result.push_back(new IRAdd(dest, src, new IRConst(1)));
	}
	else if (op == "--")
	{
	    result.push_back(new IRSub(dest, src, new IRConst(1)));
	}
	
	return dest;
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << op;
	inner->pretty_print(out, 0);
	return out;
    } 
};

class PostfixUnary : public Factor
{
public:
    Expression* inner;
    string op;
    
    PostfixUnary(string _op, Expression* _inner, ASTDebug debug)
    :
	Factor(UNARY_AST, debug),
	inner(_inner),
	op(_op)
    {}

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	inner->do_type_checking(symbol_table);
	return new PrimitiveType("int");
    }

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	inner->resolve_identifiers(ident_map);
    }

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* src = inner->emit(result);

	if (op == "++")
	{
	    result.push_back(new IRAdd(src, src, new IRConst(1)));
	}
	else if (op == "--")
	{
	    result.push_back(new IRSub(src, src, new IRConst(1)));
	}
	
	return src;
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	inner->pretty_print(out, 0);
	out << op;
	return out;
    } 
};

// consts will always be ints, as we can only use integers
class Constant : public Factor
{
public:
    int val;
    Constant(int _val, ASTDebug debug)
    : Factor(CONST, debug),
    val(_val)
    {}

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	return new PrimitiveType("int");
    }
    
    virtual IRConst* emit(vector<IRNode*>& result)
    {
        return new IRConst(val);
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << val;
	return out;
    } 
};

class Variable : public Expression
{
public:
    string name;
    Variable(string _name, ASTDebug debug)
    :
	Expression(VAR, debug),
	name(_name)
    {}

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	return new IRVar(name);
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	if (symbol_table.count(name) > 0)
	{
	    if (symbol_table[name].type->get_typename() == "function")
	    {
		fail("Function used as variable", debug_info);
	    }
	}
	return new PrimitiveType("int");
    }

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	if (ident_map.count(name) == 0)
	{
	    fail("Variable " + name + " Not declared", debug_info);
	}

	name = ident_map[name].name;
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << name;
	return out;
    } 
};

class Assignment : public Expression
{
    Expression* dest;
    Expression* src;

public:
    Assignment(Expression* _dest, Expression* _src, ASTDebug debug)
    :
	Expression(ASSIGNMENT, debug),
	dest(_dest),
	src(_src)
    {}

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	if (dest->type != VAR)
	{
	    stringstream s;
	    dest->pretty_print(s, 0);
	    fail("Invalid L-value " + s.str(), debug_info);
	}

	src->resolve_identifiers(ident_map);
	dest->resolve_identifiers(ident_map);
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	src->do_type_checking(symbol_table);
	return new PrimitiveType("int");
    }

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* dest_var = dest->emit(result);
	IROperand* value = src->emit(result);
	IRLoad* result_op = new IRLoad(dest_var, value);
	result.push_back(result_op);
	return value;		// assignement returns the assigned value
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "ASSIGN ";
	dest->pretty_print(out, 0);
	out << " = ";
	src->pretty_print(out, 0);
	return out;
    } 
};

class TernaryConditional : public Expression
{
public:
    Expression* cond;
    Expression* true_val;
    Expression* false_val;

    TernaryConditional(Expression* _cond, Expression* _true_val, Expression* _false_val, ASTDebug debug)
    :
	Expression(TERNARY_AST, debug),
	cond(_cond),
	true_val(_true_val),
	false_val(_false_val)
    {}

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	cond->resolve_identifiers(ident_map);
	true_val->resolve_identifiers(ident_map);
	false_val->resolve_identifiers(ident_map);
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	cond->do_type_checking(symbol_table);
	true_val->do_type_checking(symbol_table);
	false_val->do_type_checking(symbol_table);
	return new PrimitiveType("int");
    }

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* cond_ptr = cond->emit(result);

	IROperand* result_var = temp_var();
	
	string false_label = uniq_label();
	string end_label = uniq_label();
	
	result.push_back(new IRJumpZero(cond_ptr, false_label));
	IROperand* true_result = true_val->emit(result);
	result.push_back(new IRLoad(result_var, true_result));
	result.push_back(new IRJump(end_label));
	result.push_back(new IRLabel(false_label));
	IROperand* false_result = false_val->emit(result);
	result.push_back(new IRLoad(result_var, false_result));
	result.push_back(new IRLabel(end_label));

	return result_var;
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "(TERNARY";
	cond->pretty_print(out, 0);
	out << " ? ";
	true_val->pretty_print(out, 0);
	out << " : ";
	false_val->pretty_print(out, 0);
	out << ")" <<  endl;
	return out;
    } 
    
};

class Return : public Statement
{
public:
    Expression* val;
    Return(Expression* _val, ASTDebug debug)
    :
	Statement(RET, debug),
	val(_val)
    {}

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	val->resolve_identifiers(ident_map);
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	val->do_type_checking(symbol_table);
	return new PrimitiveType("int");
    }
    
    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* res_val = val->emit(result);
	IRReturn* return_statement = new IRReturn(res_val);
	result.push_back(return_statement);

	return res_val;
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "RETURN ";
	val->pretty_print(out, 0);
	out << endl;
	return out;
    } 
};

class Condition : public Statement
{
public:
    Expression* cond;
    BlockItem* then;
    Statement* otherwise;

    // otherwise (else clause may not exist, hence that pointer could be zero)
    Condition(Expression* _cond, Statement* _then, Statement* _else, ASTDebug debug)
    :
	Statement(CONDITIONAL, debug),
	cond(_cond),
	then(_then),
	otherwise(_else)
    {}

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	cond->do_type_checking(symbol_table);
	then->do_type_checking(symbol_table);
	if (otherwise)
	    otherwise->do_type_checking(symbol_table);
	
	return nullptr;
    }

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	cond->resolve_identifiers(ident_map);
	then->resolve_identifiers(ident_map);

	if (otherwise)
	    otherwise->resolve_identifiers(ident_map);
    }

    virtual void label_loops(string curr_loop_label)
    {
	then->label_loops(curr_loop_label);
	if (otherwise)
	    otherwise->label_loops(curr_loop_label);
    }

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* cond_ptr = cond->emit(result);

	if (otherwise)		// if the condition has an else clause
	{
	    string else_label = uniq_label();
	    string end_label = uniq_label();
	
	    result.push_back(new IRJumpZero(cond_ptr, else_label));
	    then->emit(result);
	    result.push_back(new IRJump(end_label));
	    result.push_back(new IRLabel(else_label));

	    otherwise->emit(result);

	    result.push_back(new IRLabel(end_label));
	}
	else			// lone if statement
	{
	    string end_label = uniq_label();
	
	    result.push_back(new IRJumpZero(cond_ptr, end_label));
	    then->emit(result);
	    result.push_back(new IRLabel(end_label));
	}

	return new IROperand();
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "IF ";
	
	cond->pretty_print(out, 0);
	out << ":" << endl;

	then->pretty_print(out, indentation + 1);
	
	if (otherwise)
	{
	    indent();
	    out << "ELSE:" << endl;
	    otherwise->pretty_print(out, indentation + 1);
	}
	
	return out;
    } 
    
};

class Compound : public Statement
{
public:
    Block* body;

    Compound(Block* _body, ASTDebug debug)
    :
	Statement(COMPOUND, debug),
	body(_body)
    {}

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	body->do_type_checking(symbol_table);
	return new PrimitiveType("int");
    }
    
    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	body->resolve_identifiers(ident_map);
    }

    virtual void label_loops(string curr_loop_label)
    {
	body->label_loops(curr_loop_label);
    }

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	for (BlockItem* b : body->items) {
	    b->emit(result);
	}

	return new IROperand();
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {

	body->pretty_print(out, indentation);
	
	return out;
    } 
};

class For : public Statement
{
public:
    BlockItem* initializer;	// can be either expression or declaration
    Expression* condition;
    Expression* post;		// most commonly its increment
    Statement* body;
    string label;

    For(BlockItem* _initializer,
	Expression* _condition,
	Expression* _post,
	Statement* _body,
	ASTDebug debug)
    :
	Statement(FOR, debug),
	initializer(_initializer),
	condition(_condition),
	post(_post),
	body(_body),
	label("")
    {}

    virtual IRNode* emit(vector<IRNode*>& result) {

	if (initializer)
	    initializer->emit(result);
	
	result.push_back(new IRLabel("start_" + label));

	IROperand* cond_ptr;
	if (condition)
	    cond_ptr = condition->emit(result);
	else
	    cond_ptr = new IRConst(1);
	
	result.push_back(new IRJumpZero(cond_ptr, "break_" + label));
	
	body->emit(result);

	result.push_back(new IRLabel("continue_" + label));

	if (post)
	    post->emit(result);

	result.push_back(new IRJump("start_" + label));
	
	result.push_back(new IRLabel("break_" + label));
	
	return nullptr;
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	initializer->do_type_checking(symbol_table);
	condition->do_type_checking(symbol_table);
	post->do_type_checking(symbol_table);
	body->do_type_checking(symbol_table);
	return new PrimitiveType("int");
    }

    virtual void label_loops(string curr_loop_label)
    {
	label = uniq_loop_label("for");
	body->label_loops(label);
    }

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {

	// as the for loop initializer introduces a new variable scope
	map<string, identifier> ident_map_copy;
	copy_ident_map(ident_map_copy, ident_map);
	
	if (initializer)
	    initializer->resolve_identifiers(ident_map_copy);
	if (condition)
	    condition->resolve_identifiers(ident_map_copy);
	if (post)
	    post->resolve_identifiers(ident_map_copy);

	body->resolve_identifiers(ident_map_copy);
    }
    
    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "FOR [" << label << "] (";
	if (initializer)
	    initializer->pretty_print(out, 0);
	out << " ; ";
	if (condition)
	    condition->pretty_print(out, 0);
	out << " ; ";
	if (post)
	    post->pretty_print(out, 0);
	out << "): " << endl;
	body->pretty_print(out, indentation+1);
	return out;
    }
};

class While : public Statement
{
public:
    Expression* condition;
    Statement* body;
    string label;

    While(Expression* _cond, Statement* _body, ASTDebug _debug)
    :
	Statement(WHILE, _debug),
	condition(_cond),
	body(_body)
    {}

    virtual IRNode* emit(vector<IRNode*>& result) {

	result.push_back(new IRLabel("continue_" + label));
	
	IROperand* cond_ptr = condition->emit(result);
	result.push_back(new IRJumpZero(cond_ptr, "break_" + label));
	
	body->emit(result);

	result.push_back(new IRJump("continue_" + label));
	
	result.push_back(new IRLabel("break_" + label));
	
	return nullptr;
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {	
	condition->do_type_checking(symbol_table);
	body->do_type_checking(symbol_table);
	return new PrimitiveType("int");
    }

    virtual void label_loops(string curr_loop_label)
    {
	label = uniq_loop_label("while");
	body->label_loops(label);
    }

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {

	condition->resolve_identifiers(ident_map);
	body->resolve_identifiers(ident_map);
    }
    
    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "WHILE [" << label << "] ";
	condition->pretty_print(out, 0);
	out << ": " << endl;
	body->pretty_print(out, indentation+1);
	return out;
    }
};

class DoWhile : public Statement
{
public:
    Expression* condition;
    Statement* body;
    string label;

    DoWhile(Expression* _cond, Statement* _body, ASTDebug _debug)
    :
	Statement(DO_WHILE, _debug),
	condition(_cond),
	body(_body)
    {}

    virtual IRNode* emit(vector<IRNode*>& result) {

	string start_label = "start_" + label;
	result.push_back(new IRLabel(start_label));
	
	body->emit(result);

	result.push_back(new IRLabel("continue_" + label));

	IROperand* cond_ptr = condition->emit(result);
	result.push_back(new IRJumpNotZero(cond_ptr, start_label));

	result.push_back(new IRLabel("break_" + label));
	
	return new IRNode();
    }

    virtual Type* do_type_checking(map<string, symbol>& symbol_table)
    {
	condition->do_type_checking(symbol_table);
	body->do_type_checking(symbol_table);
	return new PrimitiveType("int");
    }

    virtual void label_loops(string curr_loop_label)
    {
	label = uniq_loop_label("do");
	body->label_loops(label);
    }

    virtual void resolve_identifiers(map<string, identifier>& ident_map)
    {
	condition->resolve_identifiers(ident_map);
	body->resolve_identifiers(ident_map);
    }
    
    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "DO [" << label << "]:" << endl;
	body->pretty_print(out, indentation+1);

	indent();
	out << "WHILE ";
	condition->pretty_print(out, 0);
	
	return out;
    }
};

class Break : public Statement
{
public:
    string label;

    Break(ASTDebug _debug)
    :
	Statement(BREAK, _debug)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	result.push_back(new IRJump("break_" + label));
	return nullptr;
    }
    
    virtual void label_loops(string curr_loop_label)
    {
	label = curr_loop_label;
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();
	out << "BREAK [" << label << "]" << endl;
	return out;
    }
};

class Continue : public Statement
{
public:
    string label;

    Continue(ASTDebug _debug)
    :
	Statement(CONTINUE, _debug)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	result.push_back(new IRJump("continue_" + label));
	return nullptr;
    }

    virtual void label_loops(string curr_loop_label)
    {
	label = curr_loop_label;
    }

    virtual ostream& pretty_print(ostream& out, int indentation)
    {
	indent();	
	out << "CONTINUE [" << label << "]" << endl;
	return out;
    }
};

Program* parse(deque<token>& tokens);
ostream& operator<<(ostream& out, AST& ast);
