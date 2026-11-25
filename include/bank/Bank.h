#pragma once
#include "bank/Account.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace bank {

class Bank {
public:
    // --- account lifecycle ---
    int createAccount(AccountType type, const std::string& owner,
                      const std::string& pin, Cents initialDeposit = 0);
    void closeAccount(int id, const std::string& pin);

    // --- lookups ---
    Account& get(int id);
    const Account& get(int id) const;
    std::size_t accountCount() const { return accounts_.size(); }
    std::vector<const Account*> allAccounts() const;

    // --- operations (PIN is required for anything that takes money out) ---
    void deposit(int id, Cents amount);
    void withdraw(int id, const std::string& pin, Cents amount);
    void transfer(int fromId, const std::string& pin, int toId, Cents amount);
    void applyMonthEnd();

    // --- persistence ---
    void save(const std::string& path) const;
    void load(const std::string& path);

private:
    Account& authenticate(int id, const std::string& pin);

    // unique_ptr: the map owns each account, no manual new/delete anywhere.
    std::map<int, std::unique_ptr<Account>> accounts_;
    int nextId_ = 1001;
};

} // namespace bank
