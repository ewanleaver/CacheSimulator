#!/bin/bash

# Build script, Ewan Leaver

CC="g++" #gcc
OS_TYPE="Darwin" #DICE

$CC -o sim line.cpp proc.cpp sim.cpp  -O0 -g

# lineCount, lineSize, writeBufSize, retireAtN, TSO

./sim trace1.out 128 4 64 1 1