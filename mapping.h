#include <stdlib.h>
#include <gmp.h>
#include <pbc/pbc.h>

#define ID_LEN   	8

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

int main(){
    return 0;
}