#pragma once

#include <vector>
#include <string>

using namespace std;

class ASMNode {};

class ASMInstruction : public ASMNode {};

class ASMFunction : public ASMNode
{
public:
    vector<ASMInstruction> body;

    ASMFunction(vector<ASMInstruction> _body) : body(_body) {}
};

class ASMProgram : public ASMNode
{
public:
    ASMFunction* entry;
    ASMProgram(ASMFunction* _entry) : entry(_entry) {}
};

class ASMOperand : public ASMNode {};

class ASMLoad : public ASMInstruction
{
public:
    ASMOperand* dest;
    ASMOperand* src;

    ASMLoad(ASMOperand* _dest, ASMOperand* _src)
	: dest(_dest),
	  src(_src)
    {}
};

class ASMAllocateStack : public ASMInstruction
{
public:
    int size;
    ASMAllocateStack(int _size) : size(_size) {}
};

class ASMReturn : public ASMInstruction
{
    
};

class ASMUnary : public ASMInstruction
{
public:
    ASMOperand* operand;
    ASMUnary(ASMOperand* _operand) : operand(_operand) {}
};


// bitwise not
class ASMNot : public ASMUnary 
{
public:
    ASMNot(ASMOperand* _op) : ASMUnary(_op) {}
};

// reverse the sign
class ASMNeg : public ASMUnary
{
public:
    ASMNeg(ASMOperand* _op) : ASMUnary(_op) {}
};
