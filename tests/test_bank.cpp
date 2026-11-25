#include <catch2/catch_test_macros.hpp>
#include "bank/Bank.h"
#include "bank/Exceptions.h"
#include <cstdio>

using namespace bank;

TEST_CASE("Money formatting and parsing") {
    REQUIRE(formatMoney(123456) == "$1,234.56");
    REQUIRE(formatMoney(-5) == "-$0.05");
    REQUIRE(parseMoney("12.5") == 1250);
    REQUIRE(parseMoney("100") == 10000);
    REQUIRE_THROWS_AS(parseMoney("abc"), InvalidInputException);
    REQUIRE_THROWS_AS(parseMoney("1.234"), InvalidInputException);
    REQUIRE_THROWS_AS(parseMoney("-5"), InvalidInputException);
}

TEST_CASE("Deposits and withdrawals") {
    Bank bank;
    int id = bank.createAccount(AccountType::Savings, "Alice", "1234", 10000);
    bank.deposit(id, 500);
    REQUIRE(bank.get(id).balance() == 10500);
    bank.withdraw(id, "1234", 2500);
    REQUIRE(bank.get(id).balance() == 8000);
    REQUIRE(bank.get(id).history().size() == 3);

    REQUIRE_THROWS_AS(bank.deposit(id, 0), InvalidAmountException);
    REQUIRE_THROWS_AS(bank.withdraw(id, "1234", 999999), InsufficientFundsException);
    REQUIRE_THROWS_AS(bank.withdraw(id, "0000", 100), AuthenticationException);
    REQUIRE(bank.get(id).balance() == 8000);
}

TEST_CASE("Checking accounts can use their overdraft, savings cannot") {
    Bank bank;
    int chk = bank.createAccount(AccountType::Checking, "Bob", "1111", 1000);
    bank.withdraw(chk, "1111", 30000);                       // within $500 overdraft
    REQUIRE(bank.get(chk).balance() == -29000);
    REQUIRE_THROWS_AS(bank.withdraw(chk, "1111", 30000), InsufficientFundsException);

    int sav = bank.createAccount(AccountType::Savings, "Cara", "2222", 1000);
    REQUIRE_THROWS_AS(bank.withdraw(sav, "2222", 1001), InsufficientFundsException);
}

TEST_CASE("Transfers move money atomically") {
    Bank bank;
    int a = bank.createAccount(AccountType::Checking, "A", "1234", 5000);
    int b = bank.createAccount(AccountType::Savings, "B", "4321", 0);
    bank.transfer(a, "1234", b, 2000);
    REQUIRE(bank.get(a).balance() == 3000);
    REQUIRE(bank.get(b).balance() == 2000);

    REQUIRE_THROWS_AS(bank.transfer(a, "1234", 9999, 100), AccountNotFoundException);
    REQUIRE_THROWS_AS(bank.transfer(a, "1234", a, 100), InvalidInputException);
    REQUIRE(bank.get(a).balance() == 3000);                  // nothing changed
}

TEST_CASE("Month-end applies interest to savings and fees to checking") {
    Bank bank;
    int sav = bank.createAccount(AccountType::Savings, "S", "1234", 120000);   // $1,200
    int chk = bank.createAccount(AccountType::Checking, "C", "1234", 10000);
    bank.applyMonthEnd();
    REQUIRE(bank.get(sav).balance() == 120250);              // 2.5%/12 of $1,200 = $2.50
    REQUIRE(bank.get(chk).balance() == 9500);                // $5 fee
}

TEST_CASE("Input validation and closing accounts") {
    Bank bank;
    REQUIRE_THROWS_AS(bank.createAccount(AccountType::Savings, "", "1234"), InvalidInputException);
    REQUIRE_THROWS_AS(bank.createAccount(AccountType::Savings, "X", "12"), InvalidInputException);
    REQUIRE_THROWS_AS(bank.createAccount(AccountType::Savings, "X", "abcd"), InvalidInputException);

    int id = bank.createAccount(AccountType::Savings, "X", "1234", 100);
    REQUIRE_THROWS_AS(bank.closeAccount(id, "1234"), BankException);   // balance not zero
    bank.withdraw(id, "1234", 100);
    bank.closeAccount(id, "1234");
    REQUIRE_THROWS_AS(bank.get(id), AccountNotFoundException);
}

TEST_CASE("Save and load round-trips everything") {
    const std::string path = "test_bank.tmp";
    int a, b;
    {
        Bank bank;
        a = bank.createAccount(AccountType::Savings, "Alice Smith", "1234", 5000);
        b = bank.createAccount(AccountType::Checking, "Bob", "9999", 100);
        bank.transfer(a, "1234", b, 1000);
        bank.save(path);
    }
    Bank loaded;
    loaded.load(path);
    REQUIRE(loaded.accountCount() == 2);
    REQUIRE(loaded.get(a).owner() == "Alice Smith");
    REQUIRE(loaded.get(a).balance() == 4000);
    REQUIRE(loaded.get(b).balance() == 1100);
    REQUIRE(loaded.get(a).history().size() == 2);
    REQUIRE(loaded.get(a).verifyPin("1234"));
    int c = loaded.createAccount(AccountType::Savings, "New", "0000");
    REQUIRE(c > b);                                          // IDs keep increasing after reload
    std::remove(path.c_str());
}
