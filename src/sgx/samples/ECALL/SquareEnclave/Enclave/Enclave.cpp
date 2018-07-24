#include "sgx_eid.h"
#include "Enclave_t.h"
#include "sgx_thread.h"
#include "sgx_dh.h"

int square(int *ptr) {
	return *ptr * *ptr;
}
