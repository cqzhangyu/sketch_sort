#pragma once
#include "sketch.hpp"
#include "TowerSketch.hpp"

struct ES_heavy_t {
    uint64_t key;
    uint32_t positive;
    uint32_t all;
    bool flag;
    ES_heavy_t () {
        key = 0;
        positive = 0;
        all = 0;
        flag = true;
    }
};

class ElasticSketch : public Sketch {
   private:
    uint32_t m_heavy_size;
    uint32_t m_hash_num;
    double m_lambda;
    TowerSketch light;
    std::vector<ES_heavy_t> heavy;

   public:
    ElasticSketch(uint32_t hashNum, uint32_t width, uint32_t heavyNum, double lambda)
        : light(hashNum, width) {
        m_heavy_size = heavyNum;
        m_hash_num = hashNum;
        m_lambda = lambda;
        heavy.resize(heavyNum);
    }

    uint32_t query(uint64_t key) {
        uint32_t heavy_idx = hash(key, m_seeds[m_hash_num]) % m_heavy_size;
        if (heavy[heavy_idx].key == key) {
            if (heavy[heavy_idx].flag) {
                // no error
                return heavy[heavy_idx].positive;
            }
            else {
                // has error
                return heavy[heavy_idx].positive + light.query(key);
            }
        }
        else {
            return light.query(key);
        }
    }

    void update(uint64_t key) {
        uint32_t heavy_idx = hash(key, m_seeds[m_hash_num]) % m_heavy_size;
        heavy[heavy_idx].all ++;
        if (heavy[heavy_idx].key == key) {
            // only update the heavy part
            heavy[heavy_idx].positive ++;
        }
        else {
            if (heavy[heavy_idx].positive < heavy[heavy_idx].all * m_lambda) {
                // evict old key to the light part
                light.update(heavy[heavy_idx].key, heavy[heavy_idx].positive);

                // update the heavy bucket
                heavy[heavy_idx].positive = heavy[heavy_idx].positive + 1;
                heavy[heavy_idx].key = key;
                heavy[heavy_idx].flag = false;
            }
            else {
                // update the light part
                light.update(key);
            }
        }
    }

    uint32_t getMaxNum() {
        uint32_t m_max_num = 0;
        for (uint32_t i = 0; i < m_heavy_size; i ++) {
            m_max_num = std::max(m_max_num, heavy[i].positive + (heavy[i].flag ? 0 : light.query(heavy[i].key)));
        }
        return light.getMaxNum();
    }
};
