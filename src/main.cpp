// Demo: a producer thread sends orders through the SPSC queue; the main thread matches them.
#include <array>
#include <cstdio>
#include <thread>
#include <vector>
#include "lf_queue.h"
#include "order_book.h"

int main() {
    SpscQueue<Order, 64> q;
    const std::array<Order, 6> orders{{
        {1, Side::Sell, 101, 10},   // rests on ask side
        {2, Side::Sell, 102,  5},   // rests, worse price
        {3, Side::Sell, 101,  5},   // rests behind order 1 at same price
        {4, Side::Buy,  100,  8},   // no cross: rests
        {5, Side::Buy,  102, 18},   // sweeps 101 (order 1 then 3), then 102 (order 2, partial)
        {6, Side::Sell, 100, 10},   // hits bid 4 for 8, leftover 2 rests
    }};

    std::thread producer([&] {                            // stands in for the order server
        for (const Order& o : orders)
            while (!q.push(o)) std::this_thread::yield();
    });

    OrderBook book;
    std::vector<Trade> trades;
    trades.reserve(8);
    for (std::size_t got = 0; got < orders.size();) {
        Order o{};
        if (!q.pop(o)) { std::this_thread::yield(); continue; }
        ++got;
        trades.clear();
        book.submit(o, trades);
        std::printf("order %llu -> %zu trade(s)\n", static_cast<unsigned long long>(o.id), trades.size());
        for (const Trade& t : trades)
            std::printf("   TRADE taker=%llu maker=%llu price=%lld qty=%u\n",
                        static_cast<unsigned long long>(t.taker),
                        static_cast<unsigned long long>(t.maker),
                        static_cast<long long>(t.price), t.qty);
    }
    producer.join();
    return 0;
}
