#include "optlib/OptimizationTask.hpp"
#include <cmath>
#include <gtest/gtest.h>

using namespace optlib;

// 1. Коректне виконання основних операцій (метод золотого перетину та тернарний пошук)
TEST(OptimizationTaskTest, GoldenSectionFindsQuadraticMinimum) {
    auto f = [](double x) { return (x - 3.25) * (x - 3.25) + 1.5; };
    OptimizationTask task(0.0, 10.0, 1e-7, 500, SearchMethod::GoldenSection);
    task.optimize(f);

    EXPECT_TRUE(task.hasResult());
    EXPECT_TRUE(task.checkInvariants());
    EXPECT_NEAR(task.getBestX(), 3.25, 1e-6);
    EXPECT_NEAR(task.getBestValue(), 1.5, 1e-10);
    EXPECT_GT(task.getIterationsUsed(), 0u);
}

TEST(OptimizationTaskTest, TernarySearchFindsCosineMinimum) {
    auto f = [](double x) { return std::cos(x); };
    OptimizationTask task(1.0, 5.0, 1e-6, 500, SearchMethod::TernarySearch);
    task.optimize(f);

    EXPECT_TRUE(task.hasResult());
    EXPECT_NEAR(task.getBestX(), 3.14159265, 1e-5);
    EXPECT_NEAR(task.getBestValue(), -1.0, 1e-9);
}

// 2. Перевірка кожного сценарію помилки (EXPECT_THROW)
TEST(OptimizationExceptionTest, ThrowsOnInvalidIntervalAndTolerance) {
    EXPECT_THROW(OptimizationTask(5.0, 5.0, 1e-5), std::invalid_argument);
    EXPECT_THROW(OptimizationTask(6.0, 2.0, 1e-5), std::invalid_argument);
    EXPECT_THROW(OptimizationTask(0.0, 5.0, 0.0), std::invalid_argument);
    EXPECT_THROW(OptimizationTask(0.0, 5.0, -1e-4), std::invalid_argument);
    EXPECT_THROW(OptimizationTask(0.0, 5.0, 10.0), std::invalid_argument);
    EXPECT_THROW(OptimizationTask(0.0, 5.0, 1e-5, 0), std::invalid_argument);
    EXPECT_THROW(OptimizationTask(0.0, 5.0, 1e-5, OptimizationTask::MAX_ALLOWED_ITERATIONS + 1),
                 std::invalid_argument);
}

TEST(OptimizationExceptionTest, ThrowsNumericalDomainOnNanOrInf) {
    EXPECT_THROW(OptimizationTask(std::nan(""), 5.0, 1e-5), NumericalDomainException);
    EXPECT_THROW(OptimizationTask(0.0, std::numeric_limits<double>::infinity(), 1e-5), NumericalDomainException);

    OptimizationTask task(0.0, 4.0, 1e-5);
    auto nanFunc = [](double x) { return (x > 1.5) ? std::nan("") : x * x; };
    EXPECT_THROW(task.optimize(nanFunc), NumericalDomainException);
}

TEST(OptimizationExceptionTest, ThrowsIterationLimitWhenNotConverged) {
    auto f = [](double x) { return (x - 2.0) * (x - 2.0); };
    OptimizationTask task(0.0, 10.0, 1e-8, 3);
    EXPECT_THROW(task.optimize(f), IterationLimitException);
    EXPECT_FALSE(task.hasResult());
}

TEST(OptimizationExceptionTest, ThrowsOnNullFunctionAndOutOfRangeHistory) {
    OptimizationTask task(0.0, 4.0, 1e-4);
    EXPECT_THROW(task.optimize(nullptr), std::invalid_argument);
    EXPECT_THROW(task.getHistoryRecord(0), std::out_of_range);

    task.optimize([](double x) { return x * x; });
    EXPECT_NO_THROW(task.getHistoryRecord(0));
    EXPECT_THROW(task.getHistoryRecord(task.getHistorySize()), std::out_of_range);
}

// 3. Граничні значення аргументів та інваріанти
TEST(OptimizationBoundaryTest, BoundaryMinimumAndSingleIteration) {
    auto linear = [](double x) { return 2.0 * x + 1.0; }; // Мінімум на лівій межі x = -2.0
    OptimizationTask task(-2.0, 4.0, 1e-6, 500);
    task.optimize(linear);

    EXPECT_TRUE(task.checkInvariants());
    EXPECT_NEAR(task.getBestX(), -2.0, 1e-5);
    EXPECT_NEAR(task.getBestValue(), -3.0, 1e-5);
}

// 4. Перевірка сильної гарантії безпеки (Strong Exception Safety Guarantee)
TEST(OptimizationSafetyTest, StrongGuaranteePreservesStateOnFailure) {
    auto goodFunc = [](double x) { return (x - 3.0) * (x - 3.0) + 2.0; };
    OptimizationTask task(0.0, 6.0, 1e-6, 500, SearchMethod::GoldenSection);
    task.optimize(goodFunc);

    const double prevA = task.getLowerBound();
    const double prevB = task.getUpperBound();
    const double prevTol = task.getTolerance();
    const double prevX = task.getBestX();
    const double prevVal = task.getBestValue();
    const std::size_t prevIter = task.getIterationsUsed();

    // Невдала багатокрокова операція (цільова функція генерує NaN)
    auto failingFunc = [](double x) { return (x > 0.0) ? std::nan("") : x * x; };
    EXPECT_THROW(task.updateAndOptimize(-10.0, 10.0, 1e-7, 400, failingFunc, SearchMethod::TernarySearch),
                 NumericalDomainException);

    // Стан об'єкта повинен залишитися повністю незмінним
    EXPECT_TRUE(task.hasResult());
    EXPECT_TRUE(task.checkInvariants());
    EXPECT_DOUBLE_EQ(task.getLowerBound(), prevA);
    EXPECT_DOUBLE_EQ(task.getUpperBound(), prevB);
    EXPECT_DOUBLE_EQ(task.getTolerance(), prevTol);
    EXPECT_DOUBLE_EQ(task.getBestX(), prevX);
    EXPECT_DOUBLE_EQ(task.getBestValue(), prevVal);
    EXPECT_EQ(task.getIterationsUsed(), prevIter);
}
