#pragma once
#include <stdexcept>
#include <string>

namespace bank {

// Base class so callers can catch every bank error with one handler.
class BankException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class InvalidAmountException : public BankException {
public:
    InvalidAmountException() : BankException("Amount must be greater than zero.") {}
};

class InsufficientFundsException : public BankException {
public:
    InsufficientFundsException() : BankException("Insufficient funds.") {}
};

class AccountNotFoundException : public BankException {
public:
    explicit AccountNotFoundException(int id)
        : BankException("Account " + std::to_string(id) + " not found.") {}
};

class AuthenticationException : public BankException {
public:
    AuthenticationException() : BankException("Incorrect PIN.") {}
};

class InvalidInputException : public BankException {
public:
    using BankException::BankException;
};

class StorageException : public BankException {
public:
    using BankException::BankException;
};

} // namespace bank
