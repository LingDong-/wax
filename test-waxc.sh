#!/bin/sh
waxfn=examples/traceskeleton
valgrind ./waxc \
    --c $waxfn.c \
    --cpp $waxfn.cpp \
    --cs $waxfn.cs \
    --java $waxfn.java \
    --json $waxfn.json \
    --lua $waxfn.lua \
    --py $waxfn.py \
    --swift $waxfn.swift \
    --ts $waxfn.ts \
    --wat $waxfn.wat \
    $waxfn.wax
