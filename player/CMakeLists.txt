include_directories(../include)

SET(CMAKE_CXX_FLAGS "-std=c++14")

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(playagi
	main.cpp
)

target_link_libraries(playagi agi)
target_link_libraries(playagi ${SDL2_LIBRARIES})
target_link_libraries(playagi ${Boost_LIBRARIES})
