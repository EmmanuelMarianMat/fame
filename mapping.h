#include <stdlib.h>
#include <gmp.h>
#include <pbc/pbc.h>
#include <iostream>
#include <math.h>
#include <string>
#include <cstring>
#include "base64.h"
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

#define IS_SAME_GROUP(a, b) \
	if(strncmp((const char *) a->pairing->hash_id, (const char *) b->pairing->hash_id, ID_LEN) != 0) {	\
		cout<<"mixing group elements from different curves.";	\
		return NULL;	\
	}

int hash_to_bytes(uint8_t *input_buf, int input_len, uint8_t *output_buf, int hash_len, uint8_t hash_prefix)
{
	SHA256_CTX sha2;
	const int new_input_len = input_len + 2; // extra byte for prefix
	uint8_t new_input[new_input_len];
	memset(new_input, 0, new_input_len);
	new_input[0] = (uint8_t)1; // block number (always 1 by default)
	new_input[1] = hash_prefix; // set hash prefix
	memcpy(new_input+2, input_buf, input_len); // copy input bytes

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

int check_membership(Element *elementObj) {
	int result = -1;
	element_t e;

	if(elementObj->element_type == ZR) {
		/* check value is between 1 and order */
		mpz_t zr;
		mpz_init(zr);
		element_to_mpz(zr, elementObj->e);
		int ans = mpz_cmp(zr, elementObj->pairing->pair_obj->Zr->order);
		result = ans <= 0 ? TRUE : FALSE;
		mpz_clear(zr);
	}
	/* for G1, G2, and GT test e^q == 1 (mod q)? */
	else if(elementObj->element_type == G1) {
		element_init_G1(e, elementObj->pairing->pair_obj);
		element_pow_mpz(e, elementObj->e, elementObj->pairing->pair_obj->G1->order);
//		element_printf("Elment->e => '%B'\n", e);
		result = element_is1(e) ? TRUE : FALSE; // TODO: verify this
		element_clear(e);
	}
	else if(elementObj->element_type == G2) {
		element_init_G2(e, elementObj->pairing->pair_obj);
		element_pow_mpz(e, elementObj->e, elementObj->pairing->pair_obj->G2->order);
//		element_printf("Elment->e => '%B'\n", e);
		result = element_is1(e) ? TRUE : FALSE; // TODO: verify this
		element_clear(e);
	}
	else if(elementObj->element_type == GT) {
		element_init_GT(e, elementObj->pairing->pair_obj);
		element_pow_mpz(e, elementObj->e, elementObj->pairing->pair_obj->GT->order);
//		element_printf("Elment->e => '%B'\n", e);
		result = element_is1(e) ? TRUE : FALSE; // TODO: verify this
		element_clear(e);
	}
	else {/* not a valid type */ }
	return result;
}

class Pairing_module {
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

	Element *Apply_pairing(Element *lhs, Element *rhs, Pairing *group = NULL){
		Element *newObject;
		IS_SAME_GROUP(lhs, rhs);
		if(pairing_is_symmetric(lhs->pairing->pair_obj)) {
			newObject = createNewElement(GT, lhs->pairing);
			pairing_apply(newObject->e, lhs->e, rhs->e, rhs->pairing->pair_obj);
			return newObject;
		}
		if(lhs->element_type == rhs->element_type){
			if(lhs->element_type == G1){
				cout<<"Both elements are of type G1 in asymmetric pairing";
				return NULL;
			}
			if(lhs->element_type == G2){
				cout<<"Both elements are of type G2 in asymmetric pairing";
				return NULL;
			}
			cout<<"Unexpected elements type in asymmetric pairing product";
			return NULL;
		}
		newObject = createNewElement(GT, lhs->pairing);
		if(lhs->element_type == G1)
			pairing_apply(newObject->e, lhs->e, rhs->e, rhs->pairing->pair_obj);
		else if(lhs->element_type == G2)
			pairing_apply(newObject->e, rhs->e, lhs->e, rhs->pairing->pair_obj);
		return newObject;
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
		return newObject;
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

	static const char* Serialize_cmp(Element *element = NULL, int compression=1){
		if(element->elem_initialized == FALSE) {
			cout<<"Element not initialized.";
			return NULL;
		}

		int elem_len = 0;
		uint8_t *data_buf = NULL;
		size_t bytes_written;
		if(element->element_type == ZR || element->element_type == GT) {
			elem_len = element_length_in_bytes(element->e);
			data_buf = (uint8_t *) malloc(elem_len + 1);
			if(data_buf == NULL){
				cout<<"heap full";
				return NULL;
			}
			// write to char buffer
			bytes_written = element_to_bytes(data_buf, element->e);
		}
		else if(element->element_type != NONE_G) {
		// object initialized now retrieve element and serialize to a char buffer.
			if(compression){
				elem_len = element_length_in_bytes_compressed(element->e);
			}else{
				elem_len = element_length_in_bytes(element->e);
			}
			data_buf = (uint8_t *) malloc(elem_len + 1);
			if(data_buf == NULL){
				cout<<"heap full";
				return NULL;
			}
			// write to char buffer
			if(compression)
				bytes_written = element_to_bytes_compressed(data_buf, element->e);
			else
				bytes_written = element_to_bytes(data_buf, element->e);
		}
		else {
			cout<<"Invalid element type.";
			return NULL;
		}

		// convert to base64 and return as a string?
		size_t length = 0;
		char *base64_data_buf = NewBase64Encode(data_buf, bytes_written, FALSE, &length);
		//PyObject *result = PyUnicode_FromFormat("%d:%s", element->element_type, (const char *) base64_data_buf);
		// free(base64_data_buf);
		char *result;
		int i = sprintf(result, "%d:%s", element->element_type, (const char *) base64_data_buf);
		free(base64_data_buf);
		free(data_buf);
		return result;
	}

	static Element *Deserialize_cmp(char *object, Pairing *group = NULL, int compression = 1) {
		Element *origObject = NULL;

		uint8_t *serial_buf = (uint8_t *)object;
		int type = atoi((const char *) &(serial_buf[0]));
		uint8_t *base64_buf = (uint8_t *)(serial_buf + 2);

		size_t deserialized_len = 0;
		uint8_t *binary_buf = (uint8_t*)NewBase64Decode((const char *) base64_buf, strlen((char *) base64_buf), &deserialized_len);

		if((type == ZR || type == GT) && deserialized_len > 0) {
		//				debug("result => ");
		//				printf_buffer_as_hex(binary_buf, deserialized_len);
			origObject = createNewElement((GroupType)type, group);
			element_from_bytes(origObject->e, binary_buf);
			free(binary_buf);
			return origObject;
		}
		else if((type == G1 || type == G2) && deserialized_len > 0) {
			// now convert element back to an element type (assume of type ZR for now)
			origObject = createNewElement((GroupType)type, group);
			if(compression) {
				element_from_bytes_compressed(origObject->e, binary_buf);
			} else {
				element_from_bytes(origObject->e, binary_buf);
			}
			free(binary_buf);
			return origObject;
		}

		cout<<"Nothing to deserialize in element.";
		return NULL;
	}

	static bool Group_Check(Pairing *group = NULL, Element *object = NULL) {
		if(check_membership(object) == TRUE) {
			return true;
		}
		else {
			return false;
		}
	}

	static long Get_Order(Pairing *group = NULL) {
		long object;
		mpz_set_si(group->pair_obj->r, object);
		return object; /* returns a PyInt */
	}
};

int main(){
    return 0;
}