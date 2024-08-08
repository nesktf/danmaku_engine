cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(danmaku_engine CXX C)

set(SHOGLE_LIB "lib/shogle")
add_subdirectory(${SHOGLE_LIB})

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O0 -g2 -ggdb -Wno-psabi")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -Wall -Wextra -Wpedantic -O3 -Wno-psabi")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC src "${SHOGLE_LIB}/shogle")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} shogle)
