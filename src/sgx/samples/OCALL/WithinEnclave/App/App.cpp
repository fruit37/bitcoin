#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include "Enclave_u.h"
#include "sgx_urts.h"
#include "sgx_utils.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

void callOutside() {
	printf("I am outside");
}

int main(int argc, char const *argv[]) {
	if (initialize_enclave(&global_eid, "enclave.token", "signed.so") < 0) {
        	std::cout << "Fail to initialize enclave." << std::endl;
        	return 1;
    	}
	char* str = "Am I too long ? huh ?";
	callWithin(global_eid, str);


	printf("Finish main.\n");

	if(SGX_SUCCESS != sgx_destroy_enclave(global_eid))
		return -1;
	return 0;
}
