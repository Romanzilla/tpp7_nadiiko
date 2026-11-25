#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

namespace optlib {

// Базовий клас ієрархії користувацьких винятків бібліотеки оптимізації (Варіант 18)
class OptimizationException : public std::runtime_error {
private:
    std::string paramName_;
    std::string invalidValue_;
    std::string validRange_;

    static std::string formatMessage(const std::string& msg,
                                     const std::string& param,
                                     const std::string& val,
                                     const std::string& range) {
        std::ostringstream oss;
        oss << msg;
        if (!param.empty()) {
            oss << " [параметр: " << param;
            if (!val.empty()) oss << ", отримано: " << val;
            if (!range.empty()) oss << ", допустимо: " << range;
            oss << "]";
        }
        return oss.str();
    }

public:
    OptimizationException(const std::string& message,
                          std::string paramName = "",
                          std::string invalidValue = "",
                          std::string validRange = "")
        : std::runtime_error(formatMessage(message, paramName, invalidValue, validRange)),
          paramName_(std::move(paramName)),
          invalidValue_(std::move(invalidValue)),
          validRange_(std::move(validRange)) {}

    const std::string& paramName() const noexcept { return paramName_; }
    const std::string& invalidValue() const noexcept { return invalidValue_; }
    const std::string& validRange() const noexcept { return validRange_; }
};

// Помилка області визначення / нескінченних чи NaN значень
class NumericalDomainException : public OptimizationException {
public:
    NumericalDomainException(const std::string& message,
                             const std::string& paramName,
                             double badValue,
                             const std::string& validRange = "скінченне дійсне число (std::isfinite)")
        : OptimizationException(message, paramName, std::to_string(badValue), validRange) {}
};

// Помилка відсутності збіжності методу оптимізації
class ConvergenceException : public OptimizationException {
private:
    double achievedError_;
    double targetTolerance_;

public:
    ConvergenceException(const std::string& message,
                         double achievedError,
                         double targetTolerance)
        : OptimizationException(
              message,
              "precision",
              std::to_string(achievedError),
              "<= " + std::to_string(targetTolerance)),
          achievedError_(achievedError),
          targetTolerance_(targetTolerance) {}

    double achievedError() const noexcept { return achievedError_; }
    double targetTolerance() const noexcept { return targetTolerance_; }
};

// Перевищення граничної кількості ітерацій або ліміту обчислювальних ресурсів
class IterationLimitException : public ConvergenceException {
private:
    std::size_t iterationsUsed_;
    std::size_t maxIterations_;

public:
    IterationLimitException(std::size_t iterationsUsed,
                            std::size_t maxIterations,
                            double achievedError,
                            double targetTolerance)
        : ConvergenceException(
              "Перевищено максимальну кількість ітерацій (" +
                  std::to_string(iterationsUsed) + " / " + std::to_string(maxIterations) + ")",
              achievedError,
              targetTolerance),
          iterationsUsed_(iterationsUsed),
          maxIterations_(maxIterations) {}

    std::size_t iterationsUsed() const noexcept { return iterationsUsed_; }
    std::size_t maxIterations() const noexcept { return maxIterations_; }
};

} // namespace optlib
