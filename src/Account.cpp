#include "bank/Account.h"
#include "bank/Exceptions.h"
#include <cstdint>
#include <ctime>

namespace bank {

namespace {
void requirePositive(Cents amount) {
    if (amount <= 0) throw InvalidAmountException();
}
} // namespace

Account::Account(int id, std::string owner, std::string pinHash, Cents balance)
    : id_(id), owner_(std::move(owner)), pinHash_(std::move(pinHash)), balance_(balance) {}

bool Account::verifyPin(const std::string& pin) const {
    return hashPin(pin, id_) == pinHash_;
}

void Account::apply(Cents delta, TransactionType type, const std::string& note) {
    balance_ += delta;
    history_.push_back({type, delta < 0 ? -delta : delta, balance_, std::time(nullptr), note});
}

void Account::deposit(Cents amount) {
    requirePositive(amount);
    apply(amount, TransactionType::Deposit, "Deposit");
}

void Account::withdraw(Cents amount) {
    requirePositive(amount);
    if (amount > availableFunds()) throw InsufficientFundsException();
    apply(-amount, TransactionType::Withdrawal, "Withdrawal");
}

void Account::sendTransfer(Cents amount, int toId) {
    requirePositive(amount);
    if (amount > availableFunds()) throw InsufficientFundsException();
    apply(-amount, TransactionType::TransferOut, "To account " + std::to_string(toId));
}

void Account::receiveTransfer(Cents amount, int fromId) {
    requirePositive(amount);
    apply(amount, TransactionType::TransferIn, "From account " + std::to_string(fromId));
}

std::string Account::typeName() const {
    return type() == AccountType::Savings ? "Savings" : "Checking";
}

std::string hashPin(const std::string& pin, int salt) {
    std::uint64_t h = 1469598103934665603ULL ^ static_cast<std::uint64_t>(salt);
    for (unsigned char c : pin) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return std::to_string(h);
}

} // namespace bank
