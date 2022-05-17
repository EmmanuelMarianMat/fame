#include<iostream>
#include <string>
#include<vector>
#include<unordered_map>
#include <algorithm> 

unordered_map<string,int> OpType = {
    {"OR",0},
    {"AND",1},
    {"ATTR",2},
    {"THRESHOLD",3},
    {"CONDITIONAL",4},
    {"NONE",5}
};

class BinNode{
    public:
        // string value;
        string node_type;
        string attribute;
        int index;
        bool negated;
        BinNode* left;
        BinNode* right;

        BinNode(string value, BinNode *leftn = NULL, BinNode *rightn = NULL){
            if(value=="and"||value=="AND"){
                node_type = "AND";
            }
            if(value=="or"||value=="OR"){
                node_type = "OR";
            }
            else{
                negated = false;
                index = -1; //None
                // int index= NULL;  
                if(value[0]=='!'){
                    value = value.substr(1, value.size());
                    negated = true;
                } 
                size_t pos = value.find('_');
                if(pos !=string::npos)
                {
                    string rest = value.substr(pos+1, value.size());
                    value = value.substr(0, pos);
                    pos = rest.find('_');
                    index = pos != string::npos ? stoi(rest.substr(0,pos)) : stoi(rest);
                }
                
                node_type = "ATTR";
                transform(value.begin(), value.end(), value.begin(), ::toupper);
                
                attribute = value;
            }
            
            left = leftn;
            right = rightn;
        }

        string getAttributeAndIndex(){
            string return_val = "";
            string attr = "ATTR";
            if(node_type==attr){
                if(negated)
                    return_val += '!';
                return_val += attribute;
                if(index != -1)
                    return_val += '_'+to_string(index);
            }
            return return_val;
        }

        string getNodeType(){
            return node_type;
        }
        
        BinNode* getLeft(){
            return left;
        }

        BinNode* getRight(){
            return right;
        }

        void addSubNode(BinNode *leftn, BinNode *rightn){
            left = leftn;
            right = rightn;
        }
};

class PolicyParser{
    public:
        bool verbose;
        PolicyParser(bool v = false){
            verbose = v;        
        }
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
			tokens.push_back("(");
		}
		else if(ch==')'){
			tokens.push_back(")");
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
				bool parenEnd = false;
				while( ((ch = line[i])!=')') && ch!=' ' && ch!= '\n' ){
					attr.append(1,ch);
					i++;
					if(line[i]==')'){
						parenEnd = true;
					}

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
				if(parenEnd)
					tokens.push_back(")");

				negated = false;
			}
		}
		else if(ch==' ')	;	//Don't do anything
		i++;
	}while(line[i]);
	return tokens;
}

BinNode *parseExp(vector<string>, int*);

BinNode *parseSubExp(vector<string> tokens, int *index){
	string token = tokens[*index];
	if(token=="("){
		(*index)++;
		BinNode *node = parseExp(tokens, index);
		if(tokens[*index]!=")"){
            cout<<"Expected )"<<endl;
            return NULL;
        }
		(*index)++;
		return node;
	}
	else{
		(*index)++;
		return new BinNode(token);
	}

}

BinNode *parseExp(vector<string> tokens, int *index){
	cout<<*index<< endl;
	BinNode *leftExp = parseSubExp(tokens, index);
	if(*index >= tokens.size()-1){
		return leftExp;
	}
	string token = tokens[*index];
	if (token == "AND" || token == "OR"){
		(*index)++;
        BinNode *rightExp = parseExp(tokens, index);
		cout<< leftExp->attribute << rightExp->attribute<<endl;
        return new BinNode(token, leftExp, rightExp);
	}
    else{
        cout<<"some issue"<<endl;
        return NULL;
    }
}

BinNode *parse(vector<string> tokens){
	int index = 0;
	return parseExp(tokens, &index);
}

void walkThrough(BinNode *root){
	if(root){
		cout<<root->getNodeType()<<" ";
		walkThrough(root->left);
		walkThrough(root->right);
	}
}

int main() {
	string line= "TWO";
	vector<string> tokens = get_tokens(line);
    BinNode *root = parse(tokens);
	walkThrough(root);
	cout<<endl;
	return 0;
}