#!/bin/bash
set -e

if [[ $1 == 'debug' ]]; then
    DEBUG_FLAG=1
else
    DEBUG_FLAG=0
fi

make -C Linux sgxssl_no_mitigation DEBUG=${DEBUG_FLAG} SGX_MODE=HW 

# ~/SGXSan/Tool/GetLayout.sh -d Linux/sgx/test_app enclave/TestEnclave_t.o enclave/TestEnclave.o enclave/tests/ecdhtest.o enclave/tests/sha1test.o enclave/tests/missing_funcs.o enclave/tests/threadstest.o enclave/tests/stdio_func.o enclave/tests/dhtest.o enclave/tests/ecdsatest.o enclave/tests/rsa_test.o enclave/tests/bntest.o enclave/tests/sha256t.o enclave/tests/ectest.o enclave/tests/evp_smx.o /mnt/hdd/sgx-evaluate/sgxfuzz/Enclaves/intel-sgx-ssl/Linux/package/lib64/libsgx_tsgxssl.a /mnt/hdd/sgx-evaluate/sgxfuzz/Enclaves/intel-sgx-ssl/Linux/package/lib64/libsgx_tsgxssl_crypto.a /opt/intel/sgxsdk/lib64/libsgx_trts.a /opt/intel/sgxsdk/lib64/libsgx_tstdc.a /opt/intel/sgxsdk/lib64/libsgx_pthread.a /opt/intel/sgxsdk/lib64/libsgx_tcxx.a /opt/intel/sgxsdk/lib64/libsgx_tcrypto.a /opt/intel/sgxsdk/lib64/libsgx_tservice.a
