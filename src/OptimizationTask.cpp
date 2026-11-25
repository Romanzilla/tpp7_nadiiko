#include "optlib/OptimizationTask.hpp"
#include "optlib/ScopeLogger.hpp"

#include <cmath>
#include <iostream>
#include <utility>

namespace optlib {

void OptimizationTask::validateParameters(double a, double b, double tol, std::size_t maxIter) {
    if (!std::isfinite(a)) {
        throw NumericalDomainException("Нижня межа інтервалу має бути скінченним числом", "lowerBound", a);
    }
    if (!std::isfinite(b)) {
        throw NumericalDomainException("Верхня межа інтервалу має бути скінченним числом", "upperBound", b);
    }
    if (a >= b) {
        throw std::invalid_argument("Некоректний інтервал пошуку: нижня межа має бути меншою за верхню (a < b)");
    }
    if (!std::isfinite(tol)) {
        throw NumericalDomainException("Точність пошуку має бути скінченним числом", "tolerance", tol);
    }
    if (tol <= 0.0 || tol >= (b - a)) {
        throw std::invalid_argument("Точність має задовольняти умову 0 < tolerance < (b - a)");
    }
    if (maxIter == 0 || maxIter > MAX_ALLOWED_ITERATIONS) {
        throw std::invalid_argument("Кількість ітерацій має бути в діапазоні [1, 100000]");
    }
}

double OptimizationTask::evaluateChecked(const ObjectiveFunc& func, double x) {
    if (!func) {
        throw std::invalid_argument("Цільова функція не задана (порожній std::function)");
    }
    double y = func(x);
    if (!std::isfinite(y)) {
        throw NumericalDomainException(
            "Цільова функція повернула некоректне значення (NaN або Inf) у точці x = " + std::to_string(x),
            "f(x)",
            y);
    }
    return y;
}

OptimizationTask::OptimizationTask(double lowerBound,
                                   double upperBound,
                                   double tolerance,
                                   std::size_t maxIterations,
                                   SearchMethod method)
    : lowerBound_(lowerBound),
      upperBound_(upperBound),
      tolerance_(tolerance),
      maxIterations_(maxIterations),
      method_(method),
      hasResult_(false),
      bestX_(0.0),
      bestValue_(0.0),
      iterationsUsed_(0) {
    validateParameters(lowerBound_, upperBound_, tolerance_, maxIterations_);
    if (method_ != SearchMethod::GoldenSection && method_ != SearchMethod::TernarySearch) {
        throw std::invalid_argument("Невідомий метод одномірної оптимізації");
    }
}

void OptimizationTask::swap(OptimizationTask& other) noexcept {
    using std::swap;
    swap(lowerBound_, other.lowerBound_);
    swap(upperBound_, other.upperBound_);
    swap(tolerance_, other.tolerance_);
    swap(maxIterations_, other.maxIterations_);
    swap(method_, other.method_);
    swap(hasResult_, other.hasResult_);
    swap(bestX_, other.bestX_);
    swap(bestValue_, other.bestValue_);
    swap(iterationsUsed_, other.iterationsUsed_);
    history_.swap(other.history_);
}

void OptimizationTask::optimize(const ObjectiveFunc& func) {
    if (!func) {
        throw std::invalid_argument("Неможливо виконати оптимізацію: цільова функція порожня");
    }

    double a = lowerBound_;
    double b = upperBound_;
    std::size_t iter = 0;
    std::vector<IterationRecord> localHistory;
    localHistory.reserve(maxIterations_ < 256 ? maxIterations_ : 256);

    if (method_ == SearchMethod::GoldenSection) {
        const double invPhi = (std::sqrt(5.0) - 1.0) / 2.0; // ~0.6180339887
        double x1 = b - invPhi * (b - a);
        double x2 = a + invPhi * (b - a);
        double f1 = evaluateChecked(func, x1);
        double f2 = evaluateChecked(func, x2);

        while ((b - a) > tolerance_) {
            if (iter >= maxIterations_) {
                throw IterationLimitException(iter, maxIterations_, b - a, tolerance_);
            }
            ++iter;

            if (f1 < f2) {
                b = x2;
                x2 = x1;
                f2 = f1;
                x1 = b - invPhi * (b - a);
                f1 = evaluateChecked(func, x1);
                localHistory.push_back({iter, a, b, x1, f1});
            } else {
                a = x1;
                x1 = x2;
                f1 = f2;
                x2 = a + invPhi * (b - a);
                f2 = evaluateChecked(func, x2);
                localHistory.push_back({iter, a, b, x2, f2});
            }
        }
    } else {
        while ((b - a) > tolerance_) {
            if (iter >= maxIterations_) {
                throw IterationLimitException(iter, maxIterations_, b - a, tolerance_);
            }
            ++iter;

            double m1 = a + (b - a) / 3.0;
            double m2 = b - (b - a) / 3.0;
            double f1 = evaluateChecked(func, m1);
            double f2 = evaluateChecked(func, m2);

            if (f1 < f2) {
                b = m2;
                localHistory.push_back({iter, a, b, m1, f1});
            } else {
                a = m1;
                localHistory.push_back({iter, a, b, m2, f2});
            }
        }
    }

    double finalX = 0.5 * (a + b);
    double finalF = evaluateChecked(func, finalX);

    // Етап фіксації (commit): не генерує винятків (no-throw)
    bestX_ = finalX;
    bestValue_ = finalF;
    iterationsUsed_ = iter;
    hasResult_ = true;
    history_ = std::move(localHistory);
}

void OptimizationTask::updateAndOptimize(double newLowerBound,
                                         double newUpperBound,
                                         double newTolerance,
                                         std::size_t newMaxIterations,
                                         const ObjectiveFunc& func,
                                         SearchMethod newMethod) {
    // 1. Prepare: створюємо тимчасовий об'єкт і виконуємо всі потенційно небезпечні операції на ньому
    OptimizationTask temp(newLowerBound, newUpperBound, newTolerance, newMaxIterations, newMethod);
    temp.optimize(func);

    // 2. Commit: фіксуємо новий стан через noexcept swap (Strong Exception Safety Guarantee)
    swap(temp);
}

void OptimizationTask::clearResult() noexcept {
    hasResult_ = false;
    bestX_ = 0.0;
    bestValue_ = 0.0;
    iterationsUsed_ = 0;
    history_.clear();
}

const IterationRecord& OptimizationTask::getHistoryRecord(std::size_t index) const {
    if (index >= history_.size()) {
        throw std::out_of_range("Індекс історії ітерацій виходить за допустимі межі");
    }
    return history_[index];
}

bool OptimizationTask::checkInvariants() const noexcept {
    if (!std::isfinite(lowerBound_) || !std::isfinite(upperBound_) || !(lowerBound_ < upperBound_)) {
        return false;
    }
    if (!std::isfinite(tolerance_) || tolerance_ <= 0.0 || tolerance_ >= (upperBound_ - lowerBound_)) {
        return false;
    }
    if (maxIterations_ == 0 || maxIterations_ > MAX_ALLOWED_ITERATIONS) {
        return false;
    }
    if (hasResult_) {
        if (!std::isfinite(bestX_) || bestX_ < lowerBound_ || bestX_ > upperBound_) {
            return false;
        }
        if (!std::isfinite(bestValue_)) {
            return false;
        }
    }
    return true;
}

namespace {

// Рівень 3 (нижній): безпосередній запуск обчислень, де генерується виняток
double executeSearchWorker(double a, double b, double tol, const OptimizationTask::ObjectiveFunc& func) {
    ScopeLogger logger("Рівень 3: executeSearchWorker()");
    OptimizationTask task(a, b, tol, 500, SearchMethod::GoldenSection);
    task.optimize(func);
    return task.getBestX();
}

// Рівень 2 (проміжний): перехоплення, логування контексту та повторне генерування через throw;
double runSearchStage(double a, double b, double tol, const OptimizationTask::ObjectiveFunc& func) {
    ScopeLogger logger("Рівень 2: runSearchStage()");
    try {
        return executeSearchWorker(a, b, tol, func);
    } catch (const OptimizationException& e) {
        std::cout << "  [Проміжний рівень] Діагностика перед повторним throw; -> параметр: '"
                  << e.paramName() << "', значення: " << e.invalidValue() << '\n';
        throw; // Повторне генерування без зрізання типу (object slicing)
    }
}

} // namespace

// Рівень 1 (верхній): ініціація конвеєра оптимізації
double runOptimizationPipeline(double a, double b, double tol, const OptimizationTask::ObjectiveFunc& func) {
    ScopeLogger logger("Рівень 1: runOptimizationPipeline()");
    return runSearchStage(a, b, tol, func);
}

} // namespace optlib
