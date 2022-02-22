line 89 > pyobject f

112 >> longObjToMPZ

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
static Element *createNewElement(GroupType element_type, Pairing *pairing) {
	debug("Create an object of type Element\n");
	Element *retObject = PyObject_New(Element, &ElementType);
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
	Py_INCREF(retObject->pairing);
	return retObject;	
}

Element *convertToZR(PyObject *longObj, PyObject *elemObj) {
	Element *self = (Element *) elemObj;
	Element *new = createNewElement(ZR, self->pairing);

	mpz_t x;
	mpz_init(x);
#if PY_MAJOR_VERSION < 3
	PyObject *longObj2 = PyNumber_Long(longObj);
	longObjToMPZ(x, (PyLongObject *) longObj2);
	Py_DECREF(longObj2);
#else
	longObjToMPZ(x, (PyLongObject *) longObj);
#endif
	element_set_mpz(new->e, x);
	mpz_clear(x);
	return new;
}