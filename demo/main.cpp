#include "optlib/OptimizationTask.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    using namespace optlib;

    std::cout << "=== Демонстрація модернізованої бібліотеки OptimizationTask (Варіант 18) ===\n\n";

    // 1. Нормальне виконання пошуку мінімуму f(x) = (x - 3)^2 + 2 на [0, 6]
    auto parabola = [](double x) { return (x - 3.0) * (x - 3.0) + 2.0; };
    OptimizationTask task(0.0, 6.0, 1e-6, 500, SearchMethod::GoldenSection);
    task.optimize(parabola);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "1) Успішний пошук мінімуму f(x) = (x - 3)^2 + 2 на [0, 6]:\n";
    std::cout << "   x_min = " << task.getBestX() << ", f(x_min) = " << task.getBestValue()
              << ", ітерацій = " << task.getIterationsUsed()
              << ", інваріанти = " << (task.checkInvariants() ? "OK" : "FAIL") << "\n\n";

    // 2. Демонстрація сильної гарантії безпеки (Strong Exception Safety Guarantee)
    std::cout << "2) Перевірка сильної гарантії безпеки (updateAndOptimize з помилкою NaN):\n";
    auto brokenFunc = [](double x) -> double {
        if (x > 2.5) return std::nan("");
        return x * x;
    };
    try {
        task.updateAndOptimize(-5.0, 5.0, 1e-6, 500, brokenFunc);
    } catch (const NumericalDomainException& e) {
        std::cout << "   Перехоплено NumericalDomainException: " << e.what() << '\n';
    }
    std::cout << "   Стан об'єкта після винятку: інтервал [" << task.getLowerBound()
              << ", " << task.getUpperBound() << "], x_min = " << task.getBestX()
              << ", f_min = " << task.getBestValue() << " (не змінився!)\n\n";

    // 3. Демонстрація ієрархії винятків та правильного порядку блоків catch
    std::cout << "3) Демонстрація різних сценаріїв помилок (від спеціалізованого до базового catch):\n";
    for (int scenario = 1; scenario <= 4; ++scenario) {
        try {
            if (scenario == 1) {
                OptimizationTask badInterval(10.0, 2.0, 1e-5);
            } else if (scenario == 2) {
                OptimizationTask fewIters(0.0, 10.0, 1e-9, 3);
                fewIters.optimize(parabola);
            } else if (scenario == 3) {
                task.getHistoryRecord(9999);
            } else {
                OptimizationTask emptyFunc(0.0, 5.0, 1e-5);
                emptyFunc.optimize(nullptr);
            }
        } catch (const IterationLimitException& e) {
            std::cout << "   [IterationLimitException] " << e.what() << '\n';
        } catch (const NumericalDomainException& e) {
            std::cout << "   [NumericalDomainException] " << e.what() << '\n';
        } catch (const OptimizationException& e) {
            std::cout << "   [OptimizationException] " << e.what() << '\n';
        } catch (const std::out_of_range& e) {
            std::cout << "   [std::out_of_range] " << e.what() << '\n';
        } catch (const std::invalid_argument& e) {
            std::cout << "   [std::invalid_argument] " << e.what() << '\n';
        } catch (const std::exception& e) {
            std::cout << "   [std::exception] " << e.what() << '\n';
        }
    }

    // 4. Демонстрація розкручування стека (ScopeLogger) та повторного throw;
    std::cout << "\n4) Дослідження розкручування стека (ScopeLogger + throw;):\n";
    try {
        runOptimizationPipeline(0.0, 5.0, 1e-6, brokenFunc);
    } catch (const NumericalDomainException& e) {
        std::cout << "[main] Фінальне перехоплення: " << e.what() << '\n';
    }

    return 0;
}
