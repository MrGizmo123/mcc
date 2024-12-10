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

class IRLoad : public IRStatement
{
public:
    IROperand* dest;
    IROperand* src;

    IRLoad(IROperand* _dest, IROperand* _src)
    :
	dest(_dest),
	src(_src)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Load(";
	dest->pretty_print(out);
	out << ", ";
	src->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMLoad(dest->to_asm(), src->to_asm()));
    }
};

class IRJump : public IRStatement
{
public:
    string target;

    IRJump(string _target)
    :
	target(_target)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Jump(" << target << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMJump(target));
    }
};

class IRJumpZero : public IRStatement
{
public:
    IROperand* condition;
    string target;

    IRJumpZero(IROperand* _condition, string _target)
    :
	target(_target),
	condition(_condition)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "JumpZero(";
	condition->pretty_print(out);
	out << ", " << target << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMCmp(condition->to_asm(), new ASMImmediate(0)));
	result.push_back(new ASMJumpZero(target));
    }
};

class IRJumpNotZero : public IRStatement
{
public:
    IROperand* condition;
    string target;

    IRJumpNotZero(IROperand* _condition, string _target)
    :
	target(_target),
    	condition(_condition)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "JumpNotZero(";
	condition->pretty_print(out);
	out << ", " << target << ")" << endl;;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	string fail_label = uniq_label();
	
	result.push_back(new ASMCmp(condition->to_asm(), new ASMImmediate(0)));
	result.push_back(new ASMJumpZero(fail_label));
	result.push_back(new ASMJump(target));
	result.push_back(new ASMLabel(fail_label));
    }
};

class IRLabel : public IRStatement
{
public:
    string name;

    IRLabel(string _name)
    :
	name(_name)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Label(" << name << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMLabel(name));
    }
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
    IROperand* dest;
    IROperand* src;

    IRUnary(IROperand* _dest, IROperand* _src)
    :
	dest(_dest),
	src(_src)
    {}
};

class IRBinary : public IRExpr
{
public:
    IROperand* dest;
    IROperand* src1;
    IROperand* src2;
    
    IRBinary(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	dest(_dest),
	src1(_src1),
	src2(_src2)
    {}
};

class IRNeg : public IRUnary
{
public:
    IRNeg(IROperand* _dest, IROperand* _src)
    :
	IRUnary(_dest, _src)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Neg(";
	dest->pretty_print(out);
	out << ", ";
	src->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMNeg(dest->to_asm(), src->to_asm()));
    }
};

class IRNot : public IRUnary
{
public:
    IRNot(IROperand* _dest, IROperand* _src)
    :
	IRUnary(_dest, _src)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Not(";
	dest->pretty_print(out);
	out << ", ";
	src->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMNot(dest->to_asm(), src->to_asm()));
    }
};

class IRAdd : public IRBinary
{
public:
    IRAdd(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Add(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMAdd(dest->to_asm(), src1->to_asm(), src2->to_asm()));
    }
};

class IRSub : public IRBinary
{
public:
    IRSub(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Sub(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMSub(dest->to_asm(), src1->to_asm(), src2->to_asm()));
    }
};

class IRMul : public IRBinary
{
public:
    IRMul(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Mul(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMMul(dest->to_asm(), src1->to_asm(), src2->to_asm()));
    }
};

class IRDiv : public IRBinary
{
public:
    IRDiv(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Div(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMDiv(dest->to_asm(), src1->to_asm(), src2->to_asm()));
    }
};

class IRMod : public IRBinary
{
public:
    IRMod(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Mod(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMMod(dest->to_asm(), src1->to_asm(), src2->to_asm()));
    }
};

class IRBitAnd : public IRBinary
{
public:
    IRBitAnd(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "BitAnd(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	result.push_back(new ASMBitAnd(dest->to_asm(), src1->to_asm(), src2->to_asm()));
    }
};

class IREqual : public IRBinary
{
public:
    IREqual(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Equal(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	string equal_label = uniq_label();
	string end_label = uniq_label();
	
	result.push_back(new ASMCmp(src1->to_asm(), src2->to_asm()));
	result.push_back(new ASMJumpZero(equal_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(0)));
	result.push_back(new ASMJump(end_label));
	result.push_back(new ASMLabel(equal_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(1)));
	result.push_back(new ASMLabel(end_label));
	
    }
};

class IRUnequal : public IRBinary
{
public:
    IRUnequal(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Unequal(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	string equal_label = uniq_label();
	string end_label = uniq_label();
	
	result.push_back(new ASMCmp(src1->to_asm(), src2->to_asm()));
	result.push_back(new ASMJumpZero(equal_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(1)));
	result.push_back(new ASMJump(end_label));
	result.push_back(new ASMLabel(equal_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(0)));
	result.push_back(new ASMLabel(end_label));
	
    }
};

class IRGreaterEqual : public IRBinary
{
public:
    IRGreaterEqual(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "GreaterEqual(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	string fail_label = uniq_label();
	string end_label = uniq_label();
	
	result.push_back(new ASMCmp(src1->to_asm(), src2->to_asm()));
	result.push_back(new ASMJumpLess(fail_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(1)));
	result.push_back(new ASMJump(end_label));
	result.push_back(new ASMLabel(fail_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(0)));
	result.push_back(new ASMLabel(end_label));
	
    }
};

class IRLessEqual : public IRBinary
{
public:
    IRLessEqual(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "LessEqual(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	string fail_label = uniq_label();
	string end_label = uniq_label();
	
	result.push_back(new ASMCmp(src1->to_asm(), src2->to_asm()));
	result.push_back(new ASMJumpGreater(fail_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(1)));
	result.push_back(new ASMJump(end_label));
	result.push_back(new ASMLabel(fail_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(0)));
	result.push_back(new ASMLabel(end_label));
	
    }
};

class IRLess : public IRBinary
{
public:
    IRLess(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Less(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	string success_label = uniq_label();
	string end_label = uniq_label();
	
	result.push_back(new ASMCmp(src1->to_asm(), src2->to_asm()));
	result.push_back(new ASMJumpLess(success_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(0)));
	result.push_back(new ASMJump(end_label));
	result.push_back(new ASMLabel(success_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(1)));
	result.push_back(new ASMLabel(end_label));
	
    }
};


class IRGreater : public IRBinary
{
public:
    IRGreater(IROperand* _dest, IROperand* _src1, IROperand* _src2)
    :
	IRBinary(_dest, _src1, _src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Greater(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(vector<ASMNode*>& result)
    {
	string success_label = uniq_label();
	string end_label = uniq_label();
	
	result.push_back(new ASMCmp(src1->to_asm(), src2->to_asm()));
	result.push_back(new ASMJumpGreater(success_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(0)));
	result.push_back(new ASMJump(end_label));
	result.push_back(new ASMLabel(success_label));
	result.push_back(new ASMLoad(dest->to_asm(), new ASMImmediate(1)));
	result.push_back(new ASMLabel(end_label));
	
    }
};


