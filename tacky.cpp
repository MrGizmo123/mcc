#include "tacky.hpp"

using namespace std;

// used to generate unique labels
int label_count = 0;

string uniq_label()
{
    string result = "label" + to_string(label_count);
    label_count++;
    return result;
}
