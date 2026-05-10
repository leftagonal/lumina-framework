#!/bin/bash

sudo rm -rf /opt/local/include/lumina /opt/local/lib/cmake/lumina

cmake -B build -S . && sudo cmake --install build
