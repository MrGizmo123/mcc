#include <iostream>
#include <string>
#include <unordered_map>

#include "parser.hpp"
#include "tokenizer.h"
#include "CLI11.hpp"

using namespace std;

ostream& operator<<(ostream& out, deque<token>& v)
{
    if (v.size() == 0)
    {
	out << "[]" << endl;
	return out;
    }

    out << "[";
    for (token t : v)
    {
	out << "(" << t.type << " : '" << t.val << "')" << ", ";
    }

    out << "\b\b]";
    return out;
}

// taken from https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
string read_file(string filename)
{
    std::ifstream t(filename);
    std::stringstream buffer;
    buffer << t.rdbuf();

    return buffer.str();
}

int main(int argc, char** argv)
{
    CLI::App app{"mcc - a small simple c compiler for the mentat PCB computer"};
    string infile;
    app.add_option("-i,--input", infile, "The file to be compiled");
    CLI11_PARSE(app, argc, argv);

    string code = read_file(infile);
    
    deque<token> tokens = tokenize(code);
    cout << "Tok: " << tokens << endl;

    Program* p = parse(tokens);
    p->pretty_print(cout);

    cout << "---------------------------------------------------" << endl;

    map<string, variable_label> var_map;
    p->resolve_variables(var_map);
    p->pretty_print(cout);
    
    cout << "---------------------------------------------------" << endl;

    vector<IRNode*> tacky;
    IRProgram* ir_prog = (IRProgram*)p->emit(tacky);

    ir_prog->pretty_print(cout);

    cout << "---------------------------------------------------" << endl;

    vector<ASMNode*> assembly;
    ir_prog->emit(assembly);

    assembly[0]->pretty_print(cout);

    cout << "---------------------------------------------------" << endl;

    unordered_map<string, int> temps;
    assembly[0]->legalize(temps);
    assembly[0]->pretty_print(cout);

    cout << "---------------------------------------------------" << endl;

    assembly[0]->emit(cout);

    
}
