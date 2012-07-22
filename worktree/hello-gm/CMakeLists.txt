cmake_minimum_required(VERSION 2.8)
project(hello-gm)

set(funklibs_DIR ../funk/lib)

find_package(qibuild REQUIRED)
find_package(funklibs REQUIRED)

# funk code
include_directories(../funk)
#include_directories(../funk/lib)
#include_directories(../funk/lib/devil/include)
#include_directories(../funk/lib/fmod/include)
#include_directories(../funk/lib/glew/include)
#include_directories(../funk/lib/opencl/inc) # not much use
#include_directories(../funk/lib/sdl/include)
#link_directories(../funk/lib/sdl/lib)

# SDL
#set(SDL_INCLUDE_DIR ../funk/lib/sdl/include)
#set(SDL_LIBRARY SDL.lib)

#find_package(SDL)
#add_library(SDL SDL.lib)
#target_link_libraries(hello-gm SDL)

qi_submodule_create(funk SRC ../funk/*/*.cpp)
		
# Create a executable named hello-gm
qi_create_bin(hello-gm
	*.cpp
	# */*.cpp  # for subtrees, but will fail without at least one folder
	SUBMODULE funk
)

#qi_create_lib(funk_dependencies
	#../funk/lib/devil/lib/*.lib
	#../funk/lib/fmod/lib/*.lib
	#../funk/lib/glew/lib/glew32s.lib
	#../funk/lib/glew/lib/glut32.lib
	#../funk/lib/sdl/lib/SDL.lib
#)

# add naoqi libs
qi_use_lib(hello-gm ALCOMMON)
qi_use_lib(hello-gm SDL)
qi_use_lib(hello-gm GLEW)
qi_use_lib(hello-gm FMOD)
qi_use_lib(hello-gm DEVIL)

