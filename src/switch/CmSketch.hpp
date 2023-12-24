#include "sketch.hpp"

class CmSketch : public Sketch {
   private:
    uint32_t m_hashNum;
    uint32_t m_width;
    std::vector<uint32_t> m_bitmap;
    uint32_t m_max_num;

   public:
    CmSketch(uint32_t hashNum, uint32_t width)
        : m_hashNum(hashNum),
          m_width(width),
          m_bitmap(m_hashNum * m_width, 0),
          m_max_num(0) {
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
                m_bitmap[i * m_width + row_idx]++;
            m_max_num = std::max(m_max_num, m_bitmap[i * m_width + row_idx]);
        }
    }

    std::vector<uint32_t> getBitmap() { return m_bitmap; }

    std::pair<uint32_t, uint32_t> getBitmapSize() {
        return {m_hashNum, m_width};
    }

    uint32_t getMaxNum() { return m_max_num; }
};