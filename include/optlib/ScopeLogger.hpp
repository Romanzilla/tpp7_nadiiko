#pragma once

#include <iostream>
#include <string>

namespace optlib {

// Допоміжний RAII-клас для дослідження розкручування стека (stack unwinding)
class ScopeLogger {
private:
    std::string scopeName_;
    std::ostream& out_;

public:
    explicit ScopeLogger(std::string scopeName, std::ostream& out = std::cout) noexcept
        : scopeName_(std::move(scopeName)), out_(out) {
        out_ << "[ScopeLogger] Створено: " << scopeName_ << '\n';
    }

    ~ScopeLogger() noexcept {
        out_ << "[ScopeLogger] Знищено:  " << scopeName_ << '\n';
    }

    ScopeLogger(const ScopeLogger&) = delete;
    ScopeLogger& operator=(const ScopeLogger&) = delete;
};

} // namespace optlib
