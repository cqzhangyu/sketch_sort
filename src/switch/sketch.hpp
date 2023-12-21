#pragma once
#include <cmath>
#include <limits>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <iostream>

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
};

class CmSketch : public Sketch {
   private:
    uint32_t m_hashNum;
    uint32_t m_width;
    std::vector<uint32_t> m_bitmap;
    uint32_t m_max_num;

   public:
    CmSketch(uint32_t hashNum, uint32_t width)
        : m_hashNum(hashNum), m_width(width), m_bitmap(m_hashNum * m_width, 0), m_max_num(0) {
        if (m_hashNum > m_seeds.size()) {
            throw std::runtime_error("Too many hash functions");
        }
    }

    uint32_t query(uint64_t key) {
        uint32_t res = std::numeric_limits<uint32_t>::max();
        for (int i = 0; i < m_hashNum; ++i) {
            uint32_t row_idx = hash(key, m_seeds[i]) % m_width;
            res = std::min(res, m_bitmap[i * m_width + row_idx]);
        }
        return res;
    }

    void update(uint64_t key) {
        uint32_t m_max_val = std::numeric_limits<uint32_t>::max();
        for (int i = 0; i < m_hashNum; ++i) {
            uint32_t row_idx = hash(key, m_seeds[i]) % m_width;
            if (m_bitmap[i * m_width + row_idx] != m_max_val)
                m_bitmap[i * m_width + row_idx] ++;
            m_max_num = std::max(m_max_num, m_bitmap[i * m_width + row_idx]);
        }
    }

    std::vector<uint32_t> getBitmap() { return m_bitmap; }

    std::pair<uint32_t, uint32_t> getBitmapSize() { return {m_hashNum, m_width}; }

    uint32_t getMaxNum() { return m_max_num; }
};
