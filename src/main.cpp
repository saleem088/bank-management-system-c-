#include "bank/Bank.h"
#include "bank/Exceptions.h"
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>

using namespace bank;

namespace {

std::string prompt(const std::string& label) {
    std::cout << label;
    std::string line;
    if (!std::getline(std::cin, line)) std::exit(0);   // Ctrl+D / end of input
    return line;
}

int promptInt(const std::string& label) {
    const std::string text = prompt(label);
    try { return std::stoi(text); }
    catch (const std::exception&) { throw InvalidInputException("'" + text + "' is not a valid number."); }
}

Cents promptMoney(const std::string& label) { return parseMoney(prompt(label)); }

std::string formatTime(std::time_t t) {
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M", std::localtime(&t));
    return buf;
}

void printMenu() {
    std::cout << "\n===== Bank Management System =====\n"
                 " 1. Create account\n"
                 " 2. Deposit\n"
                 " 3. Withdraw\n"
                 " 4. Transfer\n"
                 " 5. Account statement\n"
                 " 6. List all accounts\n"
                 " 7. Close account\n"
                 " 8. Run month-end (interest & fees)\n"
                 " 0. Save and exit\n";
}

void createAccount(Bank& bank) {
    const std::string kind = prompt("Account type (1 = Savings, 2 = Checking): ");
    if (kind != "1" && kind != "2") throw InvalidInputException("Choose 1 or 2.");
    const std::string name = prompt("Owner name: ");
    const std::string pin = prompt("Choose a 4-digit PIN: ");
    const Cents initial = promptMoney("Initial deposit (0 for none): ");
    const int id = bank.createAccount(kind == "1" ? AccountType::Savings : AccountType::Checking,
                                      name, pin, initial);
    std::cout << "Account created. Your account number is " << id << ".\n";
}

void statement(Bank& bank) {
    const int id = promptInt("Account number: ");
    const std::string pin = prompt("PIN: ");
    const Account& a = bank.get(id);
    if (!a.verifyPin(pin)) throw AuthenticationException();

    std::cout << "\n" << a.typeName() << " account #" << a.id() << " - " << a.owner()
              << "\nBalance: " << formatMoney(a.balance()) << "\n\n"
              << std::left << std::setw(18) << "Date" << std::setw(14) << "Type"
              << std::right << std::setw(14) << "Amount" << std::setw(14) << "Balance" << "  Note\n"
              << std::string(78, '-') << '\n';
    for (const auto& t : a.history()) {
        std::cout << std::left << std::setw(18) << formatTime(t.timestamp) << std::setw(14) << toString(t.type)
                  << std::right << std::setw(14) << formatMoney(t.amount)
                  << std::setw(14) << formatMoney(t.balanceAfter) << "  " << t.note << '\n';
    }
}

void listAccounts(const Bank& bank) {
    std::cout << "\n" << std::left << std::setw(8) << "ID" << std::setw(20) << "Owner"
              << std::setw(10) << "Type" << std::right << std::setw(14) << "Balance" << '\n'
              << std::string(52, '-') << '\n';
    for (const Account* a : bank.allAccounts())
        std::cout << std::left << std::setw(8) << a->id() << std::setw(20) << a->owner()
                  << std::setw(10) << a->typeName() << std::right << std::setw(14)
                  << formatMoney(a->balance()) << '\n';
    std::cout << bank.accountCount() << " account(s)\n";
}

// Returns true if the choice changed data (so the caller saves).
bool handle(Bank& bank, const std::string& choice) {
    if (choice == "1") { createAccount(bank); return true; }
    if (choice == "2") {
        const int id = promptInt("Account number: ");
        bank.deposit(id, promptMoney("Amount: "));
        std::cout << "New balance: " << formatMoney(bank.get(id).balance()) << '\n';
        return true;
    }
    if (choice == "3") {
        const int id = promptInt("Account number: ");
        const std::string pin = prompt("PIN: ");
        bank.withdraw(id, pin, promptMoney("Amount: "));
        std::cout << "New balance: " << formatMoney(bank.get(id).balance()) << '\n';
        return true;
    }
    if (choice == "4") {
        const int from = promptInt("From account: ");
        const std::string pin = prompt("PIN: ");
        const int to = promptInt("To account: ");
        bank.transfer(from, pin, to, promptMoney("Amount: "));
        std::cout << "Transfer complete.\n";
        return true;
    }
    if (choice == "5") { statement(bank); return false; }
    if (choice == "6") { listAccounts(bank); return false; }
    if (choice == "7") {
        const int id = promptInt("Account number: ");
        bank.closeAccount(id, prompt("PIN: "));
        std::cout << "Account closed.\n";
        return true;
    }
    if (choice == "8") { bank.applyMonthEnd(); std::cout << "Month-end processed.\n"; return true; }
    std::cout << "Unknown option.\n";
    return false;
}

} // namespace

int main(int argc, char* argv[]) {
    const std::string dataFile = argc > 1 ? argv[1] : "data/bank.dat";
    Bank bank;
    try {
        bank.load(dataFile);
    } catch (const BankException& e) {
        std::cerr << "Could not load data: " << e.what() << '\n';
        return 1;
    }

    while (true) {
        printMenu();
        const std::string choice = prompt("Choose an option: ");
        if (choice == "0") break;
        try {
            if (handle(bank, choice)) bank.save(dataFile);
        } catch (const BankException& e) {
            std::cout << "Error: " << e.what() << '\n';
        }
    }
    bank.save(dataFile);
    std::cout << "Goodbye!\n";
}
