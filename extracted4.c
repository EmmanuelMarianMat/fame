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