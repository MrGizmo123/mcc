#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

#include "asm.hpp"

using namespace std;

string uniq_label();

class IRNode
{
public:
    virtual void pretty_print(ostream& out) { out << "Empty IR Node" << endl;}

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMNode());
    }
};

class IROperand : public IRNode
{
public:
    virtual ASMOperand* to_asm()
    {
	return new ASMOperand();
    }
};

class IRVar : public IROperand
{
public:
    string name;
    IRVar(string _name) : name(_name) {}

    virtual void pretty_print(ostream& out) { out << "Var(" << name << ")";}

    virtual ASMOperand* to_asm()
    {
	return new ASMPsuedoReg(name);
    }
};

class IRConst : public IROperand
{
public:
    int value;
    IRConst(int _value) : value(_value) {}

    virtual void pretty_print(ostream& out) { out << "Const(" << value << ")";}

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMImmediate(value));
    }

    virtual ASMOperand* to_asm()
    {
	return new ASMImmediate(value);
    }
};

class IRExpr : public IRNode
{
        
};

class IRStatement : public IRNode
{
    
};

class IRReturn : public IRStatement
{
public:
    IROperand* val;
    IRReturn(IROperand* _val) : val(_val) {}

    virtual void pretty_print(ostream& out)
    {
	out << "Return(";
	val->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMLoad(new ASMRegister(r0), val->to_asm()));
	result.push_back(new ASMReturn());
    }
};

class IRFunction : public IRNode
{
public:
    string name;
    vector<IRNode*> body;
    IRFunction(string _name, vector<IRNode*> _body)
	: name(_name),
	  body(_body)
    {}
    
    virtual void pretty_print(ostream& out)
    {
	out << "Func" << endl;
	for (IRNode* s : body)
	{
	    s->pretty_print(out);
	}
	out << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	vector<ASMNode*> asm_body;
	for (IRNode* s : body)
	{
	    s->emit(asm_body);
	}
	
	result.push_back(new ASMFunction(name, asm_body));
    }
};

class IRProgram : public IRNode
{
public:
    IRFunction* entry;
    IRProgram(IRFunction* _entry) : entry(_entry) {}

    virtual void pretty_print(ostream& out)
    {
	out << "Prog" << endl;
	entry->pretty_print(out);
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	vector<ASMNode*> asm_body;

	entry->emit(asm_body);
	
	ASMProgram* asm_prog = new ASMProgram((ASMFunction*)(asm_body[0]));
        result.push_back(asm_prog);
    }
};

class IRUnary : public IRExpr
{
public:
    string op;
    IROperand* dest;
    IROperand* src;

    IRUnary(string _op, IROperand* _dest, IROperand* _src)
	: op(_op),
	  dest(_dest),
	  src(_src)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Unary(" << op << ", ";
	dest->pretty_print(out);
	out << ", ";
	src->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	if(op == "-")
	{
	    result.push_back(new ASMNeg(dest->to_asm(), src->to_asm()));
	}
	else if (op == "~")
	{
	    result.push_back(new ASMNot(dest->to_asm(), src->to_asm()));
	}
    }
};

class IRBinary : public IRExpr
{
public:
    string op;
    IROperand* dest;
    IROperand* src1;
    IROperand* src2;

    IRBinary(string _op, IROperand* _dest, IROperand* _src1, IROperand* _src2)
    : op(_op),
    dest(_dest),
    src1(_src1),
    src2(_src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Binary(" << op << ", ";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	if(op == "+")
	{
	    result.push_back(new ASMAdd(dest->to_asm(), src1->to_asm(), src2->to_asm()));
	}
	else if (op == "-")
	{
	    result.push_back(new ASMSub(dest->to_asm(), src1->to_asm(), src2->to_asm()));
	}
	else if (op == "*")
	{
	    result.push_back(new ASMMul(dest->to_asm(), src1->to_asm(), src2->to_asm()));
	}
	else if (op == "/")
	{
	    result.push_back(new ASMDiv(dest->to_asm(), src1->to_asm(), src2->to_asm()));
	}
	else if (op == "%")
	{
	    result.push_back(new ASMMod(dest->to_asm(), src1->to_asm(), src2->to_asm()));
	}
	else if (op == "&")
	{
	    result.push_back(new ASMBitAnd(dest->to_asm(), src1->to_asm(), src2->to_asm()));
	}
	else if (op == "&&")
	{
	    string fail_label = uniq_label();
	    result.push_back(new ASMJumpNotZero(src1->to_asm(), fail_label));
	    result.push_back(new ASMJumpNotZero(src2->to_asm(), fail_label));	    
	}
    }
};
