#include "bank/CheckingAccount.h"

namespace bank {

CheckingAccount::CheckingAccount(int id, std::string owner, std::string pinHash,
                                 Cents balance, Cents overdraftLimit)
    : Account(id, std::move(owner), std::move(pinHash), balance),
      overdraftLimit_(overdraftLimit) {}

void CheckingAccount::applyMonthEnd() {
    apply(-kMonthlyFee, TransactionType::Fee, "Monthly maintenance fee");
}

} // namespace bank
