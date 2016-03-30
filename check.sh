#!/bin/bash
set -eu

find src -name "*.cpp" -or -name "*.hpp" | xargs sed -i -e 's/\r//'

if [ ! -d "bin" ]; then
  mkdir bin
fi
make -j`nproc`
PATH=$PATH:$PWD/extra/bin:/mingw64/bin make run
