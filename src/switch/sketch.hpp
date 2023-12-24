#pragma once
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

class Sketch {
   protected:
    std::vector<uint64_t> m_seeds = {0xdeadbeef, 0xcafebabe, 0x12345678,
                                     0x87654321, 0x12341234, 0x43214321,
                                     0x87658765, 0x56785678};

    static inline uint64_t hash(uint64_t key, uint64_t seed) {
        uint64_t res = seed;
        uint8_t a0 = (key >> 0) & 0xff;
        uint8_t a1 = (key >> 8) & 0xff;
        uint8_t a2 = (key >> 16) & 0xff;
        uint8_t a3 = (key >> 24) & 0xff;
        uint8_t a4 = (key >> 32) & 0xff;
        uint8_t a5 = (key >> 40) & 0xff;
        uint8_t a6 = (key >> 48) & 0xff;
        uint8_t a7 = (key >> 56) & 0xff;
        res = res * seed + a7;
        res = res * seed + a6;
        res = res * seed + a5;
        res = res * seed + a4;
        res = res * seed + a3;
        res = res * seed + a2;
        res = res * seed + a1;
        res = res * seed + a0;
        return res;
    }

   public:
    virtual uint32_t query(uint64_t key) = 0;
    virtual void update(uint64_t key) = 0;
};