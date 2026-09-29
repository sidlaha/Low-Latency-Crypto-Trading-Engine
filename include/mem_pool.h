#pragma once
// Simple pre-allocated object pool. Single-threaded by design: only the matching thread uses it.
#include <cstddef>
#include <cstdint>
#include <vector>

template <typename T>
class MemPool {
    std::vector<T>             slots_;  // objects created once, up front
    std::vector<std::uint32_t> free_;   // stack of free slot indices
public:
    explicit MemPool(std::size_t n) : slots_(n) {
        free_.reserve(n);                                   // reserved, so later push_back never allocates
        for (std::size_t i = n; i-- > 0;) free_.push_back(static_cast<std::uint32_t>(i));
    }
    T* alloc() {                                            // nullptr when exhausted
        if (free_.empty()) return nullptr;
        const auto i = free_.back(); free_.pop_back();
        return &slots_[i];
    }
    void release(T* p) {                                    // pointer arithmetic recovers the index
        free_.push_back(static_cast<std::uint32_t>(p - slots_.data()));
    }
};
