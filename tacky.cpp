#include "tacky.hpp"

#include <map>
#include <vector>

using namespace std;

// map of function names and the functions they provide
// some provide more than one function
map<string, vector<string>> intrinsic_functions = {
    {"__display", {"__f_div", "__clear_display"}},
    {"__input", {}},
    {"__clear_display", {}},
    {"__init_display", {}},
    {"__halt", {}},
    {"__f_div", {}},
    {"__f_mul", {}}
};

bool is_function_intrinsic(string name)
{
    return intrinsic_functions.find(name) != intrinsic_functions.end();
}

vector<string> get_intrinsic_dependencies(string name)
{
    return intrinsic_functions[name];
}
