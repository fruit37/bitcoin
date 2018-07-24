#include "sgx_eid.h"
#include "Enclave_t.h"
#include "sgx_thread.h"
#include "sgx_dh.h"
#include "sgx_trts.h"

void calculatePrime() {
	//checking if the address starting at str to the next 14500000 will
	// be inside the enclave.
	for (int i = 2; i < 1000000; i++) {
		for (int j = 2; j < i; j++) {
			if (i%j == 0)
				break;
		}
	}
}
