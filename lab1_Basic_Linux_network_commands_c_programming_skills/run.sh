#!/usr/bin/env bash
set -e
set -x

say() { echo "$@"; }

{ set +x; } 2>/dev/null
say "Running Program 1 (Standard I/O) on src1.dat and src2.dat"
set -x
./prog1 src1.dat dest1.dat
./prog1 src2.dat dest2.dat

{ set +x; } 2>/dev/null
say "Showing file sizes for Program 1 outputs"
set -x
ls -la src1.dat dest1.dat
ls -la src2.dat dest2.dat

{ set +x; } 2>/dev/null
say "Verifying byte-for-byte correctness for Program 1 outputs"
set -x
cmp src1.dat dest1.dat
cmp src2.dat dest2.dat

{ set +x; } 2>/dev/null
say "Removing Program 1 destination files"
set -x
rm -f dest1.dat dest2.dat

{ set +x; } 2>/dev/null
say "Running Program 2 (System Calls) on src1.dat and src2.dat"
set -x
./prog2 src1.dat dest1_sys.dat
./prog2 src2.dat dest2_sys.dat

{ set +x; } 2>/dev/null
say "Showing file sizes for Program 2 outputs"
set -x
ls -la src1.dat dest1_sys.dat
ls -la src2.dat dest2_sys.dat

{ set +x; } 2>/dev/null
say "Verifying byte-for-byte correctness for Program 2 outputs"
set -x
cmp src1.dat dest1_sys.dat
cmp src2.dat dest2_sys.dat

{ set +x; } 2>/dev/null
say "Done"
set -x
