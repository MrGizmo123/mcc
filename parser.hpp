#pragma once

#include <algorithm>
#include <deque>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>
#include "asm.hpp"
#include "tokenizer.h"
#include "tacky.hpp"

using namespace std;

IRVar* temp_var();
string uniq_label();
string uniq_var_name(string);


inline void fail(string message)
{
    cout << "Semantic Analysis failed. " << message << endl;
    exit(-1);
}

enum ASTType
{
    FUNC,
    PROG,
    CONST,
    VAR,
    RET,
    UNARY_AST,
    BINARY_AST,
    TERNARY_AST,
    ASSIGNMENT,
    DECLARATION,
    CONDITIONAL,
    BLOCK,
    COMPOUND,
    FOR,
    WHILE,
    DO_WHILE,
    BREAK,
    CONTINUE
};

class variable_label
{
public:
    string name;
    bool declared_in_this_scope;

    variable_label(string _name = "", bool _decl = true) : name(_name), declared_in_this_scope(_decl) {}
};

void copy_var_map(map<string, variable_label>& dest, map<string, variable_label> src);

// abstract syntax tree (actually just a node in the AST)
class AST
{
public:
    ASTType type;

    AST(ASTType _type)
    : type(_type)
    {}
    
    virtual IRNode* emit(vector<IRNode*>& result) {
	result.push_back(new IRNode());
	return new IRNode();
    }

    virtual void resolve_variables(map<string, variable_label>& var_map) {}
    
    virtual ostream& pretty_print(ostream& out)
    {
	out << "Empty AST" << endl;
	return out;
    } 
};

class BlockItem : public AST
{
public:
    BlockItem(ASTType _type)
    : AST(_type)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	result.push_back(new IRStatement());
	return new IRNode();
    }

    virtual ostream& pretty_print(ostream& out)
    {
	out << "Block Item" << endl;
	return out;
    } 
};

class Block : public AST
{
public:
    vector<BlockItem*> items;
    
    Block(vector<BlockItem*> _items)
    :
	AST(BLOCK),
	items(_items)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	for (BlockItem* s : items) {
	    s->emit(result);
	}

	return new IRNode();
    }

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	map<string, variable_label> var_map_copy;

	copy_var_map(var_map_copy, var_map);
	
	for (BlockItem* b : items) {
	    b->resolve_variables(var_map_copy);
	}
    }
    
    virtual ostream& pretty_print(ostream& out)
    {
	out << "Block" << endl;
	return out;
    } 
};

class Statement : public BlockItem
{
public:
    Statement(ASTType _type)
    : BlockItem(_type)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	result.push_back(new IRStatement());
	return new IRNode();
    }

    virtual ostream& pretty_print(ostream& out)
    {
	out << "Generic Statement" << endl;
	return out;
    } 
};

class Function : public AST
{
public:
    string name;
    string return_type;
    Block* body;
    
    Function(string _name, string _return_type, Block* _body)
    :
	AST(FUNC),
	name(_name),
	return_type(_return_type),
	body(_body)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	vector<IRNode*> result_body;

	body->emit(result_body);

	// all functions have a return 0 at the end
	result_body.push_back(new IRReturn(new IRConst(0)));

	IRFunction* this_func = new IRFunction(name, result_body);
	result.push_back(this_func);
	return this_func;
    }

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	body->resolve_variables(var_map);
    }

    virtual ostream& pretty_print(ostream& out)
    {
	out << "FUNC " <<  return_type << " " << name << " :" << endl;
	out << "\t" << "params : ()" << endl;
	out << "\t" << "body: " << endl;
	
	for (BlockItem* s : body->items) {
	    out << "\t\t";
	    s->pretty_print(out);
	}
	
	return out;
    } 
};

class Program : public AST
{
public:
    Function* entry;
    Program(Function* _entry)
	: AST(PROG),
	  entry(_entry)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
        IRFunction* ir_entry = (IRFunction*)entry->emit(result);
	return new IRProgram(ir_entry);
    }

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	entry->resolve_variables(var_map);
    }
    
    virtual ostream& pretty_print(ostream& out)
    {
	entry->pretty_print(out);
	return out;
    } 
};

