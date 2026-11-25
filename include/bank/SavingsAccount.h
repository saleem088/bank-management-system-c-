#pragma once
#include "bank/Account.h"

namespace bank {

class SavingsAccount : public Account {
public:
    // rateBasisPoints: 250 means 2.50% per year (integers avoid float errors).
    SavingsAccount(int id, std::string owner, std::string pinHash,
                   Cents balance = 0, int rateBasisPoints = 250);

    AccountType type() const override { return AccountType::Savings; }
    void applyMonthEnd() override;
    Cents availableFunds() const override { return balance(); }
    long long parameter() const override { return rateBasisPoints_; }

private:
    int rateBasisPoints_;
};

} // namespace bank
