#!/bin/bash
clear

SCRIPT_PATH="${0%/*}"
cd $SCRIPT_PATH

python ci-diff.py

echo ""
echo "compare src->X with src->X->X"
cmp   .out/ci-smoketest3.fce  .out/ci-smoketest3.fcediff33
cmp   .out/ci-smoketest4.fce  .out/ci-smoketest4.fcediff44
cmp   .out/ci-smoketest4m.fce .out/ci-smoketest4m.fcediff4m4m
cksum .out/ci-smoketest3.fce  .out/ci-smoketest3.fcediff33
cksum .out/ci-smoketest4.fce  .out/ci-smoketest4.fcediff44
cksum .out/ci-smoketest4m.fce .out/ci-smoketest4m.fcediff4m4m

echo ""
echo "compare src with src->4"
cmp   fce/Snowman_car.fce     .out/ci-smoketest4.fce
cksum fce/Snowman_car.fce     .out/ci-smoketest4.fce

echo ""
echo "compare src->4 with src->4m->4"
cmp   .out/ci-smoketest4.fce .out/ci-smoketest4.fcediff4m4
cksum .out/ci-smoketest4.fce .out/ci-smoketest4.fcediff4m4