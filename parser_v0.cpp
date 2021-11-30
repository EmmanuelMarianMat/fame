#include <iostream>
#include <string>
#include <vector>

using namespace std;

class BinNode{
    public:
        // string value;
        string node_type;
        string attribute;
        int index;
        bool negated;
        BinNode* left;
        BinNode* right;
};

class vector<string> get_tokens(string line){
	bool negated = false;
	char ch;
	int i= 0, level= 0;
	string attr;
	// class BinNode *new_node = NULL;
	// struct BinNode_mem *mem_list = NULL;
	vector<string> tokens;
	// cout<<'\n';
	do{
		ch = line[i];
		if(ch=='('){
			tokens.push_back(to_string(ch));
		}
		else if(ch==')'){
			tokens.push_back(to_string(ch));
		}
		/*else if(ch=='!'){
			negated = true;
		}*/
		else if((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9') || ch=='!'){
			if((ch=='a' || ch=='A') && (line[i+1]=='n' || line[i+1]=='N') && (line[i+2]=='d' || line[i+2]=='D')){
					/*new_node = new struct BinNode;
					new_node->node_type = "AND";
					new_node->left = NULL;
					new_node->right = NULL;
					if(level==0){
						mem_list->prev= new struct BinNode_mem;
						mem_list->prev->next= mem_list;
						mem_list= mem_list->prev;
						mem_list->memory= new_node;
						mem_list->prev= NULL;
					}
					else(){
					}*/
					tokens.push_back("AND");
					i+=2;
					level++;
			}
			else if((ch=='o' || ch=='O') && (line[i+1]=='r' || line[i+1]=='R')){
					/*new_node = new struct BinNode;
					new_node->node_type = "OR";
					new_node->left = NULL;
					new_node->right = NULL;*/
					tokens.push_back("OR");
					i+=1;
					level++;
			}
			else{
				attr = ch;
				i++;
				while( ((ch = line[i])!=')') && ch!=' ' && ch!= '\n' ){
					attr.append(1,ch);
					i++;
				}
				/*new_node = new struct BinNode;
				new_node->node_type = "ATTR";
				new_node->attribute = attr;
				new_node->negated = negated;
				new_node->left = NULL;
				new_node->right = NULL;
				if(mem_list==NULL){
					mem_list= new struct BinNode_mem;
					mem_list->memory= new_node;
					mem_list->prev= NULL;
					mem_list->next= NULL;
				}
				else if(level){
					mem_list->next->memory= 
				}*/
				tokens.push_back(attr);

				negated = false;
			}
		}
		else if(ch==' ')	;	//Don't do anything
		i++;
	}while(line[i]);
	return tokens;
}

BinNode *parseSubExp(vector<string> tokens, int *index){
	string token = tokens[*index];
	if(token=="("){
		(*index)++;
		BinNode *node = parseExp(tokens, index);
		if(tokens[*index]!=")")
			cout<<"Expected )"<<endl;
		(*index)++;
		return node;
	}
	else{
		(*index)++;
		return new BinNode(token);
	}

}

BinNode *parseExp(vector<string> tokens, int *index){
	BinNode *leftExp = parseSubExp(tokens, index);
	if(*index >= tokens.size()){
		return leftExp;
	}

	string token = tokens[*index];
	if (token == "AND" || token == "OR"){
		(*index)++;

	}
}

BinNode *parse(vector<string> tokens){
	int index = 0;
	return parseExp(tokens, &index);
}

int main() {
	string line= "((!3ase23_23 and asdf34f^32 and ge45.34) or (234d*345 or !qwe@56) )";
	cout << line;
	vector<string> tokens = get_tokens(line);
	for(auto i : tokens){
		cout<< i<<endl;
	}
	return 0;
}
