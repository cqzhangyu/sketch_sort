/**
 * @file dSFMT.cpp
 *
 * @author Michael Axtmann <michael.axtmann@gmail.com>
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 */

#include <dSFMT.hpp>

#include <dSFMT.h>
#include <dSFMT.c>

Dsfmt::Dsfmt() : dsfmt(new dsfmt_t) {}
Dsfmt::~Dsfmt() {delete dsfmt;}

// dsfmt_t* Dsfmt::get() {
//   return dsfmt;
// }

void Dsfmt::init_gen_rand(uint32_t seed) {
  dsfmt_init_gen_rand(dsfmt, seed);
}
void Dsfmt::fill_array_close_open(double array[], int size) {
  dsfmt_fill_array_close_open(dsfmt, array, size);
}
void Dsfmt::fill_array_close1_open2(double array[], int size) {
  dsfmt_fill_array_close1_open2(dsfmt, array, size);
}
