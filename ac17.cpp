#include "mapping.h"
#include "class_definitions.h"
#include<vector>
#include<unordered_map>
#include<string>

class AC17{
    public:
        Pairing_module *group;
        int assump_size;
        MSP util;
        AC17(Pairing_module *group_obj, int assumpsz){
            group = group_obj;
            util = MSP();
            assump_size = assumpsz;
        }
        
        std::pair<std::unordered_map<const char*, std::vector<Element_class*>>, std::unordered_map<const char*, std::vector<Element_class*>>> setup(){
            std::vector<Element_class*> A, B, k, h_A, g_k, e_gh_kA;
            for(int i=0; i<assump_size; i++){
                A.push_back(group->Element_random(group, ZR));
                B.push_back(group->Element_random(group, ZR));
                k.push_back(group->Element_random(group, ZR));
            }
            k.push_back(group->Element_random(group, ZR));
            Element_class *g, *h, *e_gh;
            g = group->Element_random(group, G1);
            h = group->Element_random(group, G2);
            e_gh = group->Apply_pairing(g, h);

            for(int i=0; i<assump_size; i++){
                h_A.push_back(h->Element_pow(h, A[i]))
            }
            h_A.push_back(h);

            for(int i=0; i<assump_size+1; i++){
                g_k.push_back(g->Element_pow(g, k[i]))
            }
            
            for(int i=0; i<assump_size+1; i++){
                g_k.push_back(g->Element_pow(g, k[i]))
            }
            
            std::unordered_map<const char*, std::vector<Element_class*>> pk, msk;
            pk.insert('h_A', h_A);
            pk.insert('e_gh_kA', e_gh_kA); 

            std::vector<Element_class *> g_vect, h_vect;
            g_vect.push_back(g);
            h_vect.push_back(h);
            msk.insert('g', g_vect);
            msk.insert('h', h_vect);
            msk.insert('g_k', g_k);
            msk.insert('A', A);
            msk.insert('B', B);

            std::pair<std::unordered_map<const char*, std::vector<Element_class*>>, std::unordered_map<const char*, std::vector<Element_class*>>> ret;
            ret.first = pk;
            ret.second = msk;
            return ret;
        }

        std::pair<std::unordered_map<const char*, std::vector<Element_class *>>, std::unordered_map<const char*, std::vector<Element_class*>>> keygen(std::pair<std::unordered_map<const char*, std::vector<Element_class*>>, std::unordered_map<const char*, std::vector<Element_class*>>> pkmsk, std::vector<const char*> attr_list){
            std::vector<Element_class *> r, Br, K_0;
            Element_class *sum;
            sum->Element_set(0);
            for(int i=0; i<assump_size; i++){
                Element_class *rand = group->Element_random(group, ZR);
                r.push_back(rand);
                sum = sum->Element_add(sum, rand)
            }
            
            std::unordered_map<const char*, std::vector<Element_class*>> msk = pkmsk.second;
            for(int i=0; i<assump_size; i++){
                Element_class *elem = msk.at('B')[i];
                Br.push_back(elem->Element_mul(elem, r[i]));
            }
            Br.push_back(sum);

            for(int i=0; i<assump_size+1; i++){
                Element_class *elem = msk.at('h')[0];
                K_0.push_back(elem->Element_pow(elem, Br[i]));
            }

            std::unordered_map<const char*, std::vector<Element_class*>> K;
            std::vector<Element_class*> A, g;
            A = msk.at('A');
            g = msk.at('g')[0];
            for(auto &it: attr_list){
                std::vector<Element_class*> key;
                Element_class *sigma_attr = group->Element_random(group, ZR);
                for(int i=0; i<assump_size; i++){
                    Element_class *prod;
                    prod->Element_set(1);
                    Element_class *a = A[i];
                    for(int l=0; l<assump_size+1; l++){
                        char input_for_hash[30];
                        std::sprintf(input_for_hash, "%s%d%d", it, l, i);
                        prod = prod->Element_mul(prod, prod->Element_pow(group->Element_hash(group, input_for_hash, G1), Br[l]->Element_div(Br[l],a)));
                    }
                    prod = prod->Element_mul(prod, g->Element_pow(g, sigma_attr->Element_div(sigma_attr, a)));
                    key.push_back(prod);
                }
                key.push_back(g->Element_pow(g, sigma_attr->Element_invert(sigma_attr)));
                K.insert(it, key);
            }

            std::vector<Element_class*> Kp, g_k = msk.at("g_k");
            Element_class* sigma = group->Element_random(group, ZR);
            for(int i=0; i<assump_size; i++){
                Element_class *prod = g_k[i];
                Element_class *a = A[i];
                for(int l=0; l<assump_size+1; l++){
                    char input_for_hash[7];
                    std::sprintf(input_for_hash, "%s%d%d", "01", l, i);
                    prod = prod->Element_mul(prod, prod->Element_pow(group->Element_hash(group, input_for_hash, G1), Br[l]->Element_div(Br[l],a)));
                }
                prod = prod->Element_mul(prod, g->Element_pow(g, sigma->Element_div(sigma, a)));
                Kp.push_back(prod);
            }
            Kp.push_back(g->Element_mul(g_k[assump_size], g->Element_pow(g, sigma->Element_invert(sigma))));
            std::pair<std::unordered_map<const char*, std::vector<Element_class *>>, std::unordered_map<const char*, std::vector<Element_class*>>> ret; 
            ret.first.insert("K_0", K_0);
            ret.first.insert("Kp", Kp);
            ret.second = K;
            return ret;
        }

