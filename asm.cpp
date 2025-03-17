#include "asm.hpp"

#define ldr(x) (x == A ? "ldar " : "ldbr ")

using namespace std;

// takes a GP register and puts it in the aluregs
void emit_register_fetch(ASMRegister* reg,
			 Register alureg,
			 ostream& out)
{
    string load_op = ldr(alureg); // either ldar or ldbr
    out << "\t" << load_op;
    reg->emit(out);
    out << endl;
}

// takes a stack value and puts it in the aluregs
void emit_stack_fetch(ASMStack *stk, Register alureg, ostream &out)
{
    // Thank god I wrote this comment!
    // this is asymmetric w.r.t. the two ALU regs because the builtin
    // instruction which loads a value from memory only works for the
    // A register (ldmaa)
    
    if (alureg == B)
    {
	asmc("ldra %r12", "save the A register");
	stk->emit(out);
	asmc("ldmaa", "load the value from calculated address in stack frame");
	asmc("ldra %r13", "auxiliary");
	asmc("ldbr %r13", "transfer to B register");
	asmc("ldar %r12", "put original value back in A register");
    }
    else if (alureg == A)
    {
	stk->emit(out);
	asmc("ldmaa", "load the value from calculated address in stack frame");
    }
}

void emit_stack_store(ASMStack* stk,
		      Register alureg,
		      ostream& out)
{
    asmc("ldra %r12", "save the A register");
    asmc("ldrb %r13", "save the B register");

    stk->emit(out);		// stack addr will now be in the A register

    string out_reg = (alureg == A ? "%r12" : "%r13");

    asmc("ldmra " + out_reg, "load the value to the stack");

    asmc("ldar %r12", "restore old values");
    asmc("ldbr %r13", "restore old values");
}

// loads an operand (immediate, register, stack location) into B register
void emit_ldb_operand(ASMOperand* src, ostream& out)
{
    switch(src->type)
    {
    case IMMEDIATE:
	asmc("ldbi " + ((ASMImmediate*)src)->get_val(), "immediate"); // directly load into B reg
	break;
    case REGISTER:
	emit_register_fetch((ASMRegister*)src, B, out); // load operand into B
	break;
    case MEMORY:
	emit_stack_fetch((ASMStack*)src, B, out);  // load from stack into B reg
	break;
    }
}

// loads an operand (immediate, register, stack location) into A register
void emit_lda_operand(ASMOperand* src, ostream& out)
{
    switch(src->type)
    {
    case IMMEDIATE:
	src->emit(out);		// directly load into A reg
	break;
    case REGISTER:
	emit_register_fetch((ASMRegister*)src, A, out); // load operand into A
	break;
    case MEMORY:
	emit_stack_fetch((ASMStack*)src, A, out); // load from stack into A reg
	break;
    }
}

// stores B register into an operand (register or stack location) 
void emit_stb_operand(ASMOperand* dest, ostream& out)
{
    switch(dest->type)
    {
    case REGISTER:
	out << "\t" << "ldrb ";
	dest->emit(out);
	out << endl;
	
	break;
    case MEMORY:
	emit_stack_store((ASMStack*)dest, B, out);
	break;
    }
}

// stores A register into an operand (register or stack location) 
void emit_sta_operand(ASMOperand* dest, ostream& out)
{
    switch(dest->type)
    {
    case REGISTER:
	out << "\t" << "ldra ";
	dest->emit(out);
	out << endl;
	
	break;
    case MEMORY:
	emit_stack_store((ASMStack*)dest, A, out);
	break;
    }
}
