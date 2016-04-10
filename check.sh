#!/bin/bash
set -eu

if [ $(uname -s) == "Darwin" ]; then
  CORES=$(sysctl -n hw.logicalcpu)
  SED=gsed
else
  CORES=$(nproc)
  SED=sed
  find src -name "*.cpp" -or -name "*.hpp" | xargs $SED -i -e 's/\r//'
fi

if [ ! -d "bin" ]; then
  mkdir bin
fi

make -j$CORES
PATH=$PATH:$PWD/extra/bin:/mingw64/bin \
LD_LIBRARY_PATH=$PWD/extra/lib${LD_LIBRARY_PATH:+:}${LD_LIBRARY_PATH:-} \
DYLD_LIBRARY_PATH=$PWD/extra/lib${DYLD_LIBRARY_PATH:+:}${DYLD_LIBRARY_PATH:-} \
make run
