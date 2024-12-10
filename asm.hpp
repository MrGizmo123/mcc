#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <sstream>

using namespace std;

#define com(c) out << "\t;; " << c << endl
#define asm(x) out << left << "\t" << setw(20) << x << endl
#define asmc(x,c) out << left << "\t" << setw(20) << x << " ; " << c << endl
#define asml(x) out << left << x << ":" << endl 
#define com_self() stringstream stream; pretty_print(stream); com(stream.str())

using namespace std;

enum Register
{
    r0	,  r1,  r2,  r3,
    r4	,  r5,  r6,  r7,
    r8	,  r9, r10, r11,
    r12	, r13, r14, r15,
    rsp , A, B
};

enum ASMOperandType
{
    REGISTER,
    MEMORY,
    IMMEDIATE,
    PSUEDO
};

string uniq_label();

class ASMNode
{
public:
    virtual void pretty_print(ostream& out)
    {
	out << "ASM Node" << endl;
    }

    virtual void legalize(unordered_map<string, int>& temps) {}

    virtual void emit(ostream& out) { out << "asm_node" << endl; }
};

class ASMInstruction : public ASMNode
{
public:
    virtual void pretty_print(ostream& out)
    {
	out << "ASM Instruction" << endl;
    }

    virtual void emit(ostream& out) { out << "asm_instruction" << endl; }
};

class ASMAllocateStack : public ASMInstruction
{
public:
    int size;
    ASMAllocateStack(int _size) : size(_size) {}

    virtual void pretty_print(ostream& out)
    {
	out << "AllocateStack(" << size << ")";
    }

    virtual void emit(ostream& out)
    {
	com("Function prologue");
	asmc("ldrs %r15", "move rsp to rbp");
	asm("ldas");
	asm("ldbi " + to_string(size));
	asm("sub");
	asm("ldsa");
	out << endl;
    }
};

class ASMFunction : public ASMNode
{
public:
    string name;
    vector<ASMNode*> body;
    ASMAllocateStack* stack_space;

    ASMFunction(string _name, vector<ASMNode*> _body)
    : body(_body),
    name(_name)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << name << ":" << endl;
	for (ASMNode* i : body)
	    {
		out << "\t";
		i->pretty_print(out);
	    }
    }

    virtual void legalize(unordered_map<string, int>& temps)
    {
	for (ASMNode* i : body)
	    {
		i->legalize(temps);
	    }
	stack_space = new ASMAllocateStack(temps.size()); // save how many locations we need to reserve on the stack
    }

    virtual void emit(ostream& out)
    {
	out << name << ":" << endl;
	
	stack_space->emit(out);	// function prologue
	
	for (ASMNode* i : body)
	    {
		i->emit(out);
	    }
    }
};

class ASMProgram : public ASMNode
{
public:
    ASMFunction* entry;
    ASMProgram(ASMFunction* _entry) : entry(_entry) {}

    virtual void pretty_print(ostream& out)
    {
	out << "prog" << endl;
	entry->pretty_print(out);
    }

    virtual void legalize(unordered_map<string, int>& temps)
    {
	entry->legalize(temps);
    }

    virtual void emit(ostream& out)
    {
	com("program");
	asmc("lds 0xfffe", "initialize stack pointer");
	entry->emit(out);
	asmc("hlt", "halt at the end of program");
    }
};

class ASMOperand : public ASMNode
{
public:
    ASMOperandType type;
    
    ASMOperand(ASMOperandType _type=IMMEDIATE) : type(_type) {}
    
    virtual void pretty_print(ostream& out)
    {
	out << "ASM Operand";
    }

    virtual ASMOperand* legalize_op(unordered_map<string, int>& temps)
    {
	return this;
    }

    virtual void emit_location(ostream& out)
    {
	out << "location" << endl;
    }
};

class ASMImmediate : public ASMOperand
{
public:
    int val;
    ASMImmediate(int _val) : ASMOperand(IMMEDIATE), val(_val) {}

    virtual void pretty_print(ostream& out)
    {
	out << "Imm(" << val << ")";
    }
    
    virtual void emit(ostream& out)
    {
	asmc("ldai " + to_string(val), "immediate");
    }

    virtual string get_val()
    {
	return to_string(val);
    }
};

class ASMRegister : public ASMOperand
{
public:
    Register name;
    ASMRegister(Register _name) : ASMOperand(REGISTER), name(_name) {}

    virtual void pretty_print(ostream& out)
    {
	out << "Reg(" << name << ")";
    }

    virtual void emit(ostream& out)
    {
	out << "%r" << name;
    }
};

