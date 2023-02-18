#!/bin/bash

SCRIPT_PATH="${0%/*}"
cd $SCRIPT_PATH

mkdir .bin

FSTDVERSx="0.1"
VERSION="010"

echo FSTDVERSx="${FSTDVERSx}" VERSION="${VERSION}"  # see fst2fce.cpp

SRC="./fcec-OpAddHelperPart.cpp"
DEST="fcec-OpAddHelperPart${VERSION}"

# compiler-specific
MINGWFLAGS="-fPIE -s -O2 -Xlinker --no-insert-timestamp" #  -fstack-clash-protection -Wl,-pie
GCCFLAGS="-fPIE -fstack-clash-protection -fstack-protector-strong -D_FORTIFY_SOURCE=2 -s -O2"
CPPFLAGS="-D_GLIBCXX_ASSERTIONS -fPIE -fstack-clash-protection -fstack-protector-strong -D_FORTIFY_SOURCE=2 -s -O2"

# debug
GCCDEBUGFLAGS="-pedantic-errors -g -Wall -Wextra -Wstack-protector -fasynchronous-unwind-tables" # -fsanitize=leak
CPPDEBUGFLAGS="-g -Wall -Wextra -Wstack-protector -fasynchronous-unwind-tables" # -fsanitize=leak
GDB="-Og"

echo g++
g++ -std=c++17 -DFSTDVERSx="${FSTDVERSx}" $CPPFLAGS $SRC -o .bin/$DEST"_linux_c++"
echo clang++
clang++ -std=c++17 -DFSTDVERSx="${FSTDVERSx}" $CPPFLAGS $SRC -o .bin/$DEST"_linux_clang++"
