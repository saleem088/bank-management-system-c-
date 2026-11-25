#pragma once
#include <string>

namespace bank {

// Money is stored as whole cents in a long long. Never use double for money:
// 0.1 + 0.2 != 0.3 in floating point.
using Cents = long long;

std::string formatMoney(Cents cents);          // 123456 -> "$1,234.56"
Cents parseMoney(const std::string& text);     // "12.5" -> 1250, throws InvalidInputException

} // namespace bank
