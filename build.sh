#!/usr/bin/env bash

CXX=g++
CXXFLAGS="-Werror
    -fsanitize=address,undefined
    -Ideps
    -Ideps/imgui

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

BUILD_DIR="build"

LIBS="-lGL $(pkg-config --libs --cflags --static glfw3)"

SRCS="src/main.cpp"
EXE="${BUILD_DIR}/elid"

DEAR_IMGUI_BIN="${BUILD_DIR}/dear-imgui.o"

cd $(dirname $0)
mkdir -p ${BUILD_DIR};

if [ ! -f ${DEAR_IMGUI_BIN} ]; then
    echo "[compiling Dear Imgui...]"
    ${CXX} src/dear-imgui/dear_imgui.cpp ${LIBS} -Ideps/imgui -Ideps/imgui/backends -c -o ${DEAR_IMGUI_BIN}
fi

${CXX} ${CXXFLAGS} ${SRCS} ${DEAR_IMGUI_BIN} ${LIBS} -o ${EXE}
