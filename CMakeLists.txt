cmake_minimum_required(VERSION 3.10)
project(easydb)

add_executable(easydb main.cpp bplus.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_compile_options(-fexec-charset=UTF-8)
add_compile_options(-finput-charset=GBK)
add_compile_options(-fwide-exec-charset=UTF-8)