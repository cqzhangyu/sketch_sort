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
            /* another approach
            this approach may (or not) reduce the impact of massive small buckets on the sketches' accuracy
            parameter BEGIN : this approach starts from the BEGIN-th layer
            parameter SMALL_THRES[] : another threshold to filter very small nodes

            * update phase: 
            update arr[i] in the [0,BEGIN)-th layers' sketches as normal
            for k in [BEGIN, ...)
                update arr[i] in the k-th layer's sketch
                if sketch value does not exceed SMALL_THRES[k]
                    break!
                    // do not add the small values

            * query phase:
            query the [0,BEGIN)-th layers' sketches as normal
            sum_ancestors = 0
            when querying at the k-th layer (k >= BEGIN):
                sketch_val = query the k-th layer's sketch
                val = sketch_val + sum_ancestors
                check if val exceeds threshold (to test if this is a big node)
                ...other logic as normal

                finally,
                sum_ancestors += min(SMALL_THRES[k], sketch_val)
                    // recover the un-added small values
            */
        }

        std::cout << "sketch max_num: " << m_sketch->getMaxNum() << std::endl;
    }

    CmSketch* getSketch() { return m_sketch.get(); }
};