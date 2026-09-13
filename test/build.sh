#!/usr/bin/env sh


CXX=g++
CXXFLAGS="
    -DDEBUG_BUILD=1
    -DPOISON_MEMORY=1
    -I../src
    -I../deps/memmi/include
    -I../deps/memmi/include/memmi
    -Werror
    -fsanitize=thread
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
${CXX} ${CXXFLAGS} ${GENERATED_TEST_RUNNER} -o ${BIN} ${MEMMI_SRCS}
