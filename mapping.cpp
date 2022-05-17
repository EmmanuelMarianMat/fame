#include "mapping.h"
using namespace std;

int hash_to_bytes(uint8_t *input_buf, int input_len, uint8_t *output_buf, int hash_len, uint8_t hash_prefix) {
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

int hash_element_to_bytes(element_t *element, int hash_size, uint8_t* output_buf, int prefix) {
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

char *convert_buffer_to_hex(uint8_t * data, size_t len) {
	size_t i;
	char *tmp = (char *) malloc(len*2 + 2);
	char *tmp2 = tmp;
	memset(tmp, 0, len*2+1);

	for(i = 0; i < len; i++)
		tmp += sprintf(tmp, "%02x", data[i]);

	return tmp2;
}

// assumes that pairing structure has been initialized
Element_class *createNewElement(enum Group element_type, Pairing_module *pairing) {
	Element_class *retObject;
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

int check_membership(Element_class *elementObj) {
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
	else
		cout<<"not a valid type";

	return result;
}
