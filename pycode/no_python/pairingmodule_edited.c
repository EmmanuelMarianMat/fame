/*
 * Charm-Crypto is a framework for rapidly prototyping cryptosystems.
 *
 * Charm-Crypto is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Charm-Crypto is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Charm-Crypto. If not, see <http://www.gnu.org/licenses/>.
 *
 * Please contact the charm-crypto dev team at support@charm-crypto.com
 * for any questions.
 */

/*
 *   @file    pairingmodule.c
 *
 *   @brief   charm interface over PBC library
 *
 *   @author  ayo.akinyele@charm-crypto.com
 *
 ************************************************************************/

#include "pairingmodule.h"

int exp_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == ZR && rhs == ZR) return TRUE;
	if(lhs == G1 && rhs == ZR) return TRUE;
	if(lhs == G2 && rhs == ZR) return TRUE;
	if(lhs == GT && rhs == ZR) return TRUE;
	return FALSE; /* Fail all other cases */
}

int mul_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == rhs) return TRUE;
	if(lhs == ZR || rhs == ZR) return TRUE;
	return FALSE; /* Fail all other cases */
}

int add_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == rhs && lhs != GT) return TRUE;
	return FALSE; /* Fail all other cases */
}

int sub_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == rhs && lhs != GT) return TRUE;
	return FALSE; /* Fail all other cases */
}

int div_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == rhs) return TRUE;
	return FALSE; /* Fail all other cases */
}

int pair_rule(GroupType lhs, GroupType rhs)
{
	if(lhs == G1 && rhs == G2) return TRUE;
	else if(lhs == G2 && rhs == G1) return TRUE;
	return FALSE; /* Fall all other cases: only for MNT case */
}

int check_type(GroupType type) {
	if(type == ZR || type == G1 || type == G2 || type == GT) return TRUE;
	return FALSE;
}

#define ERROR_TYPE(operand, ...) "unsupported "#operand" operand types: "#__VA_ARGS__

#define UNARY(f, m, n) \
// TODO: static PyObject *f(PyObject *v) 

#define BINARY(f, m, n) \
// TODO: static PyObject *f(PyObject *v, PyObject *w) 

// TODO: update these two functions to convert neg numbers (original)
// TODO: PyObject *mpzToLongObj (mpz_t m)

// TODO: void longObjToMPZ (mpz_t m, PyLongObject * p)

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

