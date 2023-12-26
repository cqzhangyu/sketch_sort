#include <memory>
#include <vector>

#include "ElasticSketch.hpp"
#include "TowerSketch.hpp"
#include "CmSketch.hpp"

class Switch {
   private:
    std::unique_ptr<Sketch> m_sketch;

   public:
    constexpr static uint8_t RADIX_BIT = 8;
    constexpr static uint64_t RADIX_MASK = ((uint64_t)1 << RADIX_BIT) - 1;
    constexpr static uint64_t INITIAL_SHIFT = sizeof(uint64_t) * 8 - RADIX_BIT;
    constexpr static uint8_t LAST_SHIFT = RADIX_BIT * 2;

    Switch(uint32_t hashNum, uint32_t width, std::string sketch,
           uint32_t heavyDepth = 0, uint32_t heavyWidth = 0,
           double lambda = 0) {
        if (sketch == "cm") {
            m_sketch = std::unique_ptr<CmSketch>(new CmSketch(hashNum, width));
        } else if (sketch == "tower") {
            m_sketch =
                std::unique_ptr<TowerSketch>(new TowerSketch(hashNum, width));
        } else if (sketch == "elastic") {
            m_sketch = std::unique_ptr<ElasticSketch>(new ElasticSketch(
                hashNum, width, heavyDepth, heavyWidth, lambda));
        } else {
            throw std::runtime_error("unknown sketch");
        }
    }

    void run(uint64_t* arr, int n) {
        for (int i = 0; i < n; ++i) {
            uint32_t min_value = std::numeric_limits<uint32_t>::max();
            for (uint8_t shift = INITIAL_SHIFT; shift >= LAST_SHIFT;
                 shift -= RADIX_BIT) {
                uint64_t key = ((arr[i] >> shift) << shift) | shift;
                // printf("key: %016lx shift: %d\n", key, shift);
                uint32_t cur_value = m_sketch->query(key);
                if (cur_value < min_value) {
                    m_sketch->update(key);
                    min_value = cur_value + 1;
                }
                

                // printf("key: %016lx shift: %d\n", key, shift);

                // optimization! conservative update!
                // the sketch values in the lower layers should not be greater
                // than the sketch values in its higher layers!
            }
        }
    }

    Sketch* getSketch() { return m_sketch.get(); }
};