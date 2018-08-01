//This file is intended to be blank as this example illustrates an Enclave with imported library.
//The Trusted folder contains the files of the imported library.
#include "sgx_eid.h"
#include "Enclave_t.h"
#include "sgx_thread.h"
#include "sgx_dh.h"
#include "sgx_tcrypto.h"
#include "sgx_tkey_exchange.h"
#include "sgx_tae_service.h"
#include "string.h"

void checkStat(sgx_status_t status) {
	if (status == SGX_ERROR_INVALID_PARAMETER) {
		printString("Invalid parameter");
	} else if (status == SGX_ERROR_OUT_OF_MEMORY) {
		printString("No memory");
	} else if (status == SGX_ERROR_UNEXPECTED) {
		printString("Unexpected");
	} else if (status != SGX_SUCCESS) {
		printString("Not Covered");
	}
	//printString("Success");
}

//This is the public key of the service provider, and it should be hardcoded inside the enclave.
static const sgx_ec256_public_t g_sp_pub_key = {
    {
        0x72, 0x12, 0x8a, 0x7a, 0x17, 0x52, 0x6e, 0xbf,
        0x85, 0xd0, 0x3a, 0x62, 0x37, 0x30, 0xae, 0xad,
        0x3e, 0x3d, 0xaa, 0xee, 0x9c, 0x60, 0x73, 0x1d,
        0xb0, 0x5b, 0xe8, 0x62, 0x1c, 0x4b, 0xeb, 0x38
    },
    {
        0xd4, 0x81, 0x40, 0xd9, 0x50, 0xe2, 0x57, 0x7b,
        0x26, 0xee, 0xb7, 0x41, 0xe7, 0xc6, 0x14, 0xe2,
        0x24, 0xb7, 0xbd, 0xc9, 0x03, 0xf2, 0x9a, 0x28,
        0xa8, 0x3c, 0xc8, 0x10, 0x11, 0x14, 0x5e, 0x06
    }
};
//This is the private key of the service provider, and it should be stored securly.
static const sgx_ec256_private_t g_sp_priv_key = {
    {
        0x90, 0xe7, 0x6c, 0xbb, 0x2d, 0x52, 0xa1, 0xce,
        0x3b, 0x66, 0xde, 0x11, 0x43, 0x9c, 0x87, 0xec,
        0x1f, 0x86, 0x6a, 0x3b, 0x65, 0xb6, 0xae, 0xea,
        0xad, 0x57, 0x34, 0x53, 0xd1, 0x03, 0x8c, 0x01
    }
};

static const uint8_t SPID [16] = {
	    0x7D, 0xBC, 0x54, 0xAB, 0x5C, 0x88, 0xFF, 0x30,
	    0xD1, 0xBA, 0xED, 0xA2, 0xE2, 0xA6, 0xF8, 0xA3
	    };

void msg0(int* pse, sgx_ra_context_t* ptr){
	checkStat(sgx_ra_init(&g_sp_pub_key, *pse, ptr));
}

