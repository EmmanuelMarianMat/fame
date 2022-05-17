#ifndef ELEMENT_CLASS_H
#define ELEMENT_CLASS_H
#include "pairing_module.h"
#include "mapping.h"
#include<gmp.h>
#include<pbc.h>
class Pairing_module;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

class Element_class {
	public:
		Pairing_module *pairing;
		element_t e;
		enum Group element_type;
		int elem_initialized;
		element_pp_t e_pp;
		int elem_initPP;
		bool Element_initPP();
		void Element_set(long int value);
		Element_class* Element_pow(Element_class *, Element_class *);
		Element_class* Element_add(Element_class *, Element_class *);
		Element_class* Element_mul(Element_class *, Element_class *);
		Element_class* Element_div(Element_class *, Element_class *);
		Element_class* Element_invert(Element_class *);
};
#endif