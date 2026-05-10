#!/bin/bash

sudo rm -rf /usr/local/include/lumina /usr/local/lib/cmake/lumina

cmake -B build -S . && sudo cmake --install build
