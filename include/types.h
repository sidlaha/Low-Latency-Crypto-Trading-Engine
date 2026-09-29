#pragma once
// Core types shared by every component.
#include <cstdint>

using Price   = std::int64_t;   // price in integer ticks (no floating point)
using Qty     = std::uint32_t;
using OrderId = std::uint64_t;
enum class Side : std::uint8_t { Buy, Sell };

struct Order { OrderId id; Side side; Price price; Qty qty; };          // qty = quantity still open
struct Trade { OrderId taker; OrderId maker; Price price; Qty qty; };   // one fill
