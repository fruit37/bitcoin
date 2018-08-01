#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include "Enclave_u.h"
#include "sgx_urts.h"
#include "sgx_utils.h"
#include "sgx_uae_service.h"
#include "sgx_ukey_exchange.h"
#include "base64.h"
#include "connection.h"
#include "restclient.h"
#include "base64.h"
#include "json.hpp"
#include <bitset>

using namespace std;
using json = nlohmann::json;

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

void printString(const char* ptr) {
	printf(ptr);
	cout << "\n";
}

void checkStatus(sgx_status_t status) {
	if (status == SGX_ERROR_INVALID_PARAMETER) {
		std::cout << "Invalid parameter!!!" << std::endl;
	} else if (status == SGX_ERROR_AE_INVALID_EPIDBLOB) {
		std::cout << "EPID corrupted" << std::endl;
	} else if (status == SGX_ERROR_EPID_MEMBER_REVOKED) {
		std::cout << "EPIDBLOB" << std::endl;
	} else if (status == SGX_ERROR_BUSY) {
		std::cout << "Busy" << std::endl;
	} else if (status == SGX_ERROR_UPDATE_NEEDED) {
		std::cout << "Update needed" << std::endl;
	} else if (status == SGX_ERROR_OUT_OF_MEMORY) {
		std::cout << "No Memory" << std::endl;
	} else if (status == SGX_ERROR_OUT_OF_EPC) {
		std::cout << "Out of EPC" << std::endl;
	} else if (status == SGX_ERROR_SERVICE_UNAVAILABLE) {
		std::cout << "AE Service not respond" << std::endl;
	} else if (status == SGX_ERROR_SERVICE_TIMEOUT) {
		std::cout << "Service time out" << std::endl;
	} else if (status == SGX_ERROR_NETWORK_FAILURE) {
		std::cout << "Network problem" << std::endl;
	} else if (status == SGX_ERROR_INVALID_STATE) {
		std::cout << "Invalid state" << std::endl;
	} else if (status == SGX_ERROR_INVALID_SIGNATURE) {
		std::cout << "Invalid signature" << std::endl;
	} else if (status == SGX_ERROR_MAC_MISMATCH) {
		std::cout << "Mac mismatch" << std::endl;
	} else if (status == SGX_ERROR_KDF_MISMATCH) {
		std::cout << "KDF mismatch" << std::endl;
	} else if (status == SGX_ERROR_UNRECOGNIZED_PLATFORM) {
		std::cout << "Unrecognized platform" << std::endl;
	} else if (status == SGX_ERROR_UNEXPECTED) {
		std::cout << "Unexpected error" << std::endl; 
	} else if (status != SGX_SUCCESS) {
		std::cout << "Error Not Yet Covered" << std::endl;
	} else {
		std::cout << "No error" << std::endl;
	}
}

//For service provider to get the SigRL
void requestSignRL(sgx_epid_group_id_t* gid, char* respBody) {
	//Note: the gid is 00000AC7 and it is encoded as BIG ENDIAN
	string website = "https://test-as.sgx.trustedservices.intel.com:443";
	RestClient::init();
	RestClient::Connection* conn = new RestClient::Connection(website);
	conn->SetCertPath("/home/billy/Documents/samples/ISA/certificate/test.crt");
	conn->SetKeyPath("/home/billy/Documents/samples/ISA/certificate/test.key");
	conn->SetCertType("PEM");
	RestClient::Response resp = conn->get("/attestation/sgx/v2/sigrl/00000AC7");
	int code = resp.code;
	printf("The code is %d, and it should be 200.\n", code);
	//The SigRL (resp.body) should be null, nothing is in the Revocational List.
	for (int i = 0; i < resp.body.length(); i++) {
		*(respBody+i) = resp.body.at(i);
	}
}

