cmake_minimum_required(VERSION 2.8)
project(hello)

find_package(qibuild)


# Create a executable named hello
# with the source file: main.cpp
qi_create_bin(hello "main.cpp")

# add naoqi libs
qi_use_lib(hello ALCOMMON)