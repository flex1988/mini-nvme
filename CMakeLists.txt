project(mini-nvme)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(mini-nvme STATIC ${SOURCES})

set(INCLUDE_PATHS "include"
                  "src")

target_include_directories(mini-nvme PRIVATE ${INCLUDE_PATHS})

add_executable(nvme_example ${PROJECT_SOURCE_DIR}/example/nvme_init.cpp)
target_include_directories(nvme_example PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_dependencies(nvme_example mini-nvme)
target_link_libraries(nvme_example mini-nvme)

