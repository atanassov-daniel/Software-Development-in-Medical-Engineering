cmake_minimum_required(VERSION 3.5)

project(MyLibUnitTests VERSION 0.1 LANGUAGES CXX)

find_package(Qt6Test REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

enable_testing(true)

add_executable(MyLibUnitTests tst_mylibunittest.cpp)
add_test(MyLibUnitTests MyLibTestClass)

add_dependencies(MyLibUnitTests MyLib)
target_link_libraries(MyLibUnitTests PRIVATE Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Widgets MyLib)
