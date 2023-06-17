#!/bin/bash
set -e

for ARG in "$@"
do
   KEY="$(echo $ARG | cut -f1 -d=)"
   VAL="$(echo $ARG | cut -f2 -d=)"
   export "$KEY"="$VAL"
done

TOP_DIR=$(realpath $(dirname $0))
MAKE_FLAGS="CC=clang-13 CXX=clang++-13 SGX_SDK=$(realpath ${TOP_DIR}/../install) SKIP_INTELCPU_CHECK=TRUE VERBOSE=1"
MODE=${MODE:="RELEASE"}
FUZZER=${FUZZER:="LIBFUZZER"}
SIM=${SIM:="TRUE"}

echo "-- MODE: ${MODE}"
echo "-- FUZZER: ${FUZZER}"
echo "-- SIM: ${SIM}"

if [[ "${MODE}" = "DEBUG" ]]
then
    MAKE_FLAGS+=" DEBUG=1"
else
    MAKE_FLAGS+=" DEBUG=0"
fi

if [[ "${FUZZER}" = "KAFL" ]]
then
    MAKE_FLAGS+=" KAFL_FUZZER=1"
else
    MAKE_FLAGS+=" KAFL_FUZZER=0"
fi

if [[ "${SIM}" = "TRUE" ]]
then
    MAKE_FLAGS+=" SGX_MODE=SIM"
else
    MAKE_FLAGS+=" SGX_MODE=HW"
fi

echo "-- MAKE_FLAGS: ${MAKE_FLAGS}"

make -C Linux sgxssl_no_mitigation ${MAKE_FLAGS}
