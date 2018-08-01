set(SOURCE_FILES
        App.cpp
        Enclave_u.c
	sgx_utils.cpp
        )

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave_u.c
        COMMAND ${SGX_EDGER8R} --untrusted ${CMAKE_CURRENT_SOURCE_DIR}/../Enclave/Enclave.edl --untrusted ${CMAKE_CURRENT_SOURCE_DIR}/../Enclave/print.edl
        --search-path ${SGX_SDK}/include
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


add_executable(app ${SOURCE_FILES})

if (SGX_MODE STREQUAL HW)
    set(Urts_Library_Name sgx_urts)
    set(Trts_Library_Name sgx_trts)
    set(SERVICE_LIB_NAME sgx_tservice)
else()
    set(Urts_Library_Name sgx_urts_sim)
    set(Trts_Library_Name sgx_trts_sim)
    set(SERVICE_LIB_NAME sgx_tservice_sim)
endif()

include_directories(
        ${SGX_SDK}/include
    )

set(CRYPTO_LIB_NAME sgx_tcrypto)

if(SGX_DEBUG STREQUAL 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -UNDEBUG -UEDEBUG")
elseif(SGX_PRERELEASE STREQUAL 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -UDEBUG -DNDEBUG -DEDEBUG")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -UDEBUG -DNDEBUG -UEDEBUG")
endif()

if(SGX_MODE STREQUAL HW)
    set(UAE_SERVICE_LIB sgx_uae_service)
else()
    set(UAE_SERVICE_LIB sgx_uae_service_sim)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SGX_COMMON_CFLAGS} -fPIC -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11")

target_link_libraries(app "-Wl,-L${SGX_LIBRARY_PATH} \
        -Wl,--start-group -l${Urts_Library_Name} -l${UAE_SERVICE_LIB} \
	-Wl,--whole-archive -lsgx_uprotected_fs -Wl,--no-whole-archive \
        -Wl,-lpthread \
        -Wl,-lboost_system \
        -Wl,-lboost_filesystem \
        -Wl,-lboost_program_options")

add_dependencies(app signed.enclave)

install(TARGETS app DESTINATION ${PROJECT_BINARY_DIR})
