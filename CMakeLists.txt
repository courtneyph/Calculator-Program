cmake_minimum_required(VERSION 3.19)
project(spring2024-csc3380-lecture7)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

enable_testing()
include(CTest)

add_executable(calculator_final calculator.cpp)

add_test(NAME CalculatorTests COMMAND calculator_final --run-tests)