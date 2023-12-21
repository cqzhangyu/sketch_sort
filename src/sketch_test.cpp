#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>

#include "common/cmdline.h"
#include "common/utils.hpp"
#include "switch/ElasticSketch.hpp"
#include "switch/sketch.hpp"
#include "switch/TowerSketch.hpp"
#include "generator/generator.hpp"

int main(int argc, char **argv) {
    cmdline::parser parser;
    parser.add<int>("num", 'n', "number of elements", false, 65536);
    parser.add<std::string>("gen", 'g', "generator type", false, "random");
    parser.add<int>("width", 'w', "sketch width", false, 65536);
    parser.add<int>("heavy_depth", 0, "heavy part depth of Elastic Sketch", false, 4);
    parser.add<int>("heavy_width", 0, "heavy part width of Elastic Sketch", false, 4096);
    parser.add<double>("lambda", 0, "lambda in Elastic Sketch", false, 32);

    parser.parse(argc, argv);

    int n = parser.get<int>("num");
    std::string genstr = parser.get<std::string>("gen");
    int width = parser.get<int>("width");
    int heavy_depth = parser.get<int>("heavy_depth");
    int heavy_width = parser.get<int>("heavy_width");
    double lambda = parser.get<double>("lambda");

    CmSketch cms(3, width);
    TowerSketch tower(3, width);
    if (heavy_depth * heavy_width > width) {
        fprintf(stderr, "error! heavy must < width!\n");
        exit(1);
    }
    ElasticSketch es(3, width - heavy_depth * heavy_width, heavy_depth, heavy_width, lambda);
    std::set<uint64_t> keys;

    uint64_t *arr = new uint64_t[n];
    if (genstr == "random") {
        GenRandom gen;
        gen(arr, arr + n);
    }
    else if (genstr == "exponential") {
        GenExponential gen;
        gen(arr, arr + n);
    }
    else if (genstr == "zipf") {
        GenZipf gen;
        gen(arr, arr + n);
    }

    for (int i = 0; i < n; i ++) {
        cms.update(arr[i]);
        tower.update(arr[i]);
        es.update(arr[i]);
        keys.insert(arr[i]);
    }

    uint64_t sum_cms = 0;
    uint64_t sum_tower = 0;
    uint64_t sum_es = 0;
    
    for (uint64_t key : keys) {
        sum_cms += cms.query(key);
        sum_tower += tower.query(key);
        sum_es += es.query(key);
    }
    printf("Correct : %d\n", n);
    printf("CMS : %lu\n", sum_cms);
    printf("Tower : %lu\n", sum_tower);
    printf("Elastic : %lu\n", sum_es);
    return 0;
}

