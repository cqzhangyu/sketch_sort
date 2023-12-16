/**
 * @file SFMT.cpp
 *
 * @author Michael Axtmann <michael.axtmann@gmail.com>
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 */

#include <memory>
#include <iostream>

#include "SFMT.hpp"

#include <SFMT.c>
#include <SFMT.h>

#if defined(__cplusplus)
extern "C" {
#endif

Sfmt::Sfmt() : sfmt(new sfmt_t) {}
Sfmt::~Sfmt() {
  delete sfmt;
}

sfmt_t* Sfmt::get() {
  return sfmt;
}

  void Sfmt::fill_array32(uint32_t * array, int size) {
    sfmt_fill_array32(sfmt, array, size);
  }
  void Sfmt::fill_array64(uint64_t * array, int size) {
    sfmt_fill_array64(sfmt, array, size);
  }
  void Sfmt::init_gen_rand(uint32_t seed) {
    sfmt_init_gen_rand(sfmt, seed);
  }

#if defined(__cplusplus)
}
#endif

  
