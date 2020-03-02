#!/usr/bin/env bash

git clone https://github.com/google/googletest.git
cd googletest

mkdir install
cd install

cmake -DCMAKE_CXX_FLAGS="-std=c++11" ../
make
sudo make install
