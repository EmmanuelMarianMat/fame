// Include guard
#ifndef MSP_FUNCTIONS_H_
#define MSP_FUNCTIONS_H_

// User-defined header files
#include "class_definitions.h"

using namespace std;

void walkThrough(BinNode *root, string indent, bool last){
    if(root){
        if(root->getNodeType()=="AND"||root->getNodeType()=="OR")
            cout<<indent<<"+- "<<root->getNodeType()<<endl;
        else
            cout<<indent<<"+- "<<root->getNodeType()<<"("<<root->getAttributeAndIndex()<<")"<<endl;
    }
    indent += last ? "   " : "|  ";
    if(root->left && root->right){
        walkThrough(root->left, indent, false);
        walkThrough(root->right, indent, true);
    }
    else if(root->left)
        walkThrough(root->left, indent, true);
}

#endif