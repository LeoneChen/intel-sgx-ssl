#!/bin/bash
set -e

for ARG in "$@"
do
   KEY="$(echo $ARG | cut -f1 -d=)"
   VAL="$(echo $ARG | cut -f2 -d=)"
   export "$KEY"="$VAL"
done

TOP_DIR=$(realpath -s $(dirname $0))
MAKE_FLAGS="CC=clang-13 CXX=clang++-13 AR=llvm-ar-13 OPT=opt-13 LD=lld OBJCOPY=objcopy VERBOSE=1 SGX_MODE=SIM SGX_SDK=$(realpath -s ${TOP_DIR}/../install)"
MODE=${MODE:="RELEASE"}
FUZZER=${FUZZER:="LIBFUZZER"}

echo "-- MODE: ${MODE}"
echo "-- FUZZER: ${FUZZER}"

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

echo "-- MAKE_FLAGS: ${MAKE_FLAGS}"

make -C Linux sgxssl_no_mitigation ${MAKE_FLAGS}
