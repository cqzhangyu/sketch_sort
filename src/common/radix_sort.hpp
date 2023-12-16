#pragma once
#include <cstdint>
#include <unistd.h>
#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>

#define SWITCH_TO_INSERTION 16
#define SWITCH_TO_QSORT 256

static inline uint64_t max(uint64_t a, uint64_t b) {
    /* return the max a and b */
    return a > b ? a : b;
}

static inline uint64_t min(uint64_t a, uint64_t b) {
    /* return the min a and b */
    return a < b ? a : b;
}

static void q_sort(uint64_t a[], size_t size) {
    /*
     * sort in place the array a of size size using quicksort while
     * the array size is > SWITCH_TO_INSERTION, otherwise switch
     * to an insertion sort (faster on small arrays due to better
     * caching)
     */

    if(size < SWITCH_TO_INSERTION) {
        /*
         * embedded insertion sort to avoid function call
         * (we may also have used an inline function)
         */

        // use gcc __builtin_expect to optimize pipelining
        if(__builtin_expect(size < 2, 0)) {
            return;
        }

        uint64_t v;
        size_t i, j;

        // ^= faster than = 0
        for(i ^= i; i < size; ++i) {
            v = a[i];

            for(j = i; j && v < a[j - 1]; --j) {
                a[j] = a[j - 1];
            }

            a[j] = v;
        }

        return;
    }

    uint64_t
        mid = a[size >> 1],
            *l = a,
            *r = a + size - 1,
            t,
            /*
             * take the pivot as the middle value of 3 items
             * avoid to have a degenerated qsort (why not
             * take the average?)
             */
            p = *l < mid ?
                (mid < *r ? mid : max(*r, *l))
                : (*l < *r ? *l : max(mid, *r));

    while(l <= r) {
        // while well "ordered"
        while(*l < p) {
            l++;
        }

        // while well "ordered"
        while(*r > p) {
            r--;
        }

        // if we haven't finish the partition, swap elements
        if(l <= r) {
            t = *l;
            *l++ = *r;
            *r-- = t;
        }
    }

    // sort sub-arrays
    q_sort(a, (r - a) + 1);
    q_sort(l, (a - l) + size);
}

static void radix_sort(uint64_t a[], uint64_t *buffer,
        size_t size, uint8_t shift) {
    // switch to a quick sort if array is small or shift == 0
    if(size < SWITCH_TO_QSORT || !shift) {
        q_sort(a, size);
        return;
    }

    size_t i;
    uint8_t radix;

    size_t buckets_size[0x100] = {0}, // size needed in each bucket
           buckets_index[0x100] = {0}, // first address of each bucket
           buckets_cindex[0x101] = {0}; // 0x101 because of next loop
                                        // add all indexes

    for(i = size; i;) {
        radix = (a[--i] >> shift) & 0xff;
        ++buckets_size[radix];
    }

    uint64_t *buckets[0x100];
    uint64_t *ptr = buffer;

    for(i ^= i; i < 0x100; ++i) {
        buckets[i] = ptr;
        ptr += (uint64_t)buckets_size[i];
        buckets_cindex[i + 1] = buckets_cindex[i] + buckets_size[i];
    }

    // add the numbers in the correct buckets
    for(i = size; i--;) {
        radix = (a[i] >> shift) & 0xff;
        buckets[radix][buckets_index[radix]++] = a[i];
    }

    // in the next level of recursion we shift the shift to the right
    const uint8_t new_shift = shift - 010;

    for(i = 0; i < 0x100; ++i) {
        radix_sort(buckets[i], a + buckets_cindex[i], buckets_size[i], new_shift);
    }
    memcpy(a, buffer, size * sizeof (uint64_t));
}
