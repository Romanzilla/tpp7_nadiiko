cmake_minimum_required(VERSION 3.16)
project(tpp7_nadiiko VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 1. Статична бібліотека оптимізації (Варіант 18)
add_library(optlib STATIC
    src/OptimizationTask.cpp
)
target_include_directories(optlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 2. Демонстраційна програма
add_executable(demo_app demo/main.cpp)
target_link_libraries(demo_app PRIVATE optlib)

# 3. Демонстрація некоректного використання винятків (int / std::string)
add_executable(bad_exceptions demo/bad_exceptions.cpp)

# 4. Модульні тести Google Test (підтримка vcpkg, системного libgtest-dev та FetchContent)
enable_testing()
find_package(GTest CONFIG QUIET)
if(NOT GTest_FOUND)
    find_package(GTest QUIET)
endif()

if(GTest_FOUND)
    add_executable(runTests tests/test_optimization.cpp)
    if(TARGET GTest::gtest_main)
        target_link_libraries(runTests PRIVATE optlib GTest::gtest_main)
    else()
        target_link_libraries(runTests PRIVATE optlib ${GTEST_BOTH_LIBRARIES} pthread)
    endif()
    add_test(NAME OptimizationUnitTests COMMAND runTests)
endif()
