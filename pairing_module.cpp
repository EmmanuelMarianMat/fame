#include "pairing_module.h"
#include <iostream>
using namespace std;

Element_class* Pairing_module::Element_elem(Pairing_module *group, enum Group type, unsigned long long_obj) {
    Element_class *retObject = NULL;

    if(type >= ZR && type <= GT)
        retObject = createNewElement(type, group);
    else {
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

Element_class *Pairing_module::Apply_pairing(Element_class *lhs, Element_class *rhs, Pairing_module *group) {
    Element_class *newObject;
    IS_SAME_GROUP(lhs, rhs);
    if(pairing_is_symmetric(lhs->pairing->pair_obj)) {
        newObject = createNewElement(GT, lhs->pairing);
        pairing_apply(newObject->e, lhs->e, rhs->e, rhs->pairing->pair_obj);
        return newObject;
    }

    if(lhs->element_type == rhs->element_type) {
        if(lhs->element_type == G1) {
            cout<<"Both elements are of type G1 in asymmetric pairing";
            return NULL;
        }
        if(lhs->element_type == G2) {
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

char* Pairing_module::sha2_hash(Element_class *object, int label) {
    char* hash_hex = NULL;
    if(object->elem_initialized == FALSE) {
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
    char* str(hash_hex);
    free(hash_hex);
    return str;
}

// The hash function should be able to handle elements of various types and accept
// a field to hash too. For example, a string can be hashed to Zr or G1, an element in G1 can be
Element_class *Pairing_module::Element_hash(Pairing_module *group, char* str, enum Group type) {
    Element_class *newObject = NULL, *object = NULL;
    int result;

    char *tmp = NULL;

    int hash_len = mpz_sizeinbase(group->pair_obj->r, 2) / BYTE;
    uint8_t hash_buf[hash_len];
    memset(hash_buf, 0, hash_len);

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

Element_class *Pairing_module::Element_random(Pairing_module *group, int arg1, int seed) {
    Element_class *retObject;
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

    if(seed > -1)
        pbc_random_set_deterministic((uint32_t) seed);

    element_random(retObject->e);
    retObject->elem_initialized = TRUE;
    retObject->elem_initPP = FALSE;
    retObject->element_type = (enum Group)e_type;
    /* set the group object for element operations */
    retObject->pairing = group;
    return retObject;
}

const char* Pairing_module::Serialize_cmp(Element_class *element, int compression) {
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
        if(data_buf == NULL) {
            cout<<"heap full";
            return NULL;
        }
        // write to char buffer
        bytes_written = element_to_bytes(data_buf, element->e);
    }
    else if(element->element_type != NONE_G) {
    // object initialized now retrieve element and serialize to a char buffer.
        if(compression)
            elem_len = element_length_in_bytes_compressed(element->e);
        else
            elem_len = element_length_in_bytes(element->e);

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

Element_class *Pairing_module::Deserialize_cmp(char *object, Pairing_module *group, int compression) {
    Element_class *origObject = NULL;

    uint8_t *serial_buf = (uint8_t *)object;
    int type = atoi((const char *) &(serial_buf[0]));
    uint8_t *base64_buf = (uint8_t *)(serial_buf + 2);

    size_t deserialized_len = 0;
    uint8_t *binary_buf = (uint8_t*)NewBase64Decode((const char *) base64_buf, strlen((char *) base64_buf), &deserialized_len);

    if((type == ZR || type == GT) && deserialized_len > 0) {
    //				debug("result => ");
    //				printf_buffer_as_hex(binary_buf, deserialized_len);
        origObject = createNewElement((enum Group)type, group);
        element_from_bytes(origObject->e, binary_buf);
        free(binary_buf);
        return origObject;
    }
    else if((type == G1 || type == G2) && deserialized_len > 0) {
        // now convert element back to an element type (assume of type ZR for now)
        origObject = createNewElement((enum Group)type, group);
        if(compression)
            element_from_bytes_compressed(origObject->e, binary_buf);
        else
            element_from_bytes(origObject->e, binary_buf);

        free(binary_buf);
        return origObject;
    }

    cout<<"Nothing to deserialize in element.";
    return NULL;
}

bool Pairing_module::Group_Check(Pairing_module *group, Element_class *object) {
    if(check_membership(object) == TRUE)
        return true;
    else
        return false;
}

long Pairing_module::Get_Order(Pairing_module *group) {
    long object;
    mpz_set_si(group->pair_obj->r, object);
    return object; /* returns a PyInt */
}