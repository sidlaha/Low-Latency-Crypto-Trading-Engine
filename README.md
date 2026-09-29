# Mini Matching Engine (C++20)

A small, readable teaching implementation of the core ideas behind a low-latency exchange matching engine:

- **Lock-free SPSC queue** (`include/lf_queue.h`): fixed-size ring buffer using `std::atomic` with acquire/release ordering. One producer thread, one consumer thread.
- **Pre-allocated object pool** (`include/mem_pool.h`): all `Order` objects are created up front, so the matching path does not call `new`/`malloc` for orders.
- **Price-time priority order book** (`include/order_book.h`): one instrument, integer tick prices, limit orders only. Best price trades first; at equal prices the oldest order trades first. Trades execute at the resting order's price.
- **Demo** (`src/main.cpp`): a producer thread pushes sample orders through the queue; the main thread matches them and prints trades.

## Build and run

```bash
# Direct
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -pthread -Iinclude src/main.cpp -o mini_engine
./mini_engine

# Or with CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/mini_engine
```

## Expected output

```
order 1 -> 0 trade(s)
order 2 -> 0 trade(s)
order 3 -> 0 trade(s)
order 4 -> 0 trade(s)
order 5 -> 3 trade(s)
   TRADE taker=5 maker=1 price=101 qty=10
   TRADE taker=5 maker=3 price=101 qty=5
   TRADE taker=5 maker=2 price=102 qty=3
order 6 -> 1 trade(s)
   TRADE taker=6 maker=4 price=100 qty=8
```

Order 5 shows both rules: it takes the better price (101) before 102, and at 101 it fills order 1 before order 3 because order 1 arrived first.

## Design notes

- Prices are integers (ticks), never floating point, so comparisons are exact.
- The queue's head and tail counters each sit on their own 64-byte cache line (`alignas(64)`) to avoid false sharing between the two threads.
- The pool is single-threaded by design: only the matching thread touches it.
- No `-ffast-math`: it is unsafe for price and P&L arithmetic and gains nothing for integer matching.

## Limitations (honest scope)

- Not production-grade. There are no unit tests, no cancel/modify, no order types beyond limit, and no input validation.
- The book uses `std::map` and `std::deque` for readability. `std::deque` can allocate, so the whole path is **not** allocation-free.
- No networking, persistence, risk checks, or market data publishing.
- **No benchmarks are provided.** Any latency figure would need a proper measurement (defined start/end points, warm-up, pinned cores, percentile histogram) before being quoted.

## Layout

```
include/types.h        Price, Qty, Order, Trade
include/lf_queue.h     SpscQueue<T, N>
include/mem_pool.h     MemPool<T>
include/order_book.h   OrderBook
src/main.cpp           demo
CMakeLists.txt
```
