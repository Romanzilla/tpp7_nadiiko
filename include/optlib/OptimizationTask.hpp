#pragma once

#include "Exceptions.hpp"
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace optlib {

enum class SearchMethod {
    GoldenSection,
    TernarySearch
};

struct IterationRecord {
    std::size_t step;
    double left;
    double right;
    double candidateX;
    double candidateF;
};

// Клас пошуку мінімуму унімодальної функції однієї змінної (Варіант 18)
// Інваріанти класу:
//   1) lowerBound_ < upperBound_, обидві межі скінченні (std::isfinite)
//   2) 0 < tolerance_ < (upperBound_ - lowerBound_) та std::isfinite(tolerance_)
//   3) 1 <= maxIterations_ <= MAX_ALLOWED_ITERATIONS
//   4) Якщо hasResult_ == true, то bestX_ належить [lowerBound_, upperBound_] і std::isfinite(bestValue_)
class OptimizationTask {
public:
    using ObjectiveFunc = std::function<double(double)>;
    static constexpr std::size_t MAX_ALLOWED_ITERATIONS = 100000;

private:
    double lowerBound_;
    double upperBound_;
    double tolerance_;
    std::size_t maxIterations_;
    SearchMethod method_;

    bool hasResult_;
    double bestX_;
    double bestValue_;
    std::size_t iterationsUsed_;
    std::vector<IterationRecord> history_;

    static void validateParameters(double a, double b, double tol, std::size_t maxIter);
    static double evaluateChecked(const ObjectiveFunc& func, double x);

public:
    // Конструктор перевіряє інваріанти параметрів (Strong guarantee: у разі винятку об'єкт не створюється)
    OptimizationTask(double lowerBound,
                     double upperBound,
                     double tolerance = 1e-6,
                     std::size_t maxIterations = 1000,
                     SearchMethod method = SearchMethod::GoldenSection);

    // Гарантія відсутності винятків (no-throw) для обміну станами (Copy-and-Swap)
    void swap(OptimizationTask& other) noexcept;

    // Пошук екстремуму на поточному інтервалі
    // Гарантія безпеки: Strong (обчислення виконуються у локальних змінних і фіксуються лише після успіху)
    void optimize(const ObjectiveFunc& func);

    // Багатокрокова операція оновлення параметрів та перерахунку екстремуму
    // Гарантія безпеки: Strong (Prepare-then-Commit / Copy-and-Swap: при будь-якому винятку стан не змінюється)
    void updateAndOptimize(double newLowerBound,
                           double newUpperBound,
                           double newTolerance,
                           std::size_t newMaxIterations,
                           const ObjectiveFunc& func,
                           SearchMethod newMethod = SearchMethod::GoldenSection);

    // Скидання збережених результатів (no-throw)
    void clearResult() noexcept;

    // Доступ до елемента історії ітерацій із перевіркою меж (Strong guarantee)
    const IterationRecord& getHistoryRecord(std::size_t index) const;

    // Селектори властивостей (no-throw guarantee)
    double getLowerBound() const noexcept { return lowerBound_; }
    double getUpperBound() const noexcept { return upperBound_; }
    double getTolerance() const noexcept { return tolerance_; }
    std::size_t getMaxIterations() const noexcept { return maxIterations_; }
    SearchMethod getMethod() const noexcept { return method_; }
    bool hasResult() const noexcept { return hasResult_; }
    double getBestX() const noexcept { return bestX_; }
    double getBestValue() const noexcept { return bestValue_; }
    std::size_t getIterationsUsed() const noexcept { return iterationsUsed_; }
    std::size_t getHistorySize() const noexcept { return history_.size(); }

    // Перевірка інваріантів класу (no-throw)
    bool checkInvariants() const noexcept;
};

// Демонстраційний 3-рівневий ланцюжок викликів із розкручуванням стека та повторним throw;
double runOptimizationPipeline(double a, double b, double tol, const OptimizationTask::ObjectiveFunc& func);

} // namespace optlib
