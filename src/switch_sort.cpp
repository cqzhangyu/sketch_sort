#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>

#include "common/radix_sort.hpp"
#include "common/utils.hpp"
#include "generator/generator.hpp"
#include "switch/switch.hpp"

class Trie {
   private:
    class Node {
       public:
        // for leaf nodes
        int begin;
        int end;
        uint64_t cm_size;

        uint64_t child_idx;
        uint8_t shift;

        Node() : begin(0), end(0), cm_size(0), child_idx(0), shift(0) {}

        void print() {
            if (end - begin > cm_size) {
                printf("!!!!!!!!!");

                printf(
                    "begin: %d end: %d cm_size: %lu shift: %d child_idx: %ld\n",
                    begin, end, cm_size, shift, child_idx);
            }
        }
    };

    uint32_t m_threshold;
    std::vector<Trie::Node> m_nodes;
    std::vector<size_t> m_leaves;
    uint64_t m_max_idx;

    void build_trie(CmSketch* cm, uint64_t prefix, uint8_t shift,
                    uint64_t idx) {
        uint32_t cm_size = cm->query(prefix);
        // printf("prefix: %016lx cm_size: %lu shift: %d idx: %lu\n", prefix,
        // cm_size, shift, idx);
        if (cm_size < m_threshold) {
            m_leaves.push_back(idx);
            m_nodes[idx].cm_size = cm_size;
            m_nodes[idx].shift = shift;
            // m_nodes[idx].child_idx = 0;
            return;
        }

        // expand the subtree
        m_nodes[idx].child_idx = m_nodes.size();
        m_nodes.resize(m_nodes.size() + (1 << Switch::RADIX_BIT), Node());

        uint8_t new_shift = shift - Switch::RADIX_BIT;
        if (new_shift == 0) return;

        for (uint64_t i = 0; i < (1 << Switch::RADIX_BIT); ++i) {
            build_trie(cm, prefix | (i << new_shift), new_shift,
                       m_nodes[idx].child_idx + i);
        }
    }

   public:
    Trie(CmSketch* cm, uint64_t threshold)
        : m_threshold(threshold), m_max_idx(0) {
        m_nodes.resize((1 << Switch::RADIX_BIT), Node());
        for (uint64_t i = 0; i < (1 << Switch::RADIX_BIT); ++i) {
            build_trie(cm, (i << Switch::INITIAL_SHIFT), Switch::INITIAL_SHIFT,
                       i);
        }

        for (size_t i = 1; i < m_leaves.size(); ++i) {
            m_nodes[m_leaves[i]].begin = m_nodes[m_leaves[i - 1]].begin +
                                         m_nodes[m_leaves[i - 1]].cm_size;
            m_nodes[m_leaves[i]].end = m_nodes[m_leaves[i]].begin;
        }

        m_max_idx =
            m_nodes[m_leaves.back()].begin + m_nodes[m_leaves.back()].cm_size;

        printf("max_idx: %lu\n", m_max_idx);
    }

