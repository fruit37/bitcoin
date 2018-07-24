#include <stdio.h>
#include <iostream>
#include "Enclave_u.h"
#include "sgx_urts.h"
#include "sgx_utils.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

int main(int argc, char const *argv[]) {
	if (initialize_enclave(&global_eid, "enclave.token", "signed.so") < 0) {
        	std::cout << "Fail to initialize enclave." << std::endl;
        	return 1;
    	}
	int ptr;
	char* str = "Hello";
	sgx_status_t status = calculatePrime(global_eid);
	if (status != SGX_SUCCESS) {
		std::cout << "FAILURE!!!" << std::endl;
	}
	printf("Successful.\n");
	
	if(SGX_SUCCESS != sgx_destroy_enclave(global_eid))
		return -1;
	return 0;
}
