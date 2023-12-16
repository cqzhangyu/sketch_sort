// src/pbbs_generators/utils
//
// Generator classes of this benchmark.
//
// This code contains highly modified code snippets from the Problem
// Based Benchmark Suite - http://www.cs.cmu.edu/~pbbs/benchmarks.html
//
// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2010 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Part of Project Ips4o Benchmark Suite - https://github.com/MichaelAxtmann/ips4o-benchmark-suite
//
// Copyright (c) 2020 Michael Axtmann <michael.axtmann@gmail.com>

#pragma once

#include <algorithm>
#include <climits>
#include <random>
#include <string>

#include <tlx/math/integer_log2.hpp>
#include <SimdMt.hpp>

#include "simple_alias.hpp"
#include "zipf_distribution.hpp"
#include "parallel_for.hpp"
#include "utils.h"

template <class T>
constexpr bool isPrimitiveDatatypeConstructable() {
    return std::is_same_v<T, double>
            || std::is_same_v<T, uint32_t>
            || std::is_same_v<T, uint64_t>;
}

class GenSorted {
 public:
    GenSorted() {}

    static std::string name() { return "sorted"; }


    template <class T>
    void operator()(T* begin, T* end) {
        std_parallel_for(end - begin,
                         [begin](size_t begin_idx, size_t end_idx, size_t /*thread_id*/) {
                             for (size_t i = begin_idx; i != end_idx; ++i) {
                                 begin[i] = static_cast<T>(i);
                             }
                         });
    }
};

class GenReverse {
 public:
    GenReverse() {}

    static std::string name() { return "reverse"; }

    template <class T>
    void operator()(T* begin, T* end) {
        const size_t size = end - begin;

        std_parallel_for(end - begin, [begin, size](size_t begin_idx, size_t end_idx,
                                                    size_t /*thread_id*/) {
            for (size_t i = begin_idx; i != end_idx; ++i) {
                begin[i] = static_cast<T>(size - i);
            }
        });
    }
};

class GenRootdupls {
 public:
    GenRootdupls() {}

    static std::string name() { return "rootdupls"; }


    template <class T>
    void operator()(T* begin, T* end) {
        const size_t size = end - begin;

        const size_t froot = std::sqrt(size);
        uint64_t val = 0;

        std_parallel_for(end - begin, [begin, froot](size_t begin_idx, size_t end_idx,
                                                     size_t /*thread_id*/) {
            uint64_t val = begin_idx % froot;
            for (size_t i = begin_idx; i != end_idx; ++i) {
                if (val == froot) val = 0;
                begin[i] = static_cast<T>(val);
                ++val;
            }
        });
    }
};

class GenZero {
 public:
    GenZero() {}

    static std::string name() { return "zero"; }


    template <class T>
    void operator()(T* begin, T* end) {
        std_parallel_for(end - begin,
                         [begin](size_t begin_idx, size_t end_idx, size_t /*thread_id*/) {
                             for (size_t i = begin_idx; i != end_idx; ++i) {
                                 begin[i] = T{0};
                             }
                         });
    }
};

class GenRandom {
 public:
    GenRandom() {}

    static std::string name() { return "random"; }


    template <class T>
    void operator()(T* begin, T* end) {
        static_assert(std::is_same_v<T, double>
                      || std::is_same_v<T, uint32_t>
                      || std::is_same_v<T, uint64_t>);

        std::random_device rd;
        const uint32_t seed = rd();
        if constexpr (std::is_same_v<T, double>) {
            std_parallel_for(end - begin, [begin, seed](size_t begin_idx, size_t end_idx,
                                                        size_t thread_id) {
                SimdMtGenerator<double>::fill(seed + thread_id, begin + begin_idx,
                                              begin + end_idx);
            });
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            std_parallel_for(end - begin, [begin, seed](size_t begin_idx, size_t end_idx,
                                                        size_t thread_id) {
                SimdMtGenerator<uint32_t>::fill(seed + thread_id, begin + begin_idx,
                                                begin + end_idx);
            });
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            std_parallel_for(end - begin, [begin, seed](size_t begin_idx, size_t end_idx,
                                                        size_t thread_id) {
                SimdMtGenerator<uint64_t>::fill(seed + thread_id, begin + begin_idx,
                                                begin + end_idx);
            });
        } else {
            assert(false);
        }
    }
};

class GenZipf {
 public:
    GenZipf() {}

    static std::string name() { return "zipf"; }

    template <class T>
    void operator()(T* begin, T* end) {
        static_assert(std::is_same_v<T, uint32_t>
                      || std::is_same_v<T, uint64_t>
                      || std::is_same_v<T, double>);

        const size_t N = 1000000;
        const double s = 0.75;
        static auto vdistr = thrill::common::ZipfDistribution::make_vec(N, s);
        static wrs::simple_alias<uint32_t> alias(vdistr.begin(), vdistr.end());

        auto make_zipf = [](double d) { return alias(d); };

        if constexpr (std::is_same_v<T, uint32_t>
                      || std::is_same_v<T, uint64_t>
                      || std::is_same_v<T, double>) {
            std::random_device rd;
            const uint32_t seed = rd();
            std_parallel_for(end - begin, [begin, seed, &make_zipf](size_t begin_idx,
                                                                    size_t end_idx,
                                                                    size_t thread_id) {
                const auto b = begin + begin_idx;
                const auto e = begin + end_idx;
                SimdMtGenerator<double>::fill(seed + thread_id, b, e, make_zipf);
            });
        }
    }
};

