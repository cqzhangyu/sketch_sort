/**
 * @file SFMT.hpp
 *
 * @author Michael Axtmann <michael.axtmann@gmail.com>
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 */

#include <memory>

#pragma once

struct SFMT_T;
typedef struct SFMT_T sfmt_t;

class Sfmt {
public:
  Sfmt();
  ~Sfmt();

  Sfmt(const Sfmt&) = delete;
  Sfmt& operator = (const Sfmt&) = delete;

  static constexpr size_t min64bitBlockSize() {
    return 512; // ((19937 / 128 + 1) * 2);
  }

  static constexpr size_t min32bitBlockSize() {
    return 1024; // ((19937 / 128 + 1) * 4);
  }

  /* @brief Initializes the random generator
   * 
   * The random generator must be initialized before random values are
   * generated.
   */
  void init_gen_rand(uint32_t seed);

  /* @brief Fills an array of uint32_t elements.
   * 
   * @param size The number of elements be generated. size must be a
   * multiple of 4, and greater than or equal to min32bitBlockSize()
   */
  void fill_array32(uint32_t * array, int size);

  /* @brief Fills an array of uint64_t elements.
   * 
   * @param size The number of elements be generated. size must be a
   * multiple of 4, and greater than or equal to min64bitBlockSize()
   */
  void fill_array64(uint64_t * array, int size);

  sfmt_t* get();

protected:
  sfmt_t* sfmt;
};
