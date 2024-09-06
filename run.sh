#!/bin/bash
set -e

cmake -B build || { echo "CMake configuration failed"; exit 1; }

make -C build || { echo "Make build failed"; exit 1; }

echo "--------Result--------"

./build/simpjson || { echo "Program execution failed"; exit 1; }
