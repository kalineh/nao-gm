cmake_minimum_required(VERSION 2.8)
project(hello-gm)

set(funklibs_DIR ../funk/lib)

find_package(qibuild REQUIRED)
find_package(funklibs REQUIRED)

include_directories(../funk)

qi_submodule_create(funk SRC ../funk/*/*.cpp)
		
qi_create_bin(hello-gm
	*.cpp
	# */*.cpp  # for subtrees, but will fail without at least one folder
	SUBMODULE funk
)

qi_use_lib(hello-gm ALCOMMON)
qi_use_lib(hello-gm SDL)
qi_use_lib(hello-gm GLEW)
qi_use_lib(hello-gm FMOD)
qi_use_lib(hello-gm DEVIL)

