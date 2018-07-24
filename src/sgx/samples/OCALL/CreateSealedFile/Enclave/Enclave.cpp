#include "sgx_eid.h"
#include "Enclave_t.h"
#include "sgx_thread.h"
#include "sgx_dh.h"
#include "sgx_tprotected_fs.h"

void createNewFile() {
	//File name
	const char* fileName = "HelloWorld.cpp";
	
	//Write a new file to disk via enclave file system with auto key encrypted
	SGX_FILE* ptr = sgx_fopen_auto_key(fileName, "w");
	
	char* beforeCast = "HelloWorld";
	void* afterCast = static_cast<void*>(beforeCast);
	
	size_t size = sizeof(char);
	size_t count = 20;
	
	//sgx write
	sgx_fwrite(afterCast, size, count, ptr);
	
	//sgx close
	sgx_fclose(ptr);
	
	//sgx reopen
	ptr = sgx_fopen_auto_key(fileName, "r");
	
	void* receivePtr = (void*) malloc(size*count);
	
	//sgx read
	sgx_fread(receivePtr, size, count, ptr);
	
	char* receive = static_cast<char*>(receivePtr);
	
	printString(receive);
	
	/////////////////////////////////////////////////////////////
	printString("Trying to create a new file with manual key.\n");
	
	//Generating key request
	

	
	if (ptr == NULL) {
		const char* str = "error";
		printString(str);
	}
}
