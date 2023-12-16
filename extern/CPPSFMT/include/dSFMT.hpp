/**
 * @file dSFMT.hpp
 *
 * @author Michael Axtmann <michael.axtmann@gmail.com>
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 */

#pragma once

#include <memory>

struct DSFMT_T;
typedef struct DSFMT_T dsfmt_t;

class Dsfmt {
public:
  Dsfmt();
  ~Dsfmt();
  
  Dsfmt(const Dsfmt&) = delete;
  Dsfmt& operator = (const Dsfmt&) = delete;

  static constexpr size_t min64bitBlockSize() {
    return 512; // (((19937 - 128) / 104 + 1) * 2);
  }

  // static constexpr size_t min32bitBlockSize() {
  //   return 1024; // (((19937 - 128) / 104 + 1) * 4);
  // }

  /* @brief Initializes the random generator
   * 
   * The random generator must be initialized before random values are
   * generated.
   */
  void init_gen_rand(uint32_t seed);
  
  /* @brief Fills an array of double elements.
   * 
   * Generate double pseudorandom numbers distributed in the range 
   * [0, 1).
   *
   * @param size The number of elements be generated. size must be a
   * a power of two, and greater than or equal to min64bitBlockSize().
   */
  void fill_array_close_open(double array[], int size);
  
  /* @brief Fills an array of double elements.
   * 
   * Generate double pseudorandom numbers distributed in the range 
   * [1, 2).
   * 
   * @param size The number of elements be generated. size must be a
   * a power of two, and greater than or equal to min64bitBlockSize().
   */
  void fill_array_close1_open2(double array[], int size);

protected:
  dsfmt_t* dsfmt;
};
