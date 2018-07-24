#include "sgx_eid.h"
#include "Enclave_t.h"
#include "sgx_thread.h"
#include "sgx_dh.h"
#include "sgx_trts.h"

void callWithin(const char* str) {
	//checking if the address starting at str to the next 14500000 will
	// be inside the enclave.
	int temp = sgx_is_within_enclave(str,14500000);
	if (temp == 1) {
		callOutside();
	}
}