class Expression : public Statement
{
public:
    Expression(ASTType _type)
    :
	Statement(_type)
    {}

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	result.push_back(new IRExpr());
	return new IROperand();
    }

    virtual ostream& pretty_print(ostream& out)
    {
	out << " Empty Expression ";
	return out;
    } 
};


class Declaration : public BlockItem
{
    string name;		// identifier
    Expression* val;

public:
    Declaration(string _name, Expression* _val)
    :
	BlockItem(DECLARATION),
	name(_name),
	val(_val)
    {}

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	if (var_map.count(name) > 0)
	{
	    if (var_map[name].declared_in_this_scope)
	    {
		fail("Redeclaration of variable " + name);
	    }
	}

	string unique_name = uniq_var_name(name);

	var_map[name] = variable_label(unique_name);

	if (val)		// check if initializer value exists
	{
	    val->resolve_variables(var_map);
	}

	name = unique_name;
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

    virtual ostream& pretty_print(ostream& out)
    {
	out << "DECLARE " << name;
	if (val)
	{
	    out << " = ";
	    val->pretty_print(out);
	}
	out << endl;
	return out;
    }
};

class Binary : public Expression
{
public:
    Expression* first;
    Expression* second;
    string op;

    Binary(string _op, Expression* _f, Expression* _s)
    : Expression(BINARY_AST),
    op(_op),
    first(_f),
    second(_s)
    {}

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	first->resolve_variables(var_map);
	second->resolve_variables(var_map);
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

    virtual ostream& pretty_print(ostream& out)
    {
	out << "(";
	first->pretty_print(out);
	out << op << " ";
	second->pretty_print(out);
	out << "\b) ";
	return out;
    } 

};

class Factor : public Expression
{
public:
    Factor(ASTType _type)
    : Expression(_type)
    {}

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	result.push_back(new IRExpr());
	return new IROperand();
    }

    virtual ostream& pretty_print(ostream& out)
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
    
    Unary(string _op, Expression* _inner)
    :
	Factor(UNARY_AST),
	inner(_inner),
	op(_op)
    {}

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	inner->resolve_variables(var_map);
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

    virtual ostream& pretty_print(ostream& out)
    {
	out << op;
	inner->pretty_print(out);
	return out;
    } 
};

class PostfixUnary : public Factor
{
public:
    Expression* inner;
    string op;
    
    PostfixUnary(string _op, Expression* _inner)
    :
	Factor(UNARY_AST),
	inner(_inner),
	op(_op)
    {}

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	inner->resolve_variables(var_map);
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

    virtual ostream& pretty_print(ostream& out)
    {
	inner->pretty_print(out);
	out << op;
	return out;
    } 
};

// consts will always be ints, as we can only use integers
class Constant : public Factor
{
public:
    int val;
    Constant(int _val)
    : Factor(CONST),
    val(_val)
    {}

    virtual IRConst* emit(vector<IRNode*>& result)
    {
        return new IRConst(val);
    }

    virtual ostream& pretty_print(ostream& out)
    {
	out << val << " ";
	return out;
    } 
};

class Variable : public Expression
{
public:
    string name;
    Variable(string _name)
	: Expression(VAR),
	  name(_name)
    {}

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	return new IRVar(name);
    }

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	if (var_map.count(name) == 0)
	{
	    fail("Variable " + name + " Not declared");
	}

	name = var_map[name].name;
    }

    virtual ostream& pretty_print(ostream& out)
    {
	out << " " << name << " ";
	return out;
    } 
};

class Assignment : public Expression
{
    Expression* dest;
    Expression* src;

public:
    Assignment(Expression* _dest, Expression* _src)
    :
	Expression(ASSIGNMENT),
	dest(_dest),
	src(_src)
    {}

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	if (dest->type != VAR)
	{
	    stringstream s;
	    dest->pretty_print(s);
	    fail("Invalid lvalue " + s.str());
	}

