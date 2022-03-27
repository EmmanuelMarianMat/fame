#include <stdlib.h>
#include <gmp.h>
#include <pbc/pbc.h>
#include <iostream>
#include <math.h>
#include <string>
#include <cstring>
#include <openssl/objects.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
using namespace std;

#define BYTE		8
#define TRUE		1
#define FALSE		0
#define ID_LEN   	8
#define HASH_LEN	SHA256_DIGEST_LENGTH

/* Index numbers for different hash functions.  These are all implemented as SHA1(index || message).	*/
#define HASH_FUNCTION_ELEMENTS			0
#define HASH_FUNCTION_STR_TO_Zr_CRH		1
#define HASH_FUNCTION_Zr_TO_G1_ROM		2
#define HASH_FUNCTION_STRINGS			3

enum Group {ZR = 0, G1, G2, GT, NONE_G};
typedef enum Group GroupType;

typedef struct {
	pbc_param_t p;
	char *params;
	char *param_buf;
	pairing_t pair_obj;
	int group_init;
	uint8_t hash_id[ID_LEN+1];
} Pairing;

typedef struct {
  Pairing *pairing;
  element_t e;
  GroupType element_type;
  int elem_initialized;
  element_pp_t e_pp;
  int elem_initPP;
} Element;

int hash_to_bytes(uint8_t *input_buf, int input_len, uint8_t *output_buf, int hash_len, uint8_t hash_prefix)
{
	SHA256_CTX sha2;
	const int new_input_len = input_len + 2; // extra byte for prefix
	uint8_t new_input[new_input_len];
	//	printf("orig input => \n");
	//	printf_buffer_as_hex(input_buf, input_len);
	memset(new_input, 0, new_input_len);
	new_input[0] = (uint8_t)1; // block number (always 1 by default)
	new_input[1] = hash_prefix; // set hash prefix
	memcpy(new_input+2, input_buf, input_len); // copy input bytes

	//	printf("new input => \n");
	//	printf_buffer_as_hex(new_input, new_input_len);
	// prepare output buf
	memset(output_buf, 0, hash_len);

	if (hash_len <= HASH_LEN) {
		SHA256_Init(&sha2);
		SHA256_Update(&sha2, new_input, new_input_len);
		uint8_t md[HASH_LEN];
		SHA256_Final(md, &sha2);
		memcpy(output_buf, md, hash_len);
	}
	else {
		// apply variable-size hash technique to get desired size
		// determine block count.
		int blocks = (int) ceil(((double) hash_len) / HASH_LEN);
		uint8_t md2[(blocks * HASH_LEN)];
		for(int i = 0; i < blocks; i++) {
			/* compute digest = SHA-2( i || prefix || input_buf ) || ... || SHA-2( n-1 || prefix || input_buf ) */
			uint8_t md[HASH_LEN];
			new_input[0] = (uint8_t)(i+1);
			SHA256_Init(&sha2);
			int size = new_input_len;
			SHA256_Update(&sha2, new_input, size);
			SHA256_Final(md, &sha2);
			memcpy(md2 +(i * HASH_LEN), md, HASH_LEN);
		}

		// copy back to caller
		memcpy(output_buf, md2, hash_len);
	}
	OPENSSL_cleanse(&sha2,sizeof(sha2));
	return TRUE;
}

int hash_element_to_bytes(element_t *element, int hash_size, uint8_t* output_buf, int prefix)
{
	unsigned int buf_len;
	
	buf_len = element_length_in_bytes(*element);
	uint8_t *temp_buf = (uint8_t *)malloc(buf_len+1);
	if (temp_buf == NULL)
		return FALSE;
	
	element_to_bytes(temp_buf, *element);
	if(prefix == 0)
		prefix = HASH_FUNCTION_ELEMENTS;
	else if(prefix < 0)
		// convert into a positive number
		prefix *= -1;
	int result = hash_to_bytes(temp_buf, buf_len, output_buf, hash_size, prefix);
	free(temp_buf);
	
	return result;
}

char *convert_buffer_to_hex(uint8_t * data, size_t len)
{
	size_t i;
	char *tmp = (char *) malloc(len*2 + 2);
	char *tmp2 = tmp;
	memset(tmp, 0, len*2+1);

	for(i = 0; i < len; i++)
	tmp += sprintf(tmp, "%02x", data[i]);

	return tmp2;
}

