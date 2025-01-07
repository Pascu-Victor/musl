#!/bin/bash

set -e

cmake -GNinja -B build -DCMAKE_C_ABI_COMPILED=ON
cmake --build build
# cmake --install build --prefix ./musl-build
# CC=../../../tools/llvm-build/bin/clang LD=../../../tools/llvm-build/bin/ld.lld make -j32
# CC=../../../tools/llvm-build/bin/clang LD=../../../tools/llvm-build/bin/ld.lld make install
