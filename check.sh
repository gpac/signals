#!/bin/bash
set -eu

find modules mm signals -name "*.cpp" -or -name "*.hpp" | xargs dos2unix -k -q
make -j`nproc`
make run
