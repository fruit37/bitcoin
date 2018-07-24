#include "sgx_eid.h"
#include "Enclave_t.h"
#include "sgx_thread.h"
#include "sgx_dh.h"

void pointer(int *temp) {
	//return the one to outside.
	*temp = *temp + 1;
}
