#include<iostream>
#include <string>
#include<vector>
#include<unordered_map>
#include <algorithm> 
using namespace std;

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
            else if(value=="or"||value=="OR"){
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

        vector<string> get_tokens(string line){
            bool negated = false;
            char ch;
            int i= 0, level= 0;
            string attr;
            vector<string> tokens;
            do{
                ch = line[i];
                if(ch=='('){
                    tokens.push_back("(");
                }
                else if(ch==')'){
                    tokens.push_back(")");
                }
                else if((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9') || ch=='!'){
                    if((ch=='a' || ch=='A') && (line[i+1]=='n' || line[i+1]=='N') && (line[i+2]=='d' || line[i+2]=='D')){
                            tokens.push_back("AND");
                            i+=2;
                            level++;
                    }
                    else if((ch=='o' || ch=='O') && (line[i+1]=='r' || line[i+1]=='R')){
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
                        tokens.push_back(attr);
                        if(parenEnd)
                            tokens.push_back(")");

                        negated = false;
                    }
                }
                else if(ch==' ')	;
                i++;
            }while(line[i]);
            return tokens;
        }

        BinNode* S(int* index, vector<string> tokens){
            cout<<"S"<<*index<<endl;
            BinNode *left = T(index, tokens);
            if(*index < tokens.size() && tokens[*index]=="OR"){
                (*index)++;
                BinNode *right = S(index, tokens);
                return new BinNode("OR", left, right);
            }

            return left;
        }

        BinNode* T(int* index, vector<string> tokens){
            cout<<"T"<<*index<<endl;
            BinNode *left = F(index, tokens);
            if(*index < tokens.size() && tokens[*index]=="AND"){
                (*index)++;
                BinNode *right = T(index, tokens);
                return new BinNode("AND", left, right);
            }

            return left;
        }

        BinNode* F(int* index, vector<string> tokens){
            cout<<"F"<<*index<<endl;
            string token = tokens[*index];
            (*index)++;
            BinNode *node;
            if(token=="("){
                node = S(index, tokens);
                (*index)++;
            }
            else{
                node = new BinNode(token);
            }
            return node;
        }

        BinNode* parse(string line){
            /*
            
                S → T '+' S | T
                T → F '.' T | F
                F → A | '('S')'
                A → 'a' | 'b' | 'c' | ... | ɛ
            */
            vector<string> tokens = get_tokens(line);
            int index = 0;
            return S(&index, tokens);
        }
};

void walkThrough(BinNode *root){
	if(root){
		cout<<root->getNodeType()<<" ";
		walkThrough(root->left);
		walkThrough(root->right);
	}
}

int main(){
    string line= "(ONE or TWO) or (!TWO or THREE))";
	PolicyParser parser = PolicyParser();
    BinNode *root = parser.parse(line);
    walkThrough(root);
}