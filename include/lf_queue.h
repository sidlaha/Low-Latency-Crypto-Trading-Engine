#pragma once
// Fixed-size lock-free single-producer / single-consumer (SPSC) ring buffer.
#include <array>
#include <atomic>
#include <cstddef>

// Exactly one thread may call push(), exactly one thread may call pop().
// N must be a power of two so "index % N" becomes a cheap bit-mask.
template <typename T, std::size_t N>
class SpscQueue {
    static_assert(N >= 2 && (N & (N - 1)) == 0, "N must be a power of two");
    // Each counter sits on its own 64-byte cache line to avoid false sharing.
    alignas(64) std::atomic<std::size_t> head_{0};  // next slot to read; only consumer writes
    alignas(64) std::atomic<std::size_t> tail_{0};  // next slot to write; only producer writes
    alignas(64) std::array<T, N> buf_{};            // storage
public:
    bool push(const T& v) {                          // producer thread only
        const auto t = tail_.load(std::memory_order_relaxed);   // own counter: no ordering needed
        // acquire: see consumer's latest head, so the slot is known to be fully read.
        if (t - head_.load(std::memory_order_acquire) == N) return false;  // full (wrap-safe)
        buf_[t & (N - 1)] = v;                        // write data first...
        tail_.store(t + 1, std::memory_order_release); // ...then publish it (release)
        return true;
    }
    bool pop(T& out) {                               // consumer thread only
        const auto h = head_.load(std::memory_order_relaxed);   // own counter
        // acquire: pairs with producer's release, so slot data is visible before we read it.
        if (h == tail_.load(std::memory_order_acquire)) return false;      // empty
        out = buf_[h & (N - 1)];                      // read data...
        head_.store(h + 1, std::memory_order_release); // ...then free the slot
        return true;
    }
};
