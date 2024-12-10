#pragma once

#include <deque>
#include <ostream>
#include <string>
#include "asm.hpp"
#include "tokenizer.h"
#include "tacky.hpp"

using namespace std;

IRVar* temp_name();
string uniq_label();

enum ASTType
{
    FUNC,
    PROG,
    CONST,
    VAR,
    RET,
    UNARY_AST,
    BINARY_AST
};

// abstract syntax tree (actually just a node in the AST)
class AST
{
    ASTType type;
public:
    AST(ASTType _type)
	: type(_type)
    {}
    
    ASTType get_type() { return type; }
    
    virtual IRNode* emit(vector<IRNode*>& result) {
	result.push_back(new IRNode());
	return new IRNode();
    }
    
    virtual ostream& pretty_print(ostream& out)
    {
	out << "Empty AST" << endl;
	return out;
    } 
};

class Statement : public AST
{
public:
    Statement(ASTType _type)
	: AST(_type)
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
    deque<Statement*> body;
    Function(string _name, string _return_type, deque<Statement*> _body)
	: AST(FUNC),
	  name(_name),
	  return_type(_return_type),
	  body(_body)
    {}

    virtual IRNode* emit(vector<IRNode*>& result)
    {
	vector<IRNode*> result_body;
	for (Statement* s : body)
	{
	    s->emit(result_body);
	}

	IRFunction* this_func = new IRFunction(name, result_body);
	result.push_back(this_func);
	return this_func;
    }

    virtual ostream& pretty_print(ostream& out)
    {
	out << "FUNC " <<  return_type << " " << name << " :" << endl;
	out << "\t" << "params : ()" << endl;
	out << "\t" << "body: " << endl;
	
	for (Statement* s : body)
	    {
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
    
    virtual ostream& pretty_print(ostream& out)
    {
	entry->pretty_print(out);
	return out;
    } 
};

class Expression : public AST
{
public:
    Expression(ASTType _type)
	: AST(_type)
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

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* src1 = first->emit(result);
	IROperand* src2 = second->emit(result);
	IRVar* dest = temp_name();

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

    virtual IROperand* emit(vector<IRNode*>& result)
    {
	IROperand* src = inner->emit(result);
	IRVar* dest = temp_name();

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
	
	return dest;
    }

    virtual ostream& pretty_print(ostream& out)
    {
	out << op;
	inner->pretty_print(out);
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

    virtual IRVar* emit(vector<IRNode*>& result)
    {
	return new IRVar(name);
    }

    virtual ostream& pretty_print(ostream& out)
    {
	out << " " << name << " ";
	return out;
    } 
};

class Return : public Statement
{
public:
    Expression* val;
    Return(Expression* _val)
	: Statement(RET),
	  val(_val)
    {}

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


Program* parse(deque<token>& tokens);
ostream& operator<<(ostream& out, AST& ast);
