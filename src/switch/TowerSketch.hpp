#pragma once
#include "sketch.hpp"

class TowerSketch : public Sketch {
   private:
    uint32_t m_hashNum;
    uint32_t m_width;
    std::vector<uint32_t> m_bitmap;
    uint32_t *m_row_max;

    uint32_t m_max_num;

   public:
    TowerSketch(uint32_t hashNum, uint32_t width)
        : m_hashNum(hashNum), m_width(width), m_bitmap(m_width * (1 + 2 + 4), 0), m_max_num(0) {
        if (m_hashNum > m_seeds.size()) {
            throw std::runtime_error("Too many hash functions");
        }
        if (m_hashNum != 3) {
            throw std::runtime_error("hashNum must be 3");
        }
        m_row_max = new uint32_t[m_hashNum];
        m_row_max[0] = 0xffffffffu;
        m_row_max[1] = 0xffffu;
        m_row_max[2] = 0xffu;
    }

    uint32_t query(uint64_t key) {
        uint32_t res = std::numeric_limits<uint32_t>::max();
        uint32_t row_width = m_width;
        uint32_t row_begin = 0;
        for (int i = 0; i < m_hashNum; ++i) {
            uint32_t row_idx = hash(key, m_seeds[i]) % row_width;
            uint32_t val = m_bitmap[row_begin + row_idx];
            if (val != m_row_max[i]) {
                res = std::min(res, val);
            }
            row_begin += row_width;
            row_width *= 2;
        }
        return res;
    }

    void update(uint64_t key, uint32_t val) {
        uint32_t row_width = m_width;
        uint32_t row_begin = 0;
        for (int i = 0; i < m_hashNum; ++i) {
            uint32_t row_idx = hash(key, m_seeds[i]) % row_width;
            if ((uint64_t)m_bitmap[row_begin + row_idx] + val < (uint64_t)m_row_max[i]) {
                m_bitmap[row_begin + row_idx] += val;
            }
            else {
                m_bitmap[row_begin + row_idx] = m_row_max[i];
            }
            row_begin += row_width;
            row_width *= 2;
        }
    }
    
    void update(uint64_t key) {
        update(key, 1);
    }

    std::vector<uint32_t> getBitmap() { return m_bitmap; }

    std::pair<uint32_t, uint32_t> getBitmapSize() { return {m_hashNum, m_width}; }

    uint32_t getMaxNum() {
        for (size_t i = 0; i < m_bitmap.size(); i ++) {
            m_max_num = std::max(m_max_num, m_bitmap[i]);
        }
        return m_max_num;
    }
};