cmake_minimum_required(VERSION 2.8)
project(hello-gm)

set(funklibs_DIR ../funk/lib)

find_package(qibuild REQUIRED)
find_package(funklibs REQUIRED)

include_directories(../funk)
include_directories(../glm-0.9.3.4)

# hackery to put all the funk source into vcproj
file(GLOB_RECURSE FUNK_SRCS ../funk/*.cpp)
file(GLOB_RECURSE FUNK_HDRS ../funk/*.h)
file(GLOB_RECURSE FUNK_GM ../funk/*.gm)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE HDRS *.h)
file(GLOB_RECURSE GMS *.gm)

add_custom_target(AUX_FILES SOURCES ${FUNK_HDRS} ${FUNK_GM} ${HDRS} ${GMS})

# TODO: copy the dlls to output somehow!

qi_submodule_create(funk SRC ${FUNK_SRCS})

qi_create_bin(hello-gm
	${SRCS}
	# */*.cpp  # for subtrees, but will fail without at least one folder
	SUBMODULE funk
)

qi_use_lib(hello-gm ALCOMMON)
qi_use_lib(hello-gm SDL)
qi_use_lib(hello-gm GLEW)
qi_use_lib(hello-gm FMOD)
qi_use_lib(hello-gm DEVIL)
