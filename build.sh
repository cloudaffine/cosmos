#!/usr/bin/env sh

cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" -B build/release .
cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" -B build/debug .

cd build/debug
make