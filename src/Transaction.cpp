#include "bank/Transaction.h"
#include "bank/Exceptions.h"

namespace bank {

std::string toString(TransactionType type) {
    switch (type) {
        case TransactionType::Deposit:     return "DEPOSIT";
        case TransactionType::Withdrawal:  return "WITHDRAWAL";
        case TransactionType::TransferIn:  return "TRANSFER_IN";
        case TransactionType::TransferOut: return "TRANSFER_OUT";
        case TransactionType::Interest:    return "INTEREST";
        case TransactionType::Fee:         return "FEE";
    }
    return "UNKNOWN";
}

TransactionType transactionTypeFromString(const std::string& text) {
    for (auto t : {TransactionType::Deposit, TransactionType::Withdrawal, TransactionType::TransferIn,
                   TransactionType::TransferOut, TransactionType::Interest, TransactionType::Fee}) {
        if (toString(t) == text) return t;
    }
    throw StorageException("Unknown transaction type: " + text);
}

} // namespace bank
