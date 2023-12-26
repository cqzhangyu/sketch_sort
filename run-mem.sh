#!/bin/bash

runrandom() {
    for width in 2000 4000 8000 16000; do
        heavy_width=$((width * 3 / 10))
        for gen in random; do
            for sketch in cm tower elastic; do
                echo "./build/switch_sort -n 33554432 -g $gen -s $sketch -w $width --threshold 300000 --heavy_depth 2 --heavy_width $heavy_width"
                ./build/switch_sort -n 33554432 -g $gen -s $sketch -w $width --threshold 300000 --heavy_depth 2 --heavy_width $heavy_width
            done
        done
    done
}

runzipf() {
    for width in 400 800 1600 3200; do
        heavy_width=$((width * 3 / 10))
        for gen in zipf; do
            for sketch in cm tower elastic; do
                echo "./build/switch_sort -n 33554432 -g $gen -s $sketch -w $width --threshold 300000 --heavy_depth 2 --heavy_width $heavy_width"
                ./build/switch_sort -n 33554432 -g $gen -s $sketch -w $width --threshold 300000 --heavy_depth 2 --heavy_width $heavy_width
            done
        done
    done
}


runexponential() {
    for width in 300000 600000 1200000; do
        heavy_width=$((width * 3 / 10))
        for gen in exponential; do
            for sketch in cm tower elastic; do
                echo "./build/switch_sort -n 33554432 -g $gen -s $sketch -w $width --threshold 300000 --heavy_depth 2 --heavy_width $heavy_width"
                ./build/switch_sort -n 33554432 -g $gen -s $sketch -w $width --threshold 300000 --heavy_depth 2 --heavy_width $heavy_width
            done
        done
    done
}

runexponential
