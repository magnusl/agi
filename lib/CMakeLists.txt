find_package(Boost REQUIRED COMPONENTS system filesystem)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ../include
)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

SET(CMAKE_CXX_FLAGS "-std=c++14 -Wno-attributes")

add_library(agi
	directory.cpp
	util.cpp
	logic.cpp
	interpreter.cpp
	picture.cpp
	framebuffer.cpp
	view.cpp
	#object_table.cpp
	script_loader.cpp
	volume_loader.cpp
	commands.cpp
	view_loader.cpp
	picture_loader.cpp
	cmd_exec.cpp
	conditions.cpp
	cycle.cpp
	input.cpp
	objects.cpp
	object.cpp
)
