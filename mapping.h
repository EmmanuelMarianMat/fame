#ifndef MAPPING_H
#define MAPPING_H
#include <stdlib.h>
#include <gmp.h>
#include <pbc/pbc.h>
#include <iostream>
#include <math.h>
#include <base64.h>
#include <openssl/objects.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include "element_class.h"
#include "pairing_module.h"
#include "enum.h"

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

int hash_to_bytes(uint8_t *input_buf, int input_len, uint8_t *output_buf, int hash_len, uint8_t hash_prefix);

int hash_element_to_bytes(element_t *element, int hash_size, uint8_t* output_buf, int prefix);

char *convert_buffer_to_hex(uint8_t * data, size_t len);
// assumes that pairing structure has been initialized
class Element_class *createNewElement(enum Group element_type, class Pairing_module *pairing);

int check_membership(class Element_class *elementObj);
#endif