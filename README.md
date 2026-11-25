# Лабораторна робота №7 — Обробка винятків та модульне тестування (Варіант 18)

**Виконавець:** студент ФІТ 3-10 Надійко Роман Олександрович  
**Дисципліна:** Технології прикладного програмування  
**Тема варіанту №18:** Пошук екстремумів (`OptimizationTask`)

## 1. Інваріанти класу `OptimizationTask`
1. `lowerBound_ < upperBound_`, обидві межі є скінченними дійсними числами (`std::isfinite`).
2. `0 < tolerance_ < (upperBound_ - lowerBound_)` та `std::isfinite(tolerance_)`.
3. `1 <= maxIterations_ <= MAX_ALLOWED_ITERATIONS` (`100 000`).
4. Якщо `hasResult_ == true`, то `bestX_ ∈ [lowerBound_, upperBound_]` і `std::isfinite(bestValue_)`.

## 2. Ієрархія класів винятків та гарантії безпеки
* `OptimizationException` (успадкований від `std::runtime_error`) — зберігає назву параметра, отримане значення та допустимий діапазон:
  * `NumericalDomainException` — значення `NaN` або `Inf` у межах інтервалу, точності або результаті `f(x)`.
  * `ConvergenceException` — відсутність збіжності методу в межах заданої точності.
  * `IterationLimitException` (похідний від `ConvergenceException`) — перевищення `maxIterations_`.
* Стандартні винятки: `std::invalid_argument` (некоректний інтервал `a >= b`, `tolerance <= 0`, порожня цільова функція), `std::out_of_range` (некоректний індекс історії ітерацій).
* **Сильна гарантія безпеки (`Strong`):** метод `updateAndOptimize(...)` працює за принципом *Prepare-then-Commit / Copy-and-Swap* — створює тимчасовий об'єкт `temp`, виконує повний розрахунок і фіксує зміни через `swap(temp)` (`noexcept`) лише після успішного завершення.

## 3. Дослідження розкручування стека (`ScopeLogger`)
**Очікувана послідовність повідомлень до запуску:**
1. `[ScopeLogger] Створено: Рівень 1: runOptimizationPipeline()`
2. `[ScopeLogger] Створено: Рівень 2: runSearchStage()`
3. `[ScopeLogger] Створено: Рівень 3: executeSearchWorker()`
4. Генерація `NumericalDomainException` у `executeSearchWorker()`
5. `[ScopeLogger] Знищено:  Рівень 3: executeSearchWorker()` (розкручування стека при виході з рівня 3)
6. `[Проміжний рівень] Діагностика перед повторним throw;` та повторне поширення через `throw;`
7. `[ScopeLogger] Знищено:  Рівень 2: runSearchStage()`
8. `[ScopeLogger] Знищено:  Рівень 1: runOptimizationPipeline()`
9. `[main] Фінальне перехоплення: ...`

**Фактичний результат:** повністю збігається з очікуваним порядком (деструктори локальних об'єктів викликаються у зворотному порядку їх створення).

## 4. Висновок щодо винятків примітивних типів (`demo/bad_exceptions.cpp`)
Викидання примітивних типів (`int`, `std::string`) не перехоплюється стандартним обробником `catch (const std::exception&)`, позбавляє програму поліморфної ієрархії помилок і змушує використовувати сліпий `catch (...)` без доступу до діагностичної інформації. Тому у надійному коді C++ необхідно використовувати класи винятків, успадковані від `std::exception`.

## 5. Швидке збирання та запуск (GitHub Codespaces / Linux)
```bash
sudo apt-get update -qq && sudo apt-get install -y -qq cmake g++ libgtest-dev
cmake -B build && cmake --build build -j
./build/demo_app
./build/bad_exceptions
./build/runTests
```
