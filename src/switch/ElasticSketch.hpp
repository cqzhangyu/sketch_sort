#pragma once
#include "sketch.hpp"
#include "TowerSketch.hpp"

struct ES_heavy_t {
    uint64_t key;
    uint32_t positive;
    uint32_t all;
    ES_heavy_t () {
        key = 0;
        positive = 0;
        all = 0;
    }
};

class ElasticSketch : public Sketch {
   private:
    uint32_t m_heavy_width;
    uint32_t m_heavy_depth;
    uint32_t m_hash_num;
    double m_lambda;
    TowerSketch light;
    std::vector<ES_heavy_t> heavy;

   public:
    ElasticSketch(uint32_t hashNum, uint32_t width, uint32_t heavyDepth, uint32_t heavyWidth, double lambda)
        : light(hashNum, width) {
        m_heavy_depth = heavyDepth;
        m_heavy_width = heavyWidth;
        m_hash_num = hashNum;
        m_lambda = lambda;
        heavy.resize(heavyDepth * heavyWidth);
    }

    uint32_t query(uint64_t key) {
        uint32_t res = 0;
        for (uint32_t i = 0; i < m_heavy_depth; i ++) {
            uint32_t heavy_idx = i * m_heavy_width + hash(key, m_seeds[m_hash_num + i]) % m_heavy_width;
            if (heavy[heavy_idx].key == key) {
                res += heavy[heavy_idx].positive;
            }
        }
        return res + light.query(key);
    }

    void update(uint64_t key) {
        uint32_t val = 1;
        for (uint32_t i = 0; i < m_heavy_depth; i ++) {
            uint32_t heavy_idx = i * m_heavy_width + hash(key, m_seeds[m_hash_num + i]) % m_heavy_width;
            heavy[heavy_idx].all += val;
            if (heavy[heavy_idx].key == key) {
                // hit
                heavy[heavy_idx].positive += val;
                return ;
            }
            else {
                if (heavy[heavy_idx].all > heavy[heavy_idx].positive * m_lambda) {
                    uint64_t old_key = heavy[heavy_idx].key;
                    uint32_t old_val = heavy[heavy_idx].positive;

                    // update the heavy bucket
                    heavy[heavy_idx].positive = heavy[heavy_idx].positive + 1;
                    heavy[heavy_idx].key = key;

                    // evict old key to the light part
                    key = old_key;
                    val = old_val;
                }
                else {
                    // move to the next stage
                }
            }
        }
        
        light.update(key, val);
    }

    uint32_t getMaxNum() {
        uint32_t m_max_num = 0;
        for (uint32_t i = 0; i < m_heavy_depth * m_heavy_width; i ++) {
            m_max_num = std::max(m_max_num, heavy[i].positive);
        }
        return light.getMaxNum();
    }
};
