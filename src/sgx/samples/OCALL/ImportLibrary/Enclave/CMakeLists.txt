set(SOURCE_FILES
        Enclave_t.c
	Enclave.cpp
	print.cpp
        )

set(SOURCE_FILES ${SOURCE_FILES})

set(CMAKE_VERBOSE_MAKEFILE False)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_t.c
        COMMAND ${SGX_EDGER8R} --trusted ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.edl
	--trusted ${CMAKE_CURRENT_SOURCE_DIR}/print.edl
        --search-path ${CMAKE_CURRENT_SOURCE_DIR}
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(enclave SHARED ${SOURCE_FILES})

if (SGX_MODE STREQUAL HW)
    set(TRTS_LIB_NAME sgx_trts)
    set(SERVICE_LIB_NAME sgx_tservice)
else()
    set(TRTS_LIB_NAME sgx_trts_sim)
    set(SERVICE_LIB_NAME sgx_tservice_sim)
endif()

set(CRYPTO_LIB_NAME sgx_tcrypto)

include_directories(
        ${SGX_SDK}/include
        ${SGX_SDK}/include/tlibc
        ${SGX_SDK}/include/stlport
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENCLAVE_STD_ALT ${SGX_COMMON_CFLAGS} \
    -nostdinc -fvisibility=hidden -fpie -fstack-protector")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++03 -nostdinc++")

set_target_properties(enclave PROPERTIES PREFIX "")

target_link_libraries(enclave "${SGX_COMMON_CFLAGS} \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGX_LIBRARY_PATH} \
    -Wl,--whole-archive -l${TRTS_LIB_NAME} -Wl,--no-whole-archive \
    -Wl,--start-group -lsgx_tstdc -lsgx_tstdcxx -l${CRYPTO_LIB_NAME} -l${SERVICE_LIB_NAME} -Wl,--end-group \
    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
    -Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
    -Wl,--defsym,__ImageBase=0")


add_custom_target(signed.enclave
        ${SGX_ENCLAVE_SIGNER} sign -key ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_private.pem
        -enclave ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/enclave.so
        -config ${CMAKE_CURRENT_SOURCE_DIR}/Enclave.config.xml
        -out ${PROJECT_BINARY_DIR}/signed.so)

add_dependencies(signed.enclave enclave)
