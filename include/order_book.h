#pragma once
// Order book for ONE instrument with price-time priority matching (limit orders only).
#include <algorithm>
#include <cstdio>
#include <deque>
#include <functional>
#include <map>
#include <vector>
#include "mem_pool.h"
#include "types.h"

class OrderBook {
    // Each price level is a FIFO queue: front = oldest = highest time priority.
    std::map<Price, std::deque<Order*>, std::greater<Price>> bids_;  // begin() = highest bid
    std::map<Price, std::deque<Order*>>                      asks_;  // begin() = lowest ask
    MemPool<Order> pool_{1024};

    // Match `o` against opposite side `opp`. crosses(levelPrice, limit) = is the resting price acceptable?
    template <typename Book, typename Crosses>
    void match(Book& opp, Order* o, Crosses crosses, std::vector<Trade>& out) {
        while (o->qty > 0 && !opp.empty()) {
            auto lvl = opp.begin();                       // best price first (PRICE priority)
            if (!crosses(lvl->first, o->price)) break;
            auto& q = lvl->second;
            while (o->qty > 0 && !q.empty()) {
                Order* rest = q.front();                  // oldest at this price (TIME priority)
                const Qty fill = std::min(o->qty, rest->qty);
                out.push_back({o->id, rest->id, lvl->first, fill});  // trade at RESTING order's price
                o->qty    -= fill;
                rest->qty -= fill;
                if (rest->qty == 0) { q.pop_front(); pool_.release(rest); }
            }
            if (q.empty()) opp.erase(lvl);                // drop empty price level
        }
    }
public:
    void submit(const Order& in, std::vector<Trade>& out) {
        Order* o = pool_.alloc();
        if (!o) { std::puts("pool exhausted, order rejected"); return; }
        *o = in;
        if (o->side == Side::Buy)
            match(asks_, o, [](Price lvl, Price lim) { return lvl <= lim; }, out);
        else
            match(bids_, o, [](Price lvl, Price lim) { return lvl >= lim; }, out);
        if (o->qty == 0) { pool_.release(o); return; }    // fully filled: nothing rests
        auto& side = (o->side == Side::Buy) ? bids_[o->price] : asks_[o->price];
        side.push_back(o);                                // leftover rests at BACK of its price queue
    }
};