template <class T>
inline T my_rand(uint64_t x);

template <>
uint64_t my_rand<uint64_t>(uint64_t x) {
    return hash64(x);
}

template <>
double my_rand<double>(uint64_t x) {
    return ((double)hash64(x)) / ((double)std::numeric_limits<uint64_t>::max());
}

template <>
uint32_t my_rand<uint32_t>(uint64_t x) {
    return hash32(x);
}

class GenExponential {
 public:
    GenExponential() {}

    static std::string name() { return "exponential"; }

    template <class T>
    void operator()(T* begin, T* end) {
        using KeyType = T;

        const size_t size = end - begin;
        const size_t lg = std::min<size_t>(
                tlx::integer_log2_ceil(size) + 1,
                tlx::integer_log2_ceil(std::numeric_limits<KeyType>::max()));

        std::random_device rd;
        const uint32_t seed = rd();
        std_parallel_for(size, [begin, lg, seed](size_t begin_idx, size_t end_idx,
                                                 size_t thread_id) {
            SimdMtGeneratorUint64 gen(seed + thread_id);
            for (size_t i = begin_idx; i != end_idx; ++i) {
                const size_t range = (size_t{1} << (gen() % lg));
                begin[i] = my_rand<KeyType>(range + (gen() % range));
            }
        });
    }

    void operator()(double* begin, double* end) {
        using KeyType = double;

        const size_t size = end - begin;
        const size_t lg = tlx::integer_log2_ceil(size) + 1;

        std::random_device rd;
        const uint32_t seed = rd();
        std_parallel_for(size, [begin, lg, seed](size_t begin_idx, size_t end_idx,
                                                 size_t thread_id) {
            SimdMtGeneratorUint64 gen(seed + thread_id);
            for (size_t i = begin_idx; i != end_idx; ++i) {
                size_t range = (size_t{1} << (gen() % lg));
                begin[i] = my_rand<KeyType>(range + (gen() % range));
            }
        });
    }
};

class GenAlmostsorted {
 public:
    GenAlmostsorted() {}

    static std::string name() { return "almostsorted"; }


    template <class T>
    void operator()(T* begin, T* end) {
        const size_t size = end - begin;

        std_parallel_for(end - begin,
                         [begin](size_t begin_idx, size_t end_idx, size_t /*thread_id*/) {
                             for (size_t i = begin_idx; i != end_idx; ++i) {
                                 begin[i] = static_cast<T>(i);
                             }
                         });
        // for (size_t i = 0; i != size; ++i) {
        //   begin[i] = T{i};
        // }

        size_t swaps = static_cast<size_t>(std::sqrt(static_cast<double>(size)));
        std::random_device rd;
        const uint32_t seed = rd();
        static SimdMtGeneratorUint64 gen(seed);

        for (size_t i = 0; i < swaps; i++) {
            std::swap(begin[gen() % size], begin[gen() % size]);
        }
    }
};

class GenTwicedupes {
 public:
    GenTwicedupes() {}

    static std::string name() { return "twicedupes"; }

    template <class T>
    void operator()(T* begin, T* end) {
        const size_t size = end - begin;

        uint64_t prev_pow_2 = 1;
        while (2 * prev_pow_2 <= size) { prev_pow_2 *= 2; }

        // Limit maximum size of values for 32-bit keys.
        if constexpr (std::is_same_v<T, uint32_t>) {
            uint32_t max = std::numeric_limits<T>::max();
            prev_pow_2 = std::min<uint64_t>(prev_pow_2, max);
        }

        const size_t offset_zw = prev_pow_2 / 2;

        std_parallel_for(end - begin, [offset_zw, prev_pow_2, begin](
                                              size_t begin_idx, size_t end_idx,
                                              size_t /*thread_id*/) {
            for (size_t i = begin_idx; i != end_idx; ++i) {
                begin[i] = static_cast<T>((offset_zw + i * i) % prev_pow_2);
            }
        });
    }
};

class GenEightdupes {
 public:
    GenEightdupes() {}

    static std::string name() { return "eightdupes"; }

    template <class T>
    void operator()(T* begin, T* end) {
        const size_t size = end - begin;

        uint64_t prev_pow_2 = 1;
        while (2 * prev_pow_2 <= size) { prev_pow_2 *= 2; }

        // Limit maximum size of values for 32-bit keys.
        if constexpr (std::is_same_v<T, uint32_t>) {
            uint32_t max = std::numeric_limits<T>::max();
            prev_pow_2 = std::min<uint64_t>(prev_pow_2, max);
        }

        const size_t offset_zw = prev_pow_2 / 2;

        std_parallel_for(end - begin, [offset_zw, prev_pow_2, begin](
                                              size_t begin_idx, size_t end_idx,
                                              size_t /*thread_id*/) {
            for (size_t i = begin_idx; i != end_idx; ++i) {
                uint64_t temp = (i * i) % prev_pow_2;
                temp = (temp * temp) % prev_pow_2;
                begin[i] = static_cast<T>((offset_zw + temp * temp) % prev_pow_2);
            }
        });
    }
};
