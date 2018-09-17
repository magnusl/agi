include_directories(../include)

SET(CMAKE_CXX_FLAGS "-std=c++14")

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(showpic
	showpic.cpp
)

target_link_libraries(showpic agi)
target_link_libraries(showpic ${SDL2_LIBRARIES})
target_link_libraries(showpic ${Boost_LIBRARIES})

add_executable(showview
	showview.cpp
)

target_link_libraries(showview agi)
target_link_libraries(showview ${SDL2_LIBRARIES})
target_link_libraries(showview ${Boost_LIBRARIES})