void printf_buffer_as_hex(uint8_t * data, size_t len)
{
#ifdef DEBUG
	size_t i;

	for (i = 0; i < len; i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");
#endif
}

// simply checks that the elements satisfy the properties for the given
// binary operation. Whitelist approach: only return TRUE for valid cases, otherwise FALSE
int Check_Types(GroupType l_type, GroupType r_type, char op)
{	
	switch (op) {
		// Rules: elements must be of the same type, multiplicative operations should be only used for
		// elements in field GT
		case 'a':	
			if(l_type == GT || r_type == GT) { return FALSE; }
			break;
		case 's':
			if(l_type == GT || r_type == GT) { return FALSE; }			
			break;
		case 'e':
			if(l_type != G1 && r_type != G2) { return FALSE; }
			break;
		case 'p':
			// rule for exponentiation for types
			if(l_type != G1 && l_type != G2 && l_type != GT && l_type != ZR) { return FALSE; }
			break;
		default:
			break;
	}
	
	return TRUE;
	
}

// assumes that pairing structure has been initialized
// TODO: static Element *createNewElement(GroupType element_type, Pairing *pairing) 

// TODO: Element *convertToZR(PyObject *longObj, PyObject *elemObj)

// TODO: void 	Pairing_dealloc(Pairing *self)

// TODO: void	Element_dealloc(Element* self)

// helper method 
ssize_t read_file(FILE *f, char** out) 
{
	if(f != NULL) {
		/* See how big the file is */
		fseek(f, 0L, SEEK_END);
		ssize_t out_len = ftell(f);
		debug("out_len: %zd\n", out_len);
		if(out_len <= MAX_LEN) {
			/* allocate that amount of memory only */
			if((*out = (char *) malloc(out_len+1)) != NULL) {
				fseek(f, 0L, SEEK_SET);
				if(fread(*out, sizeof(char), out_len, f) > 0)
				    return out_len;
				else
				    return -1;
			}
		}
	}

	return 0;
}

char * init_pbc_param(char *file, pairing_t *pairing)
{
	pbc_param_t params;
	FILE *fp;
	size_t count;
	char *buf = NULL;
	fp = fopen(file, "r");
	
	if(fp == NULL) {
		fprintf(stderr, "Error reading file!\n");
		return NULL;
	}
	
	debug("Reading '%s'\n", file);
	count = read_file(fp, &buf);
	debug("param='%s'\n", buf);
	fclose(fp);	

	if(pbc_param_init_set_buf(params, buf, count) == 0) {
		/* initialize the pairing_t struct with params */
		pairing_init_pbc_param(*pairing, params);
		debug("Pairing init!\n");
	}
	else {
		printf("Error: could not init pbc_param_t.\n");
		return NULL;
	}
	
	return buf;
}

/*!
 * Hash a null-terminated string to a byte array.
 *
 * @param input_buf		The input buffer.
 * @param input_len		The input buffer length (in bytes).
 * @param output_buf	A pre-allocated output buffer of size hash_len.
 * @param hash_len		Length of the output hash (in bytes). Should be approximately bit size of curve group order.
 * @param hash_prefix	prefix for hash function.
 */
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


/*!
 * Hash a group element to a byte array.  This calls hash_to_bytes().
 *
 * @param element		The input element.
 * @param hash_len		Length of the output hash (in bytes).
 * @param output_buf	A pre-allocated output buffer.
 * @param hash_num		Index number of the hash function to use (changes the output).
 * @return				FENC_ERROR_NONE or an error code.
 */

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

// take a previous hash and concatenate with serialized bytes of element and hashes into output buf
int hash2_element_to_bytes(element_t *element, uint8_t* last_buf, int hash_size, uint8_t* output_buf) {
	// assume last buf contains a hash
	unsigned int last_buflen = hash_size;
	unsigned int buf_len = element_length_in_bytes(*element);

	uint8_t* temp_buf = (uint8_t *) malloc(buf_len + 1);
	memset(temp_buf, '\0', buf_len);
	if(temp_buf == NULL) {
		return FALSE;
	}

	element_to_bytes((unsigned char *) temp_buf, *element);
	// create output buffer
	uint8_t* temp2_buf = (uint8_t *) malloc(last_buflen + buf_len + 1);
	memset(temp2_buf, 0, (last_buflen + buf_len));
	int i;
	for(i = 0; i < last_buflen; i++)
		temp2_buf[i] = last_buf[i];

	int j = 0;
	for(i = last_buflen; i < (last_buflen + buf_len); i++)
	{
		temp2_buf[i] = temp_buf[j];
		j++;
	}
	// hash the temp2_buf to bytes
	int result = hash_to_bytes(temp2_buf, (last_buflen + buf_len), output_buf, hash_size, HASH_FUNCTION_ELEMENTS);

	free(temp2_buf);
	free(temp_buf);
	return result;
}

int hash2_buffer_to_bytes(uint8_t *input_str, int input_len, uint8_t *last_hash, int hash_size, uint8_t *output_buf) {

	// concatenate last_buf + input_str (to len), then hash to bytes into output_buf
	int result;
	// copy the last hash buffer into temp buf
	// copy the current input string into buffer
	PyObject *last = PyBytes_FromStringAndSize((const char *) last_hash, (Py_ssize_t) hash_size);
	PyObject *input = PyBytes_FromStringAndSize((const char *) input_str, (Py_ssize_t) input_len);

	PyBytes_ConcatAndDel(&last, input);
	uint8_t *temp_buf = (uint8_t *) PyBytes_AsString(last);

	// hash the contents of temp_buf
	debug("last_hash => ");
	printf_buffer_as_hex(last_hash, hash_size);

	debug("input_str => ");
	printf_buffer_as_hex(input_str, input_len);

	debug("temp_buf => ");
	printf_buffer_as_hex(temp_buf, input_len + hash_size);

	result = hash_to_bytes(temp_buf, (input_len + hash_size), output_buf, hash_size, HASH_FUNCTION_STRINGS);

	Py_XDECREF(last);
	return result;
}

// TODO: PyObject *Element_new(PyTypeObject *type, PyObject *args, PyObject *kwds)

// TODO: PyObject *Pairing_new(PyTypeObject *type, PyObject *args, PyObject *kwds)

// TODO: int Element_init(Element *self, PyObject *args, PyObject *kwds)

// TODO: int Pairing_init(Pairing *self, PyObject *args, PyObject *kwds)

/*
PyObject *Element_call(Element *elem, PyObject *args, PyObject *kwds)
{
	PyObject *object;
	Element *newObject;
	
	if(!PyArg_ParseTuple(args, "O:ref", &object)) {
		EXIT_IF(TRUE, "invalid argument.");
	}
	
	newObject = (Element *) object;
	// element_printf("Elment->e => '%B'\n", newObject->e);
	debug("Element->type => '%d'\n", newObject->element_type);
	
	return NULL;
}
*/
 
// TODO: static PyObject *Element_elem(Element* self, PyObject* args)

// TODO: PyObject *Pairing_print(Pairing* self)

// TODO: PyObject *Element_print(Element* self)

// TODO: static PyObject *Element_random(Element* self, PyObject* args)

// TODO: static PyObject *Element_add(Element *self, Element *other)

// TODO: static PyObject *Element_sub(Element *self, Element *other)


/* requires more care -- understand possibilities first */
// TODO: static PyObject *Element_mul(PyObject *lhs, PyObject *rhs)

// TODO: static PyObject *Element_div(PyObject *lhs, PyObject *rhs)
 
// TODO: static PyObject *Element_invert(Element *self)

// TODO: static PyObject *Element_negate(Element *self)

// TODO: static PyObject *Element_pow(PyObject *o1, PyObject *o2, PyObject *o3)

/* We assume the element has been initialized into a specific field (G1,G2,GT,or Zr),
 * before setting the element. */
// TODO: static PyObject *Element_set(Element *self, PyObject *args)

// TODO: static PyObject  *Element_initPP(Element *self, PyObject *args)

/* Takes a list of two objects in G1 & G2 respectively and computes the multi-pairing */
// TODO: PyObject *multi_pairing(Pairing *groupObj, PyObject *listG1, PyObject *listG2)

/* this is a type method that is visible on the global or class level. Therefore,
   the function prototype needs the self (element class) and the args (tuple of Element objects).
 */
// TODO: PyObject *Apply_pairing(PyObject *self, PyObject *args)

// TODO: PyObject *sha2_hash(Element *self, PyObject *args)

// The hash function should be able to handle elements of various types and accept
// a field to hash too. For example, a string can be hashed to Zr or G1, an element in G1 can be
// TODO: static PyObject *Element_hash(Element *self, PyObject *args)

// TODO: static PyObject *Element_equals(PyObject *lhs, PyObject *rhs, int opid) {

// TODO: static PyObject *Element_long(PyObject *o1)

// TODO: static long Element_index(Element *o1)

// TODO: static PyObject *Deserialize_cmp(PyObject *self, PyObject *args) {

void print_mpz(mpz_t x, int base) {
#ifdef DEBUG
	if(base <= 2 || base > 64) return;
	size_t x_size = mpz_sizeinbase(x, base) + 2;
	char *x_str = (char *) malloc(x_size);
	x_str = mpz_get_str(x_str, base, x);
	printf("Element => '%s'\n", x_str);
	printf("Order of Element => '%zd'\n", x_size);
	free(x_str);
#endif
}

//TODO: int check_membership(Element *elementObj) 

// TODO: static PyObject *Group_Check(Element *self, PyObject *args)

// TODO: static PyObject *Get_Order(Element *self, PyObject *args)

#ifdef BENCHMARK_ENABLED

#define BenchmarkIdentifier 1
#define GET_RESULTS_FUNC	GetResultsWithPair
#define GROUP_OBJECT		Pairing
#define BENCH_ERROR			ElementError
/* helper function for granularBenchmar */
PyObject *PyCreateList(Operations *gBench, MeasureType type)
{
	int countZR = -1, countG1 = -1, countG2 = -1, countGT = -1;
	GetField(countZR, type, ZR, gBench);
	GetField(countG1, type, G1, gBench);
	GetField(countG2, type, G2, gBench);
	GetField(countGT, type, GT, gBench);

	PyObject *objList = Py_BuildValue("[iiii]", countZR, countG1, countG2, countGT);
	return objList;
}

#include "benchmark_util.c"

#endif

#if PY_MAJOR_VERSION >= 3
// TODO: PyTypeObject PairingType
#else
/* python 2.x series */
// TODO: PyTypeObject PairingType 
#endif

#if PY_MAJOR_VERSION >= 3
// TODO: PyNumberMethods element_number
// TODO: PyTypeObject ElementType 
#else
/* python 2.x series */
// TODO: PyNumberMethods element_number
// TODO: PyTypeObject ElementType
#endif

// TODO: struct module_state

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state *) PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
// TODO: static struct module_state _state;
#endif

// end
// TODO: PyMemberDef Element_members[] 

// TODO: PyMethodDef Element_methods[] 

// TODO: PyMethodDef pairing_methods[] 

#if PY_MAJOR_VERSION >= 3
// TODO: static int pairings_traverse(PyObject *m, visitproc visit, void *arg)

// TODO: static int pairings_clear(PyObject *m)

// TODO: static int pairings_free(PyObject *m)

// TODO: static struct PyModuleDef moduledef 

#define CLEAN_EXIT goto LEAVE;
#define INITERROR return NULL
PyMODINIT_FUNC
// TODO: PyInit_pairing(void) {
#else
#define CLEAN_EXIT goto LEAVE;
#define INITERROR return
// TODO: void initpairing(void) {
#endif