    void sort(uint64_t* arr, size_t n) {
        std::unique_ptr<uint64_t[]> buffer(new uint64_t[m_max_idx]);

        // move the number into the correct leaf
        for (int i = 0; i < n; ++i) {
            int x = 0;
            for (uint8_t shift = Switch::INITIAL_SHIFT; shift > 0;
                 shift -= Switch::RADIX_BIT) {
                const uint8_t radix = (uint8_t)((arr[i] >> shift) & 0xff);
                if (!m_nodes[x + radix].child_idx) {
                    buffer[m_nodes[x + radix].end++] = arr[i];
                    // printf(
                    //     "arr %d: %016lx into leaf %d: begin %d end %d cm_size "
                    //     "%lu\n",
                    //     i, arr[i], x + radix, m_nodes[x + radix].begin,
                    //     m_nodes[x + radix].end, m_nodes[x + radix].cm_size);
                    break;
                } else {
                    x = m_nodes[x + radix].child_idx;
                }
            }
        }

        // for (size_t i = 0; i < m_leaves.size(); ++i) {
        //     m_nodes[m_leaves[i]].print();
        // }

        printf("nodes size: %lu, leaves size: %lu\n", m_nodes.size(),
               m_leaves.size());

        // sort each leaf
        int arr_off = 0;
        for (int i = 0; i < m_leaves.size(); ++i) {
            int idx = m_leaves[i];
            int off = m_nodes[idx].begin;
            int size = m_nodes[idx].end - m_nodes[idx].begin;
            memcpy(arr + arr_off, buffer.get() + off, size * sizeof(uint64_t));
            // printf("=====cm_size %ld, shift %d====\n", m_nodes[idx].cm_size,
            //        m_nodes[idx].shift);
            // for (int i = 0; i < size; ++i) {
            //     printf("%016lx ", arr[arr_off + i]);
            // }
            // printf("\n");
            // q_sort(arr + arr_off, size);
            radix_sort(arr + arr_off, buffer.get() + off, size,
                       m_nodes[idx].shift - Switch::RADIX_BIT);
            arr_off += size;
        }
        printf("arr_off: %d\n", arr_off);
    }

    std::vector<Trie::Node>& getNodes() { return m_nodes; }

    std::vector<size_t>& getLeaves() { return m_leaves; }
};

int main(int argc, char** argv) {
    // srand(time(NULL));
    int n;
    uint32_t hash_num, width;
    uint64_t threshold;

    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <n> <hash_num> <width> <threshold>" << std::endl;
        exit(1);
    } else {
        n = atoi(argv[1]);
        hash_num = atoi(argv[2]);
        width = atoi(argv[3]);
        threshold = atoi(argv[4]);
    }

    std::unique_ptr<uint64_t[]> arr(new uint64_t[n]);

    GenExponential gen;
    gen(arr.get(), arr.get() + n);
    // arr[0] = 0x9f1f621ab050007e;
    // arr[1] = 0xf93d415f8e9cd8fd;
    // arr[2] = 0x28aff38934c7da27;
    // arr[3] = 0x612202468b6bdb6f;
    // arr[4] = 0x2b5a8ab41ae0b579;

    // simulating in-network sketch computation
    std::unique_ptr<Switch> sw(new Switch(hash_num, width));

    sw->run(arr.get(), n);

    // simulating on-host sorting using sketch results
    std::unique_ptr<Trie> trie(new Trie(sw->getSketch(), threshold));

    // for (uint64_t i = 0; i < n; ++i) {
    //     for (uint8_t shift = Switch::INITIAL_SHIFT; shift > 0;
    //          shift -= Switch::RADIX_BIT) {
    //         uint64_t key = (arr[i] >> shift) << shift;
    //         printf("query %16lx, shift %d, result %ld\n", key, shift,
    //         sw->getSketch()->query(key));
    //     }
    // }

    auto begin_time = std::chrono::high_resolution_clock::now();
    trie->sort(arr.get(), n);
    // std::unique_ptr<uint64_t[]> buffer(new uint64_t[n]);
    // radix_sort(arr.get(), buffer.get(), n, Switch::INITIAL_SHIFT);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = end_time - begin_time;

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration)
                     .count()
              << "ms" << std::endl;

    std::cout << "Trie size: " << trie->getNodes().size()
              << " Leaves: " << trie->getLeaves().size() << std::endl;

    for (int i = 1; i < n; i++) {
        if (arr[i - 1] > arr[i]) {
            printf("Wrong! arr[%d] = %016lx > arr[%d] = %016lx!\n", i - 1,
                   arr[i - 1], i, arr[i]);
            for (int j = i > 10 ? i - 10 : 0; j < std::min(i + 10, n); j++) {
                printf("arr[%d] = %016lx\n", j, arr[j]);
            }
            break;
        }
    }

    return 0;
}