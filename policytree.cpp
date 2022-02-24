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
    string line= "((((ONE OR THREE) AND (FOUR AND FIVE)) OR ((TWO AND FOUR) AND (ONE OR TWO))) AND((ONE OR TWO) AND ((THREE OR FIVE) AND (TWO OR FOUR))))";
	PolicyParser parser = PolicyParser();
    BinNode *root = parser.parse(line);
    // walkThrough(root, "", true);
    MSP msp = MSP();
    BinNode *policy = msp.createPolicy(line);
    // walkThrough(policy, "", true);
    unordered_map<string, vector<int>> mono_span_prog = msp.convertPolicyToMSP(policy);
    int num_cols = msp.len_longest_row;
    // for(auto i: mono_span_prog){
    //     cout<<i.first<<endl;
    //     for(auto j: i.second)
    //         cout<<j;
    //     cout<<endl;
    // }
    vector<string> attrList = msp.getAttributeList(policy);
    // for(auto i: attrList)
    //     cout<<i<<endl;
    vector<string> attrList2{"ONE", "TWO", "THREE","FOUR"};
    vector<BinNode*> prunedPolicy = msp.prune(policy, attrList2);
    for(auto i: prunedPolicy)
        cout<<i->getAttribute()<<endl;
}