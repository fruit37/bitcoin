#include "sgx_eid.h"
#include "Enclave_t.h"
#include "sgx_thread.h"
#include "sgx_dh.h"

void callEnclaveToPrint() {
	char* str = "I am from enclave.\n\0";
	//OCALL
	printString(str);
}
