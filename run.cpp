#include "mapping.h"
#include <iostream>
using namespace std;

int main(){
    uint8_t input = 0;
    uint8_t output = 0;
    uint8_t hash_prefix = 0;

    cout << hash_to_bytes(&input, 0, &output, 0, hash_prefix);
    return 0;
}