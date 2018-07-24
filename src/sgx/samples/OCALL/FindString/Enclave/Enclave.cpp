#include "sgx_eid.h"
#include "Enclave_t.h"
#include "sgx_thread.h"
#include "sgx_dh.h"
#include <string.h>

int findOccurString(char* str, char* word) {
	int count = 0;
	int size = strlen(word);
	str = strstr(str, word);
	while (str != NULL) {
		count += 1;
		str = str + size;
		str = strstr(str, word);
	}
	return count;
}
