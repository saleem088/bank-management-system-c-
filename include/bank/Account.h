#pragma once
#include "bank/Money.h"
#include "bank/Transaction.h"
#include <string>
#include <vector>

namespace bank {

enum class AccountType { Savings, Checking };

// Abstract base class. Concrete account types override the pure virtual
// functions, which is what makes the Bank class polymorphic.
class Account {
public:
    Account(int id, std::string owner, std::string pinHash, Cents balance = 0);
    virtual ~Account() = default;

    // --- getters ---
    int id() const { return id_; }
    const std::string& owner() const { return owner_; }
    const std::string& pinHash() const { return pinHash_; }
    Cents balance() const { return balance_; }
    const std::vector<Transaction>& history() const { return history_; }

    // --- behaviour ---
    bool verifyPin(const std::string& pin) const;
    void deposit(Cents amount);
    void withdraw(Cents amount);
    void receiveTransfer(Cents amount, int fromId);
    void sendTransfer(Cents amount, int toId);

    // Used by persistence to rebuild an account without re-running business rules.
    void restoreHistory(std::vector<Transaction> history) { history_ = std::move(history); }

    // --- polymorphic interface ---
    virtual AccountType type() const = 0;
    virtual void applyMonthEnd() = 0;              // interest, fees...
    virtual Cents availableFunds() const = 0;      // how much can be withdrawn
    virtual long long parameter() const = 0;       // value saved to disk (rate / limit)

    std::string typeName() const;

protected:
    void apply(Cents delta, TransactionType type, const std::string& note);

private:
    int id_;
    std::string owner_;
    std::string pinHash_;
    Cents balance_;
    std::vector<Transaction> history_;
};

// Salted FNV-1a hash. Good enough to avoid storing PINs in plain text in a
// learning project; a real system would use bcrypt/argon2.
std::string hashPin(const std::string& pin, int salt);

} // namespace bank
