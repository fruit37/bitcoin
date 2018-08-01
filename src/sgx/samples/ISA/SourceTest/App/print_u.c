#include "print_u.h"
#include <errno.h>

typedef struct ms_printString_t {
	char* ms_str;
} ms_printString_t;

static sgx_status_t SGX_CDECL print_printString(void* pms)
{
	ms_printString_t* ms = SGX_CAST(ms_printString_t*, pms);
	printString((const char*)ms->ms_str);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[1];
} ocall_table_print = {
	1,
	{
		(void*)print_printString,
	}
};
sgx_status_t callEnclaveToPrint(sgx_enclave_id_t eid)
{
	sgx_status_t status;
	status = sgx_ecall(eid, 0, &ocall_table_print, NULL);
	return status;
}

