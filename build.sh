#!/bin/sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
make -C build -j$(nproc)
