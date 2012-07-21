cmake_minimum_required(VERSION 2.8)
project(hello)

find_package(qibuild)

# Create a executable named hello
# with the source file: main.cpp
qi_create_bin(hello "main.cpp")

# Add a simple test:
enable_testing()
qi_create_test(test_hello "test.cpp")
