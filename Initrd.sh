#!/bin/bash
CUR_DIR=$(realpath .)
SGXSAN_DIR=$(realpath ${CUR_DIR}/../)

${SGXSAN_DIR}/install/initrd/gen_initrd.sh ${CUR_DIR}/target.cpio.gz ${CUR_DIR}/Linux/sgx/test_app/TestApp ${CUR_DIR}/Linux/sgx/test_app/TestEnclave.so ${SGXSAN_DIR}/install/rand_file ${SGXSAN_DIR}/install/bin/vmcall /usr/bin/addr2line /usr/bin/gdbserver