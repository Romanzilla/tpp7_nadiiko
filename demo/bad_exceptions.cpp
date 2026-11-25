#include <exception>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

void triggerPrimitiveException(int mode) {
    if (mode == 1) {
        throw 404; // Примітивний тип int
    }
    throw std::string("Помилка у вигляді std::string, не успадкована від std::exception");
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    std::cout << "=== Демонстрація некоректного підходу (викидання int та std::string) ===\n";

    for (int mode : {1, 2}) {
        try {
            triggerPrimitiveException(mode);
        } catch (const std::exception& e) {
            // Сюди керування НІКОЛИ не потрапить для int та std::string!
            std::cout << "Перехоплено через std::exception: " << e.what() << '\n';
        } catch (...) {
            std::cout << "Сценарій " << mode
                      << ": catch (const std::exception&) пропустив виняток! "
                         "Спрацював лише універсальний обробник catch (...) без доступу до тексту помилки.\n";
        }
    }
    return 0;
}
