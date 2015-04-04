#!/bin/bash
set -eu

find src -name "*.cpp" -or -name "*.hpp" | xargs dos2unix -k -q

if [ ! -d "bin" ]; then
  mkdir bin
fi
make -j`nproc`
make run
