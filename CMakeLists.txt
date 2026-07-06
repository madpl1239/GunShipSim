# CMakeLists.txt for gunsim
# 19-06-2026 by madpl

cmake_minimum_required(VERSION 4.2.2)
project(gunsim)

file(GLOB_RECURSE SOURCE ./source/*)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2.5 REQUIRED COMPONENTS
	system window graphics network)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/source)
target_link_libraries(${CMAKE_PROJECT_NAME}
	sfml-system sfml-window sfml-graphics sfml-network GLEW GL)

if(DEBUG)
	target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
		-O2 -march=native -Wall -Wextra -Wpedantic
		-Wreturn-local-addr -Wdangling-else -Wno-unused-parameter -DDEBUG)
else()
	target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
		-O2 -march=native -Wall -Wextra -Wpedantic
		-Wreturn-local-addr -Wdangling-else -Wno-unused-parameter)
endif()
