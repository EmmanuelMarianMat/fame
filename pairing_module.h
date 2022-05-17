#ifndef PAIRING_MODULE_H
#define PAIRING_MODULE_H
#include "enum.h"
#include <base64.h>
#include <pbc/pbc.h>
#include <limits.h>
//#ifndef ELEMENT_CLASS_H
#include "element_class.h"
//#endif
#include "openssl/sha.h"
#define ID_LEN 8
#define IS_SAME_GROUP(a, b) \
	if(strncmp((const char *) a->pairing->hash_id, (const char *) b->pairing->hash_id, ID_LEN) != 0) {	\
		cout<<"mixing group elements from different curves.";	\
		return NULL;	\
	}
#define HASH_LEN SHA256_DIGEST_LENGTH

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

class Element_class *createNewElement(enum Group, class Pairing_module *);

class Pairing_module {
	public:
		pbc_param_t p;
		char *params;
		char *param_buf;
		pairing_t pair_obj;
		int group_init;
		uint8_t hash_id[ID_LEN+1];
		Element_class *element;
		Element_class *Element_elem(Pairing_module *group, enum Group type,unsigned long long_obj=LONG_MAX);
		Element_class *Apply_pairing(Element_class *lhs, Element_class *rhs, Pairing_module *group = NULL);
		char* sha2_hash(Element_class *object, int label);
		Element_class *Element_hash(Pairing_module *group, char* str, enum Group type);
		Element_class *Element_random(Pairing_module *group, int arg1, int seed=-1);
		const char* Serialize_cmp(Element_class *element = NULL, int compression=1);
		Element_class *Deserialize_cmp(char *object, Pairing_module *group = NULL, int compression = 1);
		bool Group_Check(Pairing_module *group = NULL, Element_class *object = NULL);
		long Get_Order(Pairing_module *group = NULL);
};
#endif