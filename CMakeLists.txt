cmake_minimum_required(VERSION 3.18)

project(simpjson)

set(CMAKE_CXX_STANDARD 20)

aux_source_directory(./src SRC_LIST)

add_executable(simpjson ${SRC_LIST})
