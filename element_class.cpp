#include "element_class.h"

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

bool Element_class::Element_initPP() {
    if(elem_initPP == TRUE)
        std::cout<<"Pre-processing table alreay initialized.";

    if(elem_initialized == FALSE)
        std::cout<<"Must initialize element to a field (G1, G2, or GT).";

    if(element_type >= G1 && element_type <= GT) {
        element_pp_init(e_pp, e);
        elem_initPP = TRUE;
        return true;
    }
    return false;
}

void Element_class::Element_set(long int value) {
    if(elem_initialized == FALSE)
        std::cout<<"Must initialize element to a field (G1, G2, or GT).";

    if(value == 0)
            element_set0(e);
    else if(value == 1)
            element_set1(e);
    else
            element_set_si(e, (signed int) value);
}

Element_class* Element_class::Element_pow(Element_class *o1, Element_class *o2){
    Element_class *newObject = NULL;
    mpz_t n;
    IS_SAME_GROUP(o1, o2);
    if(exp_rule(o1->element_type, o2->element_type) == FALSE){
        std::cout<<"invalid exp operation\n";
        return NULL;
    else{
        if(lhs_o1->elem_initPP == TRUE) {
            // n = g ^ e where g has been pre-processed
            mpz_init(n);
            element_to_mpz(n, rhs_o2->e);
            element_pp_pow(newObject->e, n, lhs_o1->e_pp);
            mpz_clear(n);
        }
        else {
            element_pow_zn(newObject->e, lhs_o1->e, rhs_o2->e);
        }
    }

    return newObject;
}

Element_class* Element_class::Element_add(Element_class *o1, Element_class *o2){
    Element_class *newObject;
    IS_SAME_GROUP(o1, o2);
	if(add_rule(o1->element_type, o2->element_type) == FALSE){
        std::cout<<"invalid add operation.";
        return NULL;
    }
    newObject = createNewElement(o1->element_type, o1->pairing);
	element_add(newObject->e, o1->e, o2->e);

    return newObject;
}

Element_class* Element_class::Element_mul(Element_class *o1, Element_class *o2){
    Element_class *newObject;
    IS_SAME_GROUP(o1, o2);
    if(mul_rule(o1->element_type, o2->element_type) == FALSE){
        std::cout<<"invalid mul operation."<<std::endl;
        return NULL;
    }

    if(o1->element_type != ZR && o2->element_type == ZR) {
        newObject = createNewElement(o1->element_type, o1->pairing);
        element_mul_zn(newObject->e, o1->e, o2->e);		
    }
    else if(o2->element_type != ZR && o1->element_type == ZR) {
        newObject = createNewElement(o2->element_type, o1->pairing);
        element_mul_zn(newObject->e, o2->e, o1->e);
    }
    else { 
        newObject = createNewElement(o1->element_type, o1->pairing);
        element_mul(newObject->e, o1->e, o2->e);		
    }

    return newObject;
}

Element_class* Element_class::Element_div(Element_class *o1, Element_class *o2){
    Element_class *newObject = NULL;
    signed long int z;
    IS_SAME_GROUP(o1, o2);
    if(div_rule(o1->element_type, o2->element_type) == FALSE){
        std::cout << "invalid div operation.";
    }
    newObject = createNewElement(o1->element_type, o1->pairing);
    element_div(newObject->e, o1->e, o2->e);
    return newObject;
}

Element_class* Element_class::Element_invert(Element_class *o1){
    Element_class* newObject = createNewElement(o1->element_type, o1->pairing);
	element_invert(newObject->e, o1->e);
	return newObject;
}