	src->resolve_variables(var_map);
	dest->resolve_variables(var_map);
    }

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* dest_var = dest->emit(result);
	IROperand* value = src->emit(result);
	IRLoad* result_op = new IRLoad(dest_var, value);
	result.push_back(result_op);
	return value;		// assignement returns the assigned value
    }

    virtual ostream& pretty_print(ostream& out)
    {
	out << "ASSIGN";
	dest->pretty_print(out);
	out << "= ";
	src->pretty_print(out);
	out << endl;
	return out;
    } 
};

class TernaryConditional : public Expression
{
public:
    Expression* cond;
    Expression* true_val;
    Expression* false_val;

    TernaryConditional(Expression* _cond, Expression* _true_val, Expression* _false_val)
    :
	Expression(TERNARY_AST),
	cond(_cond),
	true_val(_true_val),
	false_val(_false_val)
    {}

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	cond->resolve_variables(var_map);
	true_val->resolve_variables(var_map);
	false_val->resolve_variables(var_map);
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

    ostream& pretty_print(ostream& out)
    {
	out << "(TERNARY";
	cond->pretty_print(out);
	out << " ? ";
	true_val->pretty_print(out);
	out << " : ";
	false_val->pretty_print(out);
	out << ")" <<  endl;
	return out;
    } 
    
};

class Return : public Statement
{
public:
    Expression* val;
    Return(Expression* _val)
    :
	Statement(RET),
	val(_val)
    {}

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	val->resolve_variables(var_map);
    }
    
    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* res_val = val->emit(result);
	IRReturn* return_statement = new IRReturn(res_val);
	result.push_back(return_statement);

	return res_val;
    }

    ostream& pretty_print(ostream& out)
    {
	out << "RETURN ";
	val->pretty_print(out);
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
    Condition(Expression* _cond, Statement* _then, Statement* _else)
    :
	Statement(CONDITIONAL),
	cond(_cond),
	then(_then),
	otherwise(_else)
    {}

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	cond->resolve_variables(var_map);
	then->resolve_variables(var_map);

	if (otherwise)
	    otherwise->resolve_variables(var_map);
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

    ostream& pretty_print(ostream& out)
    {
	out << "(IF ";
	cond->pretty_print(out);
	out << ":" << endl;
	then->pretty_print(out);
	
	if (otherwise)
	{
	    out << "ELSE:" << endl;
	    otherwise->pretty_print(out);
	}

	out << ")" << endl;
	
	return out;
    } 
    
};

class Compound : public Statement
{
public:
    Block* body;

    Compound(Block* _body)
    :
	Statement(COMPOUND),
	body(_body)
    {}

    virtual void resolve_variables(map<string, variable_label>& var_map)
    {
	body->resolve_variables(var_map);
    }

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	for (BlockItem* b : body->items) {
	    b->emit(result);
	}

	return new IROperand();
    }

    ostream& pretty_print(ostream& out)
    {
	for (BlockItem* b : body->items) {
	    b->pretty_print(out);
	}
	
	return out;
    } 
};

// class For : public Statement
// {
// public:
//     ForInit* initializer;
//     Expression* condition;
//     Expression* post;		// most commonly its increment
//     string label;

//     For(ForInit* _initializer,
// 	Expression* _condition,
// 	Expression* _post)
//     :
// 	Statement(FOR),
// 	initializer(_initializer),
// 	condition(_condition),
// 	post(_post),
// 	label("")
//     {}

//     virtual IRNode* emit(vector<IRNode*>& result) {
// 	result.push_back(new IRNode());
// 	return new IRNode();
//     }

//     virtual void resolve_variables(map<string, variable_label>& var_map)
//     {
// 	initializer->resolve_variables(var_map);
//     }
    
//     virtual ostream& pretty_print(ostream& out)
//     {
// 	out << "Empty AST" << endl;
// 	return out;
//     } 
    
// };

class While : public Statement
{
    
};

class DoWhile : public Statement
{
    
};

class Break : public Statement
{
    
};

class Continue : public Statement
{
    
};

Program* parse(deque<token>& tokens);
ostream& operator<<(ostream& out, AST& ast);
