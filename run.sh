#!/bin/bash

for thresh in 300000 100000 20000 4000 800; do
    ./build/switch_sort -n 33554432 -g random -s elastic -w 3000000 --threshold $thresh
done