class ASMStack : public ASMOperand
{
public:
    int offset;
    ASMStack(int _offset) : ASMOperand(MEMORY), offset(_offset) {}

    virtual void pretty_print(ostream& out)
    {
	out << "Stack(" << offset << ")";
    }

    virtual void emit(ostream& out)
    {
	com("Stack");
	asmc("ldar %r15", "load rsp into A register");
	asmc("ldbi " + to_string(offset), "load offset into B reg");
	asmc("sub", " now address is stored in A reg, can be acessed using ldmaa");
	out << endl;
    }
};

class ASMPsuedoReg : public ASMOperand
{
public:
    string ident;
    ASMPsuedoReg(string _ident) : ASMOperand(PSUEDO), ident(_ident) {}

    virtual void pretty_print(ostream& out)
    {
	out << "Psuedo(" << ident << ")";
    }

    virtual ASMOperand* legalize_op(unordered_map<string, int>& temps)
    {
        if (temps.count(ident) == 0)
	{
	    temps[ident] = temps.size(); // the size will give the offset for the stack
	    return new ASMStack(temps[ident]);
	}

	return new ASMStack(temps[ident]);
    }
};

// takes a GP register and puts it in the aluregs
void emit_register_fetch(ASMRegister* reg,
			 Register alureg,
			 ostream& out);

// takes a stack value and puts it in the aluregs
void emit_stack_fetch(ASMStack* stk,
		      Register alureg,
		      ostream& out);

void emit_lda_operand(ASMOperand* src,
		      ostream& out);

void emit_ldb_operand(ASMOperand* src,
		      ostream& out);

void emit_sta_operand(ASMOperand* dest,
		      ostream& out);

void emit_stb_operand(ASMOperand* dest,
		      ostream& out);

class ASMLoad : public ASMInstruction
{
public:
    ASMOperand* dest;
    ASMOperand* src;

    ASMLoad(ASMOperand* _dest, ASMOperand* _src)
    : dest(_dest),
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

    virtual void legalize(unordered_map<string, int>& temps)
    {
	dest = dest->legalize_op(temps);
	src = src->legalize_op(temps);
    }

    virtual void emit(ostream& out)
    {
	com_self();
	emit_lda_operand(src, out);
	emit_sta_operand(dest, out);
	out << endl;
    }
};

class ASMReturn : public ASMInstruction
{
public:
    ASMReturn() {}

    virtual void pretty_print(ostream& out)
    {
	out << "Return()" << endl;
    }

    virtual void emit(ostream& out)
    {
	com_self();
	// asm("ret");
	// asm("ret2");
	out << endl;
    }
};

class ASMUnary : public ASMInstruction
{
public:
    ASMOperand* dest;
    ASMOperand* src;
    ASMUnary(ASMOperand* _dest, ASMOperand* _src)
    : dest(_dest),
    src(_src)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Unary(";
	dest->pretty_print(out);
	out << ", ";
	src->pretty_print(out);
	out << ")" << endl;
    }

    virtual void legalize(unordered_map<string, int>& temps)
    {
        dest = dest->legalize_op(temps);
	src = src->legalize_op(temps);
    }
};

class ASMBinary : public ASMInstruction
{
public:
    ASMOperand* dest;
    ASMOperand* src1;
    ASMOperand* src2;
    
    ASMBinary(ASMOperand* _dest, ASMOperand* _src1, ASMOperand* _src2)
    : dest(_dest),
    src1(_src1),
    src2(_src2)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Binary(";
	dest->pretty_print(out);
	out << ", ";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void legalize(unordered_map<string, int>& temps)
    {
        dest = dest->legalize_op(temps);
	src1 = src1->legalize_op(temps);
	src2 = src2->legalize_op(temps);
    }
};


// bitwise not
class ASMNot : public ASMUnary 
{
public:
    ASMNot(ASMOperand* _dest, ASMOperand* _src)
    : ASMUnary(_dest, _src)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Not(";
	dest->pretty_print(out);
	out << ", ";
	src->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(ostream& out)
    {
	com_self();
	
        emit_ldb_operand(src, out);

	asmc("ldai 0xffff", "we subtract the number from -1 to get its bitwise inversion");
	asm("sub");

	emit_sta_operand(dest, out);

	out << endl << endl;
    }
};

// reverse the sign
class ASMNeg : public ASMUnary
{
public:
    ASMNeg(ASMOperand* _dest, ASMOperand* _src)
    : ASMUnary(_dest, _src)
    {}

