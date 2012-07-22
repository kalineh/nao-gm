cmake_minimum_required(VERSION 2.8)
project(hello-gm)

find_package(qibuild)

# funk code
include_directories(../funk)
qi_submodule_create(funk SRC ../funk/*/*.cpp)

# Create a executable named hello-gm
qi_create_bin(hello-gm
	*.cpp
	#*/*.cpp
	SUBMODULE funk
)

# add naoqi libs
qi_use_lib(hello-gm ALCOMMON)