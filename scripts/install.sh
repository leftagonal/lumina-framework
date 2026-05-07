#!/bin/bash

rm -rf build
cmake -B build
sudo cmake --install build