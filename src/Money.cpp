#include "bank/Money.h"
#include "bank/Exceptions.h"
#include <cctype>
#include <cstdlib>

namespace bank {

std::string formatMoney(Cents cents) {
    const bool negative = cents < 0;
    unsigned long long abs = negative ? 0ULL - static_cast<unsigned long long>(cents)
                                      : static_cast<unsigned long long>(cents);
    std::string whole = std::to_string(abs / 100);
    for (int i = static_cast<int>(whole.size()) - 3; i > 0; i -= 3) whole.insert(i, ",");
    const int frac = static_cast<int>(abs % 100);
    return std::string(negative ? "-$" : "$") + whole + "." + (frac < 10 ? "0" : "") + std::to_string(frac);
}

Cents parseMoney(const std::string& text) {
    if (text.empty()) throw InvalidInputException("Please enter an amount.");

    Cents whole = 0, frac = 0;
    std::size_t i = 0;
    bool sawDigit = false;
    for (; i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])); ++i) {
        whole = whole * 10 + (text[i] - '0');
        sawDigit = true;
        if (whole > 1'000'000'000'000LL) throw InvalidInputException("Amount is too large.");
    }
    if (i < text.size() && text[i] == '.') {
        ++i;
        int digits = 0;
        for (; i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])); ++i, ++digits) {
            if (digits >= 2) throw InvalidInputException("Use at most two decimal places.");
            frac = frac * 10 + (text[i] - '0');
            sawDigit = true;
        }
        if (digits == 1) frac *= 10;
    }
    if (!sawDigit || i != text.size()) throw InvalidInputException("'" + text + "' is not a valid amount.");
    return whole * 100 + frac;
}

} // namespace bank
