#include "sgx_eid.h"
#include "Enclave_t.h"
#include "sgx_thread.h"
#include "sgx_dh.h"
#include <string.h>
#include <stdio.h>

void executeBinEnclave(void* ptr) {
		size_t size = sizeof(char);
		size_t count = 1000;
		//SGX_FILE* file = sgx_fopen_auto_key("execute", "w");
		//sgx_fwrite(ptr, size, count, file);
		
		FILE* ptr = fopen("test", "w");
		//fwrite("Hello,testing", size, count, ptr);
		//fclose(ptr);
}
