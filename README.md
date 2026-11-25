# Bank Management System (C++17)

A console banking application built to practise modern C++ design: inheritance and
polymorphism, RAII with smart pointers, exceptions, file persistence, and unit tests.

## Features
- Savings (earns interest) and Checking (overdraft limit, monthly fee) accounts
- Deposit, withdraw, and transfer between accounts, with PIN protection
- Per-account transaction history and statements
- Month-end processing through virtual dispatch (`applyMonthEnd()`)
- Data saved to disk after every change, loaded on startup (atomic write via temp file + rename)
- Input validation and a custom exception hierarchy

## Design highlights
| Decision | Why |
|---|---|
| Money stored as integer cents | Floating point cannot represent money exactly |
| `Account` is abstract; `SavingsAccount` / `CheckingAccount` override it | `Bank` never needs `if (type == ...)` chains |
| `std::map<int, std::unique_ptr<Account>>` | O(log n) lookup, automatic ownership, no `new`/`delete` |
| `BankException` hierarchy | Business rules throw, the UI catches and prints one clean message |
| Core logic in `bank_core` library | The CLI and the tests share the same code |
| Transfers validate before moving money | A failed transfer leaves both accounts untouched |
| PINs are salted and hashed | No plain-text PINs on disk (educational hash; use bcrypt/argon2 in production) |

## Project layout
```
include/bank/   public headers
src/            implementations + main.cpp (CLI)
tests/          Catch2 unit tests
data/           runtime save file (git-ignored)
```

## Build and run
```bash
cmake -S . -B build
cmake --build build
./build/bank_app            # run the program
./build/bank_tests          # run the tests
```
Requires CMake 3.14+ and a C++17 compiler. Catch2 is downloaded automatically.
Use `-DBUILD_TESTING=OFF` to skip tests.

## Sample session
```
Account created. Your account number is 1001.
Transfer complete.
Savings account #1001 - Alice      Balance: $800.50
Date              Type          Amount      Balance  Note
2026-11-25 08:18  DEPOSIT    $1,000.50    $1,000.50  Deposit
2026-11-25 08:18  TRANSFER_OUT $200.00      $800.50  To account 1002
```
(Replace this with a real screenshot or GIF of your own run.)

## What I learned
- Designing class hierarchies around behaviour rather than data
- Why exact integer arithmetic matters for money
- Exception safety: validate first, mutate last
- Structuring a project so the logic is testable without the UI

## Future improvements
- Lock an account after 3 wrong PIN attempts
- Replace the text file with SQLite
- Proper password hashing (argon2)
- A scheduled month-end job and a standalone report generator
- GitHub Actions CI building on Linux, macOS and Windows
