#!/bin/bash
set -eu

make -j`nproc`
make run
