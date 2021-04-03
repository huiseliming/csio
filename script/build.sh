#!/bin/bash
cd `dirname $0`
# build in unix like system?
# cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B out/build
# build macOS-Universal
# cmake -DCMAKE_OSX_ARCHITECTURES=arm64;x86_64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B out/build
# build macOS-arm64
cmake -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S .. -B ../build
# build macOS-x86_64
cd ../build
make 