//Service Provider should not reside inside the enclave.
//It is only for testing purpose.
void fakeServiceProvider(sgx_ra_msg1_t* msg, int type, sgx_ra_msg2_t* msg2) {
	if (type == 1) {
		//msg2 Begin
		sgx_ecc_state_handle_t* eccHandler = (sgx_ecc_state_handle_t*) malloc(sizeof(sgx_ecc_state_handle_t)); //reserve memory to be used for ecc handler
		checkStat(sgx_ecc256_open_context(eccHandler)); //handler must be created for creating key pair
		sgx_ec256_private_t* privateKey = (sgx_ec256_private_t*) malloc(sizeof(sgx_ec256_private_t)); //reserve memory to be used for private key
		sgx_ec256_public_t* publicKey = (sgx_ec256_public_t*) malloc(sizeof(sgx_ec256_public_t)); //reserve memory to be used for public key
		checkStat(sgx_ecc256_create_key_pair(privateKey, publicKey, *eccHandler)); //create key pair - G_b
		
		sgx_ec256_dh_shared_t* sharedKey = (sgx_ec256_dh_shared_t*) malloc(sizeof(sgx_ec256_dh_shared_t)); //reserve memory to be used for shared key Gabx
		checkStat(sgx_ecc256_compute_shared_dhkey(privateKey, &(msg->g_a), sharedKey, *eccHandler)); //compute shared key, it has to be LITTLE_ENDIAN
		//TODO: Change the shared key to little endian


		const sgx_cmac_128bit_key_t zeroBlock = {0x00, }; //the key used to generate cmac is 0x00 block
		sgx_cmac_128bit_tag_t* KDK = (sgx_cmac_128bit_tag_t*) malloc(sizeof(sgx_cmac_128bit_tag_t)); //reserve memory to be used for KDK
		checkStat(sgx_rijndael128_cmac_msg(&zeroBlock, (uint8_t*)sharedKey, 32, KDK)); //generate the cmac
		
		uint8_t block[7] = {0x01, 0x53, 0x4D, 0x4B, 0x00, 0x80, 0x00}; //to be cmaced
		sgx_cmac_128bit_tag_t* SMK = (sgx_cmac_128bit_tag_t*) malloc(sizeof(sgx_cmac_128bit_tag_t)); //reserve memory to be used for ?????
		checkStat(sgx_rijndael128_cmac_msg(KDK, block, 7, SMK)); //generate cmac
		
		uint16_t quoteType = 0x0001; //0x0000 for unlinkable SP, otherwise 0x0001
		uint16_t KDF_ID = 0x0001;
		uint16_t* GbGa = (uint16_t*) malloc(sizeof(uint16_t)*64); //128bytes
		memcpy(GbGa, &(publicKey->gx), 32);
		memcpy(GbGa+16, &(publicKey->gy), 32);
		memcpy(GbGa+32, &((msg->g_a).gx), 32);
		memcpy(GbGa+48, &((msg->g_a).gy), 32);
		
		sgx_ec256_signature_t* signature = (sgx_ec256_signature_t*) malloc(sizeof(sgx_ec256_signature_t)); //reserve memory to be used for ESDCA signature
		checkStat(sgx_ecdsa_sign((const uint8_t*) GbGa, 128, (sgx_ec256_private_t*) &g_sp_priv_key, signature, *eccHandler)); //get the signature
	
		//uint8_t result;
		//checkStat(sgx_ecdsa_verify((uint8_t*) GbGa, 128, publicKey, signature, &result, *eccHandler));
		//if(SGX_EC_VALID != result)
		//	printString("Testing Invalid Signature"); //Good

		
		sgx_cmac_128bit_tag_t* CMACbySMK = (sgx_cmac_128bit_tag_t*) malloc(sizeof(sgx_cmac_128bit_tag_t));
		sgx_cmac_state_handle_t* cmacHandler = (sgx_cmac_state_handle_t*) malloc(sizeof(sgx_cmac_state_handle_t));
		checkStat(sgx_cmac128_init(SMK, cmacHandler));
		checkStat(sgx_cmac128_update((uint8_t*) (publicKey->gx), 32, *cmacHandler));
		checkStat(sgx_cmac128_update((uint8_t*) (publicKey->gy), 32, *cmacHandler));
		checkStat(sgx_cmac128_update((uint8_t*) SPID, 16, *cmacHandler));
		checkStat(sgx_cmac128_update((uint8_t*) &quoteType, 2, *cmacHandler));
		checkStat(sgx_cmac128_update((uint8_t*) &KDF_ID, 2, *cmacHandler));
		checkStat(sgx_cmac128_update((uint8_t*) &(signature->x), 2, *cmacHandler));
		checkStat(sgx_cmac128_update((uint8_t*) &(signature->y), 2, *cmacHandler));
		checkStat(sgx_cmac128_final(*cmacHandler, CMACbySMK));
		
		//Query IAS to obtain the SigRL
		char* respBody = (char*) malloc(0); //First time should not have SigRL, so size should be zero.
		requestSignRL(&(msg->gid), respBody);
		//printString("The SigRL is ");
		//printString(respBody);

		//Create msg2 to return to client
		msg2->g_b = *publicKey;
		for (int i = 0; i < 16; i++)
			(msg2->spid).id[i] = SPID[i];
		msg2->quote_type = quoteType;
		msg2->kdf_id = KDF_ID;
		msg2->sign_gb_ga = *signature;
		memcpy(msg2->mac, *CMACbySMK, 16); //for (int i = 0; i < 16; i++) (msg2->mac)[i] = *CMACbySMK[i];
		msg2->sig_rl_size = 0;
		memcpy(msg2->sig_rl, respBody, 0);
	}
}
