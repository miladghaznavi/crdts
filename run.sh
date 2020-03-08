#!/usr/bin/env bash

mkdir build
cd build

cmake ..
make
./crdts_test
