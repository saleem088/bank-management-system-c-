#pragma once
#include "bank/Money.h"
#include <ctime>
#include <string>

namespace bank {

enum class TransactionType { Deposit, Withdrawal, TransferIn, TransferOut, Interest, Fee };

std::string toString(TransactionType type);
TransactionType transactionTypeFromString(const std::string& text);

struct Transaction {
    TransactionType type;
    Cents amount;        // always positive; the type says which direction
    Cents balanceAfter;
    std::time_t timestamp;
    std::string note;
};

} // namespace bank