        std::pair<PolicyParser, std::pair<std::vector<Element_class *>, std::pair<std::unordered_map<const char*, std::vector<Element_class*>>, Element_class *>>> encrypt(std::unordered_map<const char*, std::vector<Element_class*>> pk, long int msg, const char *policy_str){
            PolicyParser policy = util.createPolicy(std::string(policy_str));
            std::unordered_map<std::string, vector<int>> mono_span_prog = util.convertPolicyToMSP(policy);
            int num_cols = util.len_longest_row;

            std::vector<Element_class *> s;
            Element_class *sum;
            sum->Element_set(0);
            for(int i=0; i<assump_size; i++){
                Element_class *rand = group->Element_random(group, ZR);
                s.push_back(rand);
                sum = sum->Element_add(sum, rand);
            }

            std::vector<Element_class *> C_0;
            std::vector<Element_class *> h_A = pk.at("h_A");
            for(int i=0; i<assump_size; i++){
                C_0.push_back(h_A[i]->Element_pow(h_A[i], s[i]));
            }
            C_0.push_back(h_A[assump_size]->Element_pow(h_A[assump_size], sum));
            std::vector<std::vector<std::vector<Element_class *>>> hash_table;
            for(int i=0; i<num_cols; i++){
                std::vector<std::vector<Element_class *>> x;
                char *input_for_hash1;
                std::sprintf(input_for_hash1, "0%d", i+1);
                for(int j=0; j<assump_size+1; j++){
                    std::vector<Element_class *> y;
                    char *input_for_hash2;
                    std::sprintf(input_for_hash2, "%s%d", input_for_hash1, j);
                    for(int k=0; k<assump_size; k++){
                        char *input_for_hash3;
                        std::sprintf(input_for_hash3, "%s%d", input_for_hash2, k);
                        Element_class *hashed_value = group->Element_hash(group, input_for_hash3, G1);
                        y.push_back(hashed_value);
                    }
                    x.push_back(y);
                }
                hash_table.push_back(x);
            }
            std::unordered_map<const char*, std::vector<Element_class*>> C;
            for(auto i: mono_span_prog){
                std::vector<Element_class *> ct;
                std::string attr_stripped = self.util.strip_index(i.first);
                for(int j=0; j<assump_size+1; j++){
                    Element_class *prod;
                    prod->Element_set(1);
                    int cols = i.second.size();
                    for(int k=0; k<assump_size+1; k++){
                        char input_for_hash[30];
                        std::sprintf(input_for_hash, "%s%d%d", it, l, i);
                        Element_class *prod1 = group->Element_hash(group, input_for_hash, G1);
                        for(int l=0; l<assump_size+1; l++){
                            prod1 = prod1->Element_mul(hash_table[l][j][k]->Element_pow(hash_table[l][j][k], i.second[j]));
                        }
                        prod = prod->Element_mul(prod, prod1->Element_pow(prod1, s[k]));
                    }
                    ct.push_back(prod);
                }
                C.insert(i.first, ct);
            }
            Element_class *Cp, *msg_elem;
            Cp->Element_set(1);
            for(int i=0; i<assump_size; i++){
                Cp = Cp->Element_mul(Cp, pk.at("e_gh_kA")[i]->Element_pow(pk.at("e_gh_kA")[i], s[i]));
            }
            msg_elem->Element_set(msg);
            Cp->Element_mul(Cp, msg_elem);

            std::pair<PolicyParser, std::pair<std::vector<Element_class *>, std::pair<std::unordered_map<const char*, std::vector<Element_class*>>, Element_class *>>> ret;
            ret.first = policy;
            ret.second.first = C_0;
            ret.second.second.first = C;
            ret.second.second.second = Cp;

            return ret;
        }

        void decrypt(std::unordered_map<const char*, std::vector<Element_class*>> pk, std::pair<PolicyParser, std::pair<std::vector<Element_class *>, std::pair<std::unordered_map<const char*, std::vector<Element_class*>>, Element_class *>>> ctxt, std::pair<std::unordered_map<const char*, std::vector<Element_class *>>, std::unordered_map<const char*, std::vector<Element_class*>>> key, std::vector<const char*> attr_list){
            vector<BinNode*> nodes = util.prune(ctxt.first = policy);
            if(nodes.size<1){
                std::cout << "Policy not satisfied" << std::endl;
                return NULL;
            }
            Element_class *prod1_GT;
            prod1_GT->Element_set(1);
            Element_class *prod2_GT;
            prod2_GT->Element_set(1);
            for(int i=0; i<assump_size+1; i++){
                Element_class *prod_H;
                prod_H->Element_set(1);
                Element_class *prod_G;
                prod_G->Element_set(1);
                for(auto node : nodes){
                    std::string attr = node->getAttributeAndIndex();
                    std::string attr_stripped = util.strip_index(attr);
                    prod_H = prod_H->Element_mul(prod_H, key.at('K')[attr_stripped][i]);
                    prod_G = prod_G->Element_mul(prod_G, ctxt).at('C').at(attr)[i];
                }
                prod1_GT = group->Apply_pairing(key.at("Kp")[i]->Element_mul(key.at("Kp")[i], prod_H), ctxt.at('C_0')[i]);
                prod2_GT = group->Apply_pairing(prod_G, key.at("K_0")[i]);
            }
            return ctxt.at("Cp")->Element_mul(ctxt.at("Cp"), prod2_GT->Element_div(prod2_GT, prod1_GT));
        }
} 