#!/bin/bash

for thresh in 100 75 50 25; do
    ./build/switch_sort -n 33554432 -g zipf -s elastic -w 100 --threshold $thresh --heavy_depth 2 --heavy_width 32
done