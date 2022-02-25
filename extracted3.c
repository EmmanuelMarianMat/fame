void 	Pairing_dealloc(Pairing *self)
{
	if(self->param_buf != NULL) {
		debug("param_buf => %p\n", self->param_buf);
		free(self->param_buf);
	}

	debug("Clear pairing => 0x%p\n", self->pair_obj);
	if(self->group_init == TRUE) {
		pairing_clear(self->pair_obj);
		pbc_param_clear(self->p);
	}

#ifdef BENCHMARK_ENABLED
	if(self->dBench != NULL) {
//		PrintPyRef("releasing benchmark object", self->dBench);
		Py_CLEAR(self->dBench);
		if(self->gBench != NULL) {
//			PrintPyRef("releasing operations object", self->gBench);
			Py_CLEAR(self->gBench);
		}
	}
#endif
	debug("Releasing pairing object!\n");
	Py_TYPE(self)->tp_free((PyObject *) self);
}

void	Element_dealloc(Element* self)
{
	if(self->elem_initialized == TRUE && self->e != NULL) {
		debug_e("Clear element_t => '%B'\n", self->e);
		if(self->elem_initPP == TRUE) {
			element_pp_clear(self->e_pp);
		}
		element_clear(self->e);
		Py_DECREF(self->pairing);
	}

	Py_TYPE(self)->tp_free((PyObject*)self);
}

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