class Pairing_module {
    // assumes that pairing structure has been initialized
    static Element *createNewElement(GroupType element_type, Pairing *pairing) {
	    Element *retObject;
	    if(element_type == ZR) {
		    element_init_Zr(retObject->e, pairing->pair_obj);
		    retObject->element_type = ZR;
	    }
	    else if(element_type == G1) {
		    element_init_G1(retObject->e, pairing->pair_obj);
		    retObject->element_type = G1;
	    }
	    else if(element_type == G2) {
		    element_init_G2(retObject->e, pairing->pair_obj);
		    retObject->element_type = G2;
	    }
	    else if(element_type == GT) {
		    element_init_GT(retObject->e, pairing->pair_obj);
		    retObject->element_type = GT;
	    }
	
	    retObject->elem_initialized = TRUE;
	    retObject->elem_initPP = FALSE;
	    retObject->pairing = pairing;
	    return retObject;	
    }

    static Element *Element_elem(Pairing *group, int type,unsigned long long_obj=LONG_MAX){
	    Element *retObject = NULL;

	    if(type >= ZR && type <= GT) {
		    retObject = createNewElement((GroupType)type, group);
	    }
        else{
    		cout<<"unrecognized group type.";
            return NULL;
        }
        if(long_obj!=LONG_MAX){
		    mpz_t m;
		    mpz_init(m);
		    element_set_mpz(retObject->e, m);
		    mpz_clear(m);
			mpz_set_ui (m, long_obj);
			element_set_mpz(retObject->e, m);
			mpz_clear(m);
        }

		return retObject;
    }

    static Element *Element_random(Pairing *group, int arg1, int seed=-1){
        Element *retObject;
    	int e_type = -1;
    	if(arg1 == ZR) {
    		element_init_Zr(retObject->e, group->pair_obj);
    		e_type = ZR;
    	}
    	else if(arg1 == G1) {
    		element_init_G1(retObject->e, group->pair_obj);
    		e_type = G1;
    	}
    	else if(arg1 == G2) {
    		element_init_G2(retObject->e, group->pair_obj);
    		e_type = G2;
    	}
    	else if(arg1 == GT) {
    		cout<<"cannot generate random elements in GT.";
            return NULL;
    	}
    	else {
    		cout<<"unrecognized group type.";
            return NULL;
    	}

    	if(seed > -1) {
    		pbc_random_set_deterministic((uint32_t) seed);
    	}
    	element_random(retObject->e);
    	retObject->elem_initialized = TRUE;
    	retObject->elem_initPP = FALSE;
    	retObject->element_type = (GroupType)e_type;
    	/* set the group object for element operations */
    	retObject->pairing = group;
        return retObject;
    }

	string sha2_hash(Element *object, int label){
		char* hash_hex = NULL;
		if(object->elem_initialized == FALSE){
			cout<<"null element object";
			return NULL;
		}
		int hash_size = HASH_LEN;
		uint8_t hash_buf[hash_size + 1];
		if(!hash_element_to_bytes(&object->e, hash_size, hash_buf, label)) {
			cout<<"failed to hash element";
			return NULL;
		}
		hash_hex = convert_buffer_to_hex(hash_buf, hash_size);
		
		string str(hash_hex);
		free(hash_hex);
		return str;
	}

	// The hash function should be able to handle elements of various types and accept
	// a field to hash too. For example, a string can be hashed to Zr or G1, an element in G1 can be
	static Element *Element_hash(Pairing *group, string objList, GroupType type) {
		Element *newObject = NULL, *object = NULL;
		int result, i;
		 
		char *tmp = NULL;

		int hash_len = mpz_sizeinbase(group->pair_obj->r, 2) / BYTE;
		uint8_t hash_buf[hash_len];
		memset(hash_buf, 0, hash_len);

		const char* str = objList.c_str();

		if(type == ZR) {
			// create an element of Zr
			// hash bytes using SHA1
			int str_length = (int) strlen(str);
			result = hash_to_bytes((uint8_t *) str, str_length, hash_buf, hash_len, HASH_FUNCTION_STR_TO_Zr_CRH);
			// extract element in hash
			if(!result) { 
				cout<<"could not hash to bytes.";
				return NULL; 
			}
			newObject = createNewElement(ZR, group);
			element_from_hash(newObject->e, hash_buf, hash_len);
		}
		else if(type == G1 || type == G2) { // depending on the curve hashing to G2 might not be supported
		    // element to G1	
			// hash bytes using SHA1
			int str_length = (int) strlen(str);
			result = hash_to_bytes((uint8_t *) str, str_length, hash_buf, hash_len, HASH_FUNCTION_Zr_TO_G1_ROM);
			if(!result) { 
				cout<<"could not hash to bytes.";
				return NULL; 
			}			
			newObject = createNewElement(type, group);
			element_from_hash(newObject->e, hash_buf, hash_len);
		}
		else {
			cout<<"cannot hash a string to that field. Only Zr or G1.";
			return NULL;
		}
	}
};

int main(){
    return 0;
}