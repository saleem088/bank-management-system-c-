#pragma once
#include "bank/Account.h"

namespace bank {

class CheckingAccount : public Account {
public:
    CheckingAccount(int id, std::string owner, std::string pinHash,
                    Cents balance = 0, Cents overdraftLimit = 50000);

    AccountType type() const override { return AccountType::Checking; }
    void applyMonthEnd() override;
    Cents availableFunds() const override { return balance() + overdraftLimit_; }
    long long parameter() const override { return overdraftLimit_; }

    static constexpr Cents kMonthlyFee = 500; // $5.00

private:
    Cents overdraftLimit_;
};

} // namespace bank
