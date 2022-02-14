// C header files
#include <string>
#include <unordered_map>

// User-defined header files
#include "class_definitions.h"
#include "msp_functions.h"

using namespace std;

// unordered_map<string,int> OpType = {
//     {"OR",0},
//     {"AND",1},
//     {"ATTR",2},
//     {"THRESHOLD",3},
//     {"CONDITIONAL",4},
//     {"NONE",5}
// };

int main(){
    string line= "((ONE and THREE) and (TWO OR FOUR))";
	PolicyParser parser = PolicyParser();
    BinNode *root = parser.parse(line);
    // walkThrough(root, "", true);
    MSP msp = MSP();
    BinNode *policy = msp.createPolicy(line);
    unordered_map<string, vector<int>> mono_span_prog = msp.convertPolicyToMSP(policy);
    int num_cols = msp.len_longest_row;
    // for(auto i: mono_span_prog){
    //     cout<<i.first<<endl;
    //     for(auto j: i.second)
    //         cout<<j;
    //     cout<<endl;
    // }
    vector<string> attrList = msp.getAttributeList(policy);
    vector<BinNode*> prunedPolicy = msp.prune(policy, attrList);
    for(auto i: prunedPolicy)
        cout<<i->getAttributeAndIndex()<<endl;
}