    virtual void pretty_print(ostream& out)
    {
        out << "Neg(";
	dest->pretty_print(out);
	out << ", ";
	src->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(ostream& out)
    {
	com_self();

	emit_ldb_operand(src, out);

	asmc("ldai 0", "we subtract the number from 0 to get its negation");
	asm("sub");

	emit_sta_operand(dest, out);

	out << endl << endl;
    }
};

class ASMAdd : public ASMBinary
{
public:
    ASMAdd(ASMOperand* _dest, ASMOperand* _src1, ASMOperand* _src2)
    : ASMBinary(_dest, _src1, _src2)
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

    virtual void emit(ostream& out)
    {
	com_self();

	emit_lda_operand(src1, out);
	emit_ldb_operand(src2, out);

	asm("add");

	emit_sta_operand(dest, out);

	out << endl << endl;
    }
    
};

class ASMSub : public ASMBinary
{
public:
    ASMSub(ASMOperand* _dest, ASMOperand* _src1, ASMOperand* _src2)
    : ASMBinary(_dest, _src1, _src2)
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

    virtual void emit(ostream& out)
    {
	com_self();

	emit_lda_operand(src1, out);
	emit_ldb_operand(src2, out);

	asm("sub");

	emit_sta_operand(dest, out);

	out << endl << endl;
    }
    
};

class ASMMul : public ASMBinary
{
public:
    ASMMul(ASMOperand* _dest, ASMOperand* _src1, ASMOperand* _src2)
    : ASMBinary(_dest, _src1, _src2)
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
    
};

class ASMDiv : public ASMBinary
{
public:
    ASMDiv(ASMOperand* _dest, ASMOperand* _src1, ASMOperand* _src2)
    : ASMBinary(_dest, _src1, _src2)
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
    
};

class ASMMod : public ASMBinary
{
public:
    ASMMod(ASMOperand* _dest, ASMOperand* _src1, ASMOperand* _src2)
    : ASMBinary(_dest, _src1, _src2)
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
    
};

class ASMBitAnd : public ASMBinary
{
public:
    ASMBitAnd(ASMOperand* _dest, ASMOperand* _src1, ASMOperand* _src2)
    : ASMBinary(_dest, _src1, _src2)
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

    virtual void emit(ostream& out)
    {
	com_self();

	emit_lda_operand(src1, out);
	emit_ldb_operand(src2, out);

	asm("and");

	emit_sta_operand(dest, out);

	out << endl << endl;
    }
    
    
};

class ASMJump : public ASMNode
{
public:
    string jump_to;

    ASMJump(string jmp_to)
    :
	jump_to(jmp_to)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Jump(" << jump_to << ")" << endl;
    }

    virtual void emit(ostream& out)
    {
	com_self();

	asm("jmp " + jump_to);

	out << endl << endl;
    }
};

class ASMJumpZero : public ASMNode
{
public:
    string jump_to;

    ASMJumpZero(string jmp_to)
    :
	jump_to(jmp_to)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "JumpZero(" << jump_to << ")" << endl;
    }

    virtual void emit(ostream& out)
    {
	com_self();

	asmc("jz " + jump_to, "jump if zero");

	out << endl << endl;
    }
};

class ASMJumpLess : public ASMNode
{
public:
    string jump_to;

    ASMJumpLess(string jmp_to)
    :
	jump_to(jmp_to)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "JumpLesser(" << jump_to << ")" << endl;
    }

    virtual void emit(ostream& out)
    {
	com_self();

	asmc("jl " + jump_to, "jump if lesser");

	out << endl << endl;
    }
};

class ASMJumpGreater : public ASMNode
{
public:
    string jump_to;

    ASMJumpGreater(string jmp_to)
    :
	jump_to(jmp_to)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "JumpGreater(" << jump_to << ")" << endl;
    }

    virtual void emit(ostream& out)
    {
	com_self();

	asmc("jg " + jump_to, "jump if lesser");

	out << endl << endl;
    }
};

class ASMCmp : public ASMBinary
{
public:
    ASMCmp(ASMOperand* _src1, ASMOperand* _src2)
    : ASMBinary(new ASMRegister(A), _src1, _src2) // cmp updates the flags and so has no dest operand
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Cmp(";
	src1->pretty_print(out);
	out << ", ";
	src2->pretty_print(out);
	out << ")" << endl;
    }

    virtual void emit(ostream& out)
    {
	com_self();

	emit_lda_operand(src1, out);
	emit_ldb_operand(src2, out);

	asm("cmp");

	out << endl << endl;
    }
    
};

class ASMLabel : public ASMNode
{
public:
    string name;

    ASMLabel(string _name)
    :
	name(_name)
    {}

    virtual void pretty_print(ostream& out)
    {
	out << "Label(" << name << ")" << endl;
    }

    virtual void emit(ostream& out)
    {
	com_self();

	asml(name);

	out << endl << endl;
    }
};
