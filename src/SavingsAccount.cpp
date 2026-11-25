#include "bank/SavingsAccount.h"

namespace bank {

SavingsAccount::SavingsAccount(int id, std::string owner, std::string pinHash,
                               Cents balance, int rateBasisPoints)
    : Account(id, std::move(owner), std::move(pinHash), balance),
      rateBasisPoints_(rateBasisPoints) {}

void SavingsAccount::applyMonthEnd() {
    if (balance() <= 0) return;
    const Cents interest = balance() * rateBasisPoints_ / 10000 / 12;
    if (interest > 0) apply(interest, TransactionType::Interest, "Monthly interest");
}

} // namespace bank
