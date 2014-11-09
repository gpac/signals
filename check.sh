#!/bin/bash
set -eu

find modules/internal modules/src mm signals -name "*.cpp" -or -name "*.hpp" | xargs dos2unix -k -q
make -j`nproc`
make run
