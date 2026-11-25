cmake_minimum_required(VERSION 3.14)
project(BankSystem VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core logic lives in a library so both the app and the tests can use it.
add_library(bank_core
    src/Money.cpp src/Transaction.cpp src/Account.cpp
    src/SavingsAccount.cpp src/CheckingAccount.cpp src/Bank.cpp)
target_include_directories(bank_core PUBLIC include)
if(MSVC)
    target_compile_options(bank_core PRIVATE /W4)
else()
    target_compile_options(bank_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(bank_app src/main.cpp)
target_link_libraries(bank_app PRIVATE bank_core)

# Tests (Catch2 is downloaded automatically). Disable with -DBUILD_TESTING=OFF
option(BUILD_TESTING "Build unit tests" ON)
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2)
    FetchContent_MakeAvailable(Catch2)

    add_executable(bank_tests tests/test_bank.cpp)
    target_link_libraries(bank_tests PRIVATE bank_core Catch2::Catch2WithMain)

    enable_testing()
    include(CTest)
    include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
    catch_discover_tests(bank_tests)
endif()
