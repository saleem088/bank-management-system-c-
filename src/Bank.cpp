#include "bank/Bank.h"
#include "bank/CheckingAccount.h"
#include "bank/Exceptions.h"
#include "bank/SavingsAccount.h"
#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace bank {

namespace {

void validateOwner(const std::string& owner) {
    if (owner.empty()) throw InvalidInputException("Name cannot be empty.");
    if (owner.find_first_of("|\n\r") != std::string::npos)
        throw InvalidInputException("Name contains invalid characters.");
}

void validatePin(const std::string& pin) {
    if (pin.size() != 4) throw InvalidInputException("PIN must be exactly 4 digits.");
    for (unsigned char c : pin)
        if (!std::isdigit(c)) throw InvalidInputException("PIN must be exactly 4 digits.");
}

std::vector<std::string> split(const std::string& line, char delim) {
    std::vector<std::string> parts;
    std::string item;
    std::istringstream stream(line);
    while (std::getline(stream, item, delim)) parts.push_back(item);
    if (!line.empty() && line.back() == delim) parts.emplace_back();
    return parts;
}

long long toNumber(const std::string& text) {
    try { return std::stoll(text); }
    catch (const std::exception&) { throw StorageException("Corrupt number in data file: " + text); }
}

} // namespace

int Bank::createAccount(AccountType type, const std::string& owner,
                        const std::string& pin, Cents initialDeposit) {
    validateOwner(owner);
    validatePin(pin);
    if (initialDeposit < 0) throw InvalidAmountException();

    const int id = nextId_++;
    const std::string hash = hashPin(pin, id);
    std::unique_ptr<Account> account;
    if (type == AccountType::Savings) account = std::make_unique<SavingsAccount>(id, owner, hash);
    else                              account = std::make_unique<CheckingAccount>(id, owner, hash);

    if (initialDeposit > 0) account->deposit(initialDeposit);
    accounts_[id] = std::move(account);
    return id;
}

Account& Bank::get(int id) {
    auto it = accounts_.find(id);
    if (it == accounts_.end()) throw AccountNotFoundException(id);
    return *it->second;
}

const Account& Bank::get(int id) const {
    auto it = accounts_.find(id);
    if (it == accounts_.end()) throw AccountNotFoundException(id);
    return *it->second;
}

Account& Bank::authenticate(int id, const std::string& pin) {
    Account& account = get(id);
    if (!account.verifyPin(pin)) throw AuthenticationException();
    return account;
}

std::vector<const Account*> Bank::allAccounts() const {
    std::vector<const Account*> result;
    for (const auto& [id, account] : accounts_) result.push_back(account.get());
    return result;
}

void Bank::closeAccount(int id, const std::string& pin) {
    Account& account = authenticate(id, pin);
    if (account.balance() != 0)
        throw BankException("Balance must be $0.00 before closing (withdraw or transfer it first).");
    accounts_.erase(id);
}

void Bank::deposit(int id, Cents amount) { get(id).deposit(amount); }

void Bank::withdraw(int id, const std::string& pin, Cents amount) {
    authenticate(id, pin).withdraw(amount);
}

void Bank::transfer(int fromId, const std::string& pin, int toId, Cents amount) {
    if (fromId == toId) throw InvalidInputException("Cannot transfer to the same account.");
    Account& from = authenticate(fromId, pin);
    Account& to = get(toId);              // fail early if the target does not exist
    from.sendTransfer(amount, toId);      // throws before any money moves
    to.receiveTransfer(amount, fromId);
}

void Bank::applyMonthEnd() {
    for (auto& [id, account] : accounts_) account->applyMonthEnd();  // virtual dispatch
}

// ---------------------------------------------------------------------------
// File format (one record per line, '|' separated):
//   BANK|1|<nextId>
//   A|<Savings|Checking>|<id>|<owner>|<pinHash>|<balanceCents>|<parameter>
//   T|<type>|<amount>|<balanceAfter>|<timestamp>|<note>     (belongs to previous A)
// ---------------------------------------------------------------------------
void Bank::save(const std::string& path) const {
    const std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) throw StorageException("Cannot write to " + tmp);
        out << "BANK|1|" << nextId_ << '\n';
        for (const auto& [id, a] : accounts_) {
            out << "A|" << a->typeName() << '|' << a->id() << '|' << a->owner() << '|'
                << a->pinHash() << '|' << a->balance() << '|' << a->parameter() << '\n';
            for (const auto& t : a->history())
                out << "T|" << toString(t.type) << '|' << t.amount << '|' << t.balanceAfter
                    << '|' << t.timestamp << '|' << t.note << '\n';
        }
        if (!out) throw StorageException("Failed while writing " + tmp);
    }
    // Write to a temp file and rename, so a crash mid-save can't corrupt the real file.
    if (std::rename(tmp.c_str(), path.c_str()) != 0)
        throw StorageException("Cannot replace " + path);
}

void Bank::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) return;  // first run: nothing to load yet

    std::map<int, std::unique_ptr<Account>> loaded;
    std::map<int, std::vector<Transaction>> histories;
    int nextId = 1001, currentId = -1;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        const auto f = split(line, '|');
        if (f[0] == "BANK" && f.size() >= 3) {
            nextId = static_cast<int>(toNumber(f[2]));
        } else if (f[0] == "A" && f.size() == 7) {
            const int id = static_cast<int>(toNumber(f[2]));
            const Cents balance = toNumber(f[5]);
            const long long param = toNumber(f[6]);
            if (f[1] == "Savings")
                loaded[id] = std::make_unique<SavingsAccount>(id, f[3], f[4], balance, static_cast<int>(param));
            else if (f[1] == "Checking")
                loaded[id] = std::make_unique<CheckingAccount>(id, f[3], f[4], balance, param);
            else throw StorageException("Unknown account type: " + f[1]);
            currentId = id;
        } else if (f[0] == "T" && f.size() == 6 && currentId != -1) {
            histories[currentId].push_back({transactionTypeFromString(f[1]), toNumber(f[2]),
                                            toNumber(f[3]), static_cast<std::time_t>(toNumber(f[4])), f[5]});
        } else {
            throw StorageException("Corrupt line in data file: " + line);
        }
    }
    for (auto& [id, history] : histories) loaded[id]->restoreHistory(std::move(history));
    accounts_ = std::move(loaded);
    nextId_ = nextId;
}

} // namespace bank
