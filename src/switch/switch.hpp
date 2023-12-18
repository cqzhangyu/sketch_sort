#include <memory>
#include <vector>

#include "sketch.hpp"

class Switch {
   private:
    std::unique_ptr<CmSketch> m_sketch;

   public:
    constexpr static uint8_t RADIX_BIT = 8;
    constexpr static uint64_t RADIX_MASK = ((uint64_t)1 << RADIX_BIT) - 1;
    constexpr static uint64_t INITIAL_SHIFT = sizeof(uint64_t) * 8 - RADIX_BIT;

    Switch(uint32_t hashNum, uint32_t width) {
        m_sketch = std::unique_ptr<CmSketch>(new CmSketch(hashNum, width));
    }

    void run(uint64_t* arr, int n) {
        for (int i = 0; i < n; ++i) {
            for (uint8_t shift = INITIAL_SHIFT; shift > 0; shift -= RADIX_BIT) {
                uint64_t key = (arr[i] >> shift) << shift;
                m_sketch->update(key);
                // printf("key: %016lx shift: %d\n", key, shift);
            }
        }

        std::cout << "max_num: " << m_sketch->getMaxNum() << std::endl;
    }

    CmSketch* getSketch() { return m_sketch.get(); }
};