//For service provider to handle msg3
void handleMsg3ServiceProvider(sgx_ra_msg3_t* msg3) {
	//TODO: Verify G_a
	
	//TODO: Verify CMAC

	//TODO: Verify Msg3 Content with Intel
	//cout << "Msg3's content is ";
	char quote[1116];
	for (int i = 0; i < 1116; i++) {
		quote[i] = (msg3->quote)[i];
	}
	string encoded = base64_encode((const unsigned char*)quote, 1116);
	//cout << encoded;


	string jsonString = "{\"isvEnclaveQuote\": \"" + encoded + "\"}";
	cout << jsonString;

	//Sending quote to Intel
	string website = "https://test-as.sgx.trustedservices.intel.com:443";
	RestClient::init();
	RestClient::Connection* conn = new RestClient::Connection(website);
	conn->SetCertPath("/home/billy/Documents/samples/ISA/certificate/test.crt");
	conn->SetKeyPath("/home/billy/Documents/samples/ISA/certificate/test.key");
	conn->SetCertType("PEM");
	conn->AppendHeader("Content-Type", "application/json");
	RestClient::Response resp = conn->post("/attestation/sgx/v2/report", jsonString);
	printf("The code is %d.", resp.code);
}

int main(int argc, char const *argv[]) {
	//INITIZALIZE THE ENCLAVE
	if (initialize_enclave(&global_eid, "enclave.token", "signed.so") < 0) {
        	std::cout << "Fail to initialize enclave." << std::endl;
        	return 1;
    	}
    
    	//MSG0 BEGIN
	cout << "MSG0 BEGIN" << endl;
    	sgx_ra_context_t context = 0; //reserve memory to be used for enclave context
   	int pse = 0; //1 to use platform service, otherwise 0
    	msg0(global_eid , &pse, &context); //Ecall inside to the enclave to execute sgx_ra_init

    	//OPTIONAL:
    	//uint32_t gid; //reserve memory to be used for the GID of the EPID
    	//checkStatus(sgx_get_extended_epid_group_id(&gid)); //get the GID
    	//cout << "The GID is " << hex << gid << "\n";
	//MSG0 END
	cout << "MSG0 END" << endl;

	//msg1 Begin
	cout << "MSG1 BEGIN" << endl;
	sgx_ra_msg1_t* msg1 = (sgx_ra_msg1_t*) malloc(68); //reserve memory to be used for msg1
	checkStatus(sgx_ra_get_msg1(context, global_eid, sgx_ra_get_ga, msg1)); //get msg1

	cout << "The GID is " << (unsigned)(msg1->gid)[0] << (unsigned)(msg1->gid)[1] << (unsigned)(msg1->gid)[2] << (unsigned)(msg1->gid)[3] << "\n";
	cout << "The GID is (hex) " << hex << (unsigned)(msg1->gid)[0] << (unsigned)(msg1->gid)[1] << (unsigned)(msg1->gid)[2] << (unsigned)(msg1->gid)[3] << "\n";

	sgx_ra_msg2_t* msg2 = (sgx_ra_msg2_t*) malloc(sizeof(sgx_ra_msg2_t));
	fakeServiceProvider(global_eid, msg1, 1, msg2); //sending msg1 to service provider, a local service provider is being used for testing purpose, msg2 is returned by the service provider
	//msg1 End
	cout << "MSG1 END" << endl;

	//msg3 Begin
	//Note: the sizeof(sgx_ra_msg2_t) and sizeof(sgx_ra_msg3_t) should depend on the real size of the message.
	const sgx_ra_msg2_t* p_msg2 = msg2;
	int sizeOfMsg2 = sizeof(sgx_ra_msg2_t);
	uint32_t sizeOfMsg3 = 0;
	sgx_ra_msg3_t* msg3 = (sgx_ra_msg3_t*) malloc(1000);
	checkStatus(sgx_ra_proc_msg2(context, global_eid, sgx_ra_proc_msg2_trusted, sgx_ra_get_msg3_trusted, p_msg2, sizeOfMsg2, &msg3, &sizeOfMsg3));
	
	//handleMsg3ServiceProvider(msg3);
	

	printf("Finish main.\n");

	if(SGX_SUCCESS != sgx_destroy_enclave(global_eid))
		return -1;
	return 0;
}
