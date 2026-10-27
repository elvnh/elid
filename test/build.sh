#!/usr/bin/env sh

CC=gcc
CXX=g++
CXXFLAGS="
    -DDEBUG_BUILD=1
    -DPOISON_MEMORY=1
    -I../src
    -I../deps/memmi/include
    -I../deps/memmi/include/memmi
    -I../deps/
    -Werror
    -fsanitize=address,undefined
    -std=c++11
    -Wall
    -Wextra
    -pedantic
    -Wpedantic
    -ggdb
    -Wshadow
    -Wcast-align
    -Wunused
    -Wconversion
    -Wsign-conversion
    -Wsign-compare
    -Wnull-dereference
    -Wdouble-promotion
    -Wlogical-not-parentheses
    -Wformat=2
    -Wformat-signedness
    -Wcast-align
    -Werror=return-type
    -Werror=overflow
    -Wsign-conversion
    -Wtype-limits
    -Wundef
    -Werror=missing-braces
    -Wno-error=unused-parameter
    -Wno-error=unused-function
    -Wno-error=unused-variable
    -Wno-error=unused-but-set-variable
"

GENERATED_TEST_RUNNER="src/generated/test_runner.cpp"
BUILD_DIR="build"
BIN="${BUILD_DIR}/elid-tests"
MEMMI_SRCS="../deps/memmi/src/memmi.c"

cd $(dirname $0);

mkdir -p ${BUILD_DIR};
mkdir -p $(dirname ${GENERATED_TEST_RUNNER});

python3 generate_test_runner.py src/**/* -o ${GENERATED_TEST_RUNNER} &&

# Compile Zydis in C mode as it is not valid for C++ mode
ZYDIS_OBJ="${BUILD_DIR}/Zydis.o"
${CC} ../deps/Zydis/Zydis.c -c -o ${ZYDIS_OBJ} -I../deps/Zydis

${CXX} ${CXXFLAGS} ${GENERATED_TEST_RUNNER} -o ${BIN} ${MEMMI_SRCS} ${ZYDIS_OBJ}
