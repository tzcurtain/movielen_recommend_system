cmake_minimum_required(VERSION 3.9)
project(recommendation_sys)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. DIRSRCS)
include_directories(./)
add_executable(recommendation_sys main.cpp utility.cpp utility.h)

