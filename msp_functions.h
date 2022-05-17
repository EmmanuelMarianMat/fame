// Include guard
#ifndef MSP_FUNCTIONS_H_
#define MSP_FUNCTIONS_H_

// User-defined header files
#include "class_definitions.h"

void walkThrough(BinNode *root, string indent, bool last){
    if(root){
        if(root->getNodeType()=="AND"||root->getNodeType()=="OR")
            cout<<indent<<"+- "<<root->getNodeType()<<endl;
        else
            cout<<indent<<"+- "<<root->getNodeType()<<"("<<root->getAttributeAndIndex()<<")"<<endl;
    }
    indent += last ? "   " : "|  ";
    if(root->getLeft() && root->getRight()){
        walkThrough(root->getLeft(), indent, false);
        walkThrough(root->getRight(), indent, true);
    }
    else if(root->getLeft())
        walkThrough(root->getLeft(), indent, true);
}

#endif