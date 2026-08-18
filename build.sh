#!/usr/bin/env bash

CXX=g++
CXXFLAGS="-Werror
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
CXXFLAGS_DEBUG="-DDEBUG_BUILD=1 -DPOISON_MEMORY=1"

CXXFLAGS="${CXXFLAGS} ${CXXFLAGS_DEBUG}"

SRCS="src/main.cpp"

BUILD_DIR="build"
BIN="${BUILD_DIR}/elid"

cd $(dirname $0)
mkdir -p ${BUILD_DIR};
${CXX} ${CXXFLAGS} ${SRCS} -o ${BIN}
