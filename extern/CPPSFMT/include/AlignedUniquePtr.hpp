/**
 * @file AlignedUniquePtr.hpp
 *
 * @author Michael Axtmann <michael.axtmann@gmail.com>
 *
 * The new BSD License is applied to this software, see LICENSE.txt
 */

#pragma once

#include <string>
#include <cassert>
#include <memory>

#pragma once

template<class T>
class AlignedUniquePtrDeleter {
public:
  AlignedUniquePtrDeleter() {
  }
  void operator () (T* p) const {
    free(p);
  };
protected:
};

template<class T>
class AlignedUniquePtr : public std::unique_ptr<T[], AlignedUniquePtrDeleter<T>> {
public:
  AlignedUniquePtr() {}
  
  AlignedUniquePtr(size_t size, size_t alignment)
    : std::unique_ptr<T[], AlignedUniquePtrDeleter<T>>(nullptr, AlignedUniquePtrDeleter<T>{}) {
    
    assert(alignment % sizeof(void*) == 0);
    
    T* ptr = nullptr;
    const auto res = posix_memalign((void **)&ptr, alignment, sizeof(T) * size);
    assert(res == 0);
    
    this->reset(ptr);
  }

  static std::string name() {
    return "vector";
  }

};
