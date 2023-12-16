/**
 * @file SimdMt.hpp
 *
 * @author Michael Axtmann <michael.axtmann@gmail.com>
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 */

#pragma once

#include <cassert>
#include <random>
#include <limits>

#include "dSFMT.hpp"
#include "SFMT.hpp"

#include "AlignedUniquePtr.hpp"

/* Fast SIMD-oriented Fast Mersenne Twister
 *
 * The fill functions use the Fast Mersenne Twister dSFMT and SFMT to
 * generate blocks of aligned values. The input to the fill function
 * must not be aligned and the number of input elements is not
 * constrained.
 *
 * The generator supports double, uint64_t, and uint32_t datatypes.
 * For doubles, values in the range of [0, 1) are generated.
 */
template<class KeyType>
class SimdMtGenerator {
public:

  /* @brief Generate random values.
   */
  template<class Iterator>
  static void fill(uint32_t seed, Iterator begin, Iterator end);

  /* @brief Generate random values.
   *
   * This function generates random values of type KeyType. Before the
   * value is assigned to the value of the iterator, it is passed to
   * the function 'unary'.
   */
  template<class Iterator, class UnaryOperator>
  static void fill(uint32_t seed, Iterator begin, Iterator end, UnaryOperator& unary);
};

/* Fast SIMD-oriented Fast Mersenne Twister
 *
 * Fast Mersenne Twister. The operator '()' returns random uint64_t
 * values. For a fast generation of these values, this class generates
 * blocks of values using the Fast Mersenne Twister SFMT.
 */
class SimdMtGeneratorUint64 {
public:

  using result_type = uint64_t;

  static result_type max() {
    return std::numeric_limits<result_type>::max();
  }

  static result_type min() {
    return std::numeric_limits<result_type>::min();
  }
  
  SimdMtGeneratorUint64(uint32_t seed);

  SimdMtGeneratorUint64(const SimdMtGeneratorUint64&) = delete;
  SimdMtGeneratorUint64& operator = (const SimdMtGeneratorUint64&) = delete;

  inline uint64_t operator()();

private:
  Sfmt sfmt;
  AlignedUniquePtr<uint64_t> arr;
  size_t size;
  size_t off;
  
};

SimdMtGeneratorUint64::SimdMtGeneratorUint64(uint32_t seed) {
  sfmt.init_gen_rand(seed);
  size_t min_size = Sfmt::min64bitBlockSize(); 
  size = std::max<size_t>(min_size, 1ul << 15);

  arr = AlignedUniquePtr<uint64_t>(size, 16);

  sfmt.fill_array64(arr.get(), size);
  off = 0;
}

inline uint64_t SimdMtGeneratorUint64::operator()() {
  if (off == size) {
    sfmt.fill_array64(arr.get(), size);
    off = 0;
  }
  const auto val = arr[off];
  ++off;
  return val;
}

template<>
template<class Iterator>
void SimdMtGenerator<uint32_t>::fill(uint32_t seed, Iterator begin, Iterator end) {
  Sfmt sfmt;
  sfmt.init_gen_rand(seed);

  size_t min_size = Sfmt::min32bitBlockSize();
  size_t size = std::max<size_t>(min_size, 1ul << 15);

  AlignedUniquePtr<uint32_t> arr(size, 16);

  sfmt.fill_array32(arr.get(), size);

  size_t off = 0;
  for (auto it = begin; it != end; ++it) {
    if (off == size) {
      sfmt.fill_array32(arr.get(), size);
      off = 0;
    }
    *it = arr[off];
    ++off;
  }
}

template<>
template<class Iterator>
void SimdMtGenerator<uint64_t>::fill(uint32_t seed, Iterator begin, Iterator end) {
  Sfmt sfmt;
  sfmt.init_gen_rand(seed);

  size_t min_size = Sfmt::min64bitBlockSize();
  size_t size = std::max<size_t>(min_size, 1ul << 15);

  AlignedUniquePtr<uint64_t> arr(size, 16);

  sfmt.fill_array64(arr.get(), size);

  size_t off = 0;
  for (auto it = begin; it != end; ++it) {
    if (off == size) {
      sfmt.fill_array64(arr.get(), size);
      off = 0;
    }
    *it = arr[off];
    ++off;
  }
  
}


template<>
template<class Iterator>
void SimdMtGenerator<double>::fill(uint32_t seed, Iterator begin, Iterator end) {
  Dsfmt dsfmt;
  dsfmt.init_gen_rand(seed);

  size_t min_size = Dsfmt::min64bitBlockSize();
  size_t size = std::max<size_t>(min_size, 1ul << 15);

  AlignedUniquePtr<double> arr(size, 16);

  dsfmt.fill_array_close_open(arr.get(), size);

  size_t off = 0;
  for (auto it = begin; it != end; ++it) {
    if (off == size) {
      dsfmt.fill_array_close_open(arr.get(), size);
      off = 0;
    }
    *it = arr[off];
    ++off;
  }
  
}

template<>
template<class Iterator, class UnaryOperator>
void SimdMtGenerator<uint32_t>::fill(uint32_t seed, Iterator begin, Iterator end, UnaryOperator& unary) {
  Sfmt sfmt;
  sfmt.init_gen_rand(seed);

  size_t min_size = Sfmt::min32bitBlockSize();
  size_t size = std::max<size_t>(min_size, 1ul << 15);

  AlignedUniquePtr<uint32_t> arr(size, 16);

  sfmt.fill_array32(arr.get(), size);

  size_t off = 0;
  for (auto it = begin; it != end; ++it) {
    if (off == size) {
      sfmt.fill_array32(arr.get(), size);
      off = 0;
    }
    *it = unary(arr[off]);
    ++off;
  }
  
}

template<>
template<class Iterator, class UnaryOperator>
void SimdMtGenerator<uint64_t>::fill(uint32_t seed, Iterator begin, Iterator end, UnaryOperator& unary) {
  Sfmt sfmt;
  sfmt.init_gen_rand(seed);

  size_t min_size = Sfmt::min64bitBlockSize();
  size_t size = std::max<size_t>(min_size, 1ul << 15);

  AlignedUniquePtr<uint64_t> arr(size, 16);

  sfmt.fill_array64(arr.get(), size);

  size_t off = 0;
  for (auto it = begin; it != end; ++it) {
    if (off == size) {
      sfmt.fill_array64(arr.get(), size);
      off = 0;
    }
    *it = unary(arr[off]);
    ++off;
  }
  
}


template<>
template<class Iterator, class UnaryOperator>
void SimdMtGenerator<double>::fill(uint32_t seed, Iterator begin, Iterator end, UnaryOperator& unary) {
  Dsfmt dsfmt;
  dsfmt.init_gen_rand(seed);

  size_t min_size = Dsfmt::min64bitBlockSize();
  size_t size = std::max<size_t>(min_size, 1ul << 15);

  AlignedUniquePtr<double> arr(size, 16);

  dsfmt.fill_array_close_open(arr.get(), size);

  size_t off = 0;
  for (auto it = begin; it != end; ++it) {
    if (off == size) {
      dsfmt.fill_array_close_open(arr.get(), size);
      off = 0;
    }
    *it = unary(arr[off]);
    ++off;
  }
  
}
