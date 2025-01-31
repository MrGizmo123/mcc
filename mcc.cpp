#include <iostream>
#include <string>
#include <unordered_map>

#include "parser.hpp"
#include "tokenizer.h"
#include "CLI11.hpp"
#include "mcc.hpp"

using namespace std;

vector<string> lines_of_code;
string code;
string infile;

CodeContext get_code_context(token tok, int number_of_lines)
{
    return CodeContext(tok.line_number, lines_of_code[tok.line_number-1]);
}

string get_line_of_code(int line)
{
    return lines_of_code[line-1];
}

string file_name()
{
    return infile;
}

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
	out
	<< "(" << t.type
	<< " : '" << t.val
	<< "', start: " << t.start_index
	<< ", end: " << t.end_index
	<< ", line: " << t.line_number
	<< ")" << ", " << endl;
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

// taken from https://stackoverflow.com/questions/13172158/c-split-string-by-line
vector<string> split_string_by_newline(string str)
{
    auto result = vector<string>{};
    auto ss = stringstream{str};

    for (string line; getline(ss, line, '\n');)
        result.push_back(line);

    return result;
}

int main(int argc, char** argv)
{
    CLI::App app{"mcc - a small simple C compiler for the Mentat PCB computer"};
    app.add_option("-i,--input", infile, "The file to be compiled");
    CLI11_PARSE(app, argc, argv);

    code = read_file(infile);
    lines_of_code = split_string_by_newline(code);
    
    
    deque<token> tokens = tokenize(code);
    //cout << "Tok: " << tokens << endl;

    Program* p = parse(tokens);
    p->pretty_print(cout);

    cout << "---------------------------------------------------" << endl;

    map<string, identifier> var_map;
    p->resolve_identifiers(var_map);

    p->label_loops("nil");

    map<string, symbol> symbol_table;
    p->do_type_checking(symbol_table);
    p->pretty_print(cout);
    
    cout << "---------------------------------------------------" << endl;

    vector<IRNode*> tacky;
    IRProgram* ir_prog = (IRProgram*)p->emit(tacky);

    ir_prog->pretty_print(cout);

    // cout << "---------------------------------------------------" << endl;

    // vector<ASMNode*> assembly;
    // ir_prog->emit(assembly);

    // assembly[0]->pretty_print(cout);

    // cout << "---------------------------------------------------" << endl;

    // unordered_map<string, int> temps;
    // assembly[0]->legalize(temps);
    // assembly[0]->pretty_print(cout);

    // cout << "---------------------------------------------------" << endl;

    // assembly[0]->emit(cout);

    
}
