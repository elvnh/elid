#!/usr/bin/env sh


CXX=g++
CXXFLAGS="
    -DDEBUG_BUILD=1
    -I../src
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
    -Werror=missing-braces
    -Wno-error=unused-parameter
    -Wno-error=unused-function
    -Wno-error=unused-variable
    -Wno-error=unused-but-set-variable
"

SRCS="src/main.cpp"

BUILD_DIR="build"
BIN="${BUILD_DIR}/elid-tests"

cd $(dirname $0);
mkdir -p ${BUILD_DIR};
${CXX} ${CXXFLAGS} ${SRCS} -o ${BIN}
