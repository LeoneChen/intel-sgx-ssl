#!/bin/bash
set -e

CUR_DIR=$(realpath $(dirname $0))
if [ -f ${CUR_DIR}/openssl_source/openssl/Makefile ]
then
    make -C ${CUR_DIR}/openssl_source/openssl distclean -s
fi
make -C ${CUR_DIR}/Linux clean -s
make -C ${CUR_DIR}/Linux clean -s DEBUG=1
rm -rf ${CUR_DIR}/Linux/package/lib64/* ${CUR_DIR}/Linux/package/include/crypto/ ${CUR_DIR}/Linux/sgx/test_app/InstrumentStatistics.json
