cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(okuu CXX C)

set(LIB_INCLUDE)
set(LIB_LINK)

# shogle
set(SHOGLE_LIB "lib/shogle")
add_subdirectory(${SHOGLE_LIB})
list(APPEND LIB_INCLUDE "${SHOGLE_LIB}/shogle")
list(APPEND LIB_LINK shogle)

# luajit
find_package(PkgConfig REQUIRED)
pkg_search_module(LuaJIT REQUIRED luajit)
list(APPEND LIB_INCLUDE ${LuaJIT_INCLUDE_DIRS})
list(APPEND LIB_LINK ${LuaJIT_LIBRARIES})

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O0 -g2 -ggdb -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O3 -Wno-psabi")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC lib src ${LIB_INCLUDE})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} shogle ${LIB_LINK})
