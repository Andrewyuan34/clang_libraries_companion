cmake_minimum_required(VERSION 3.14)

project(dead_store LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# 添加 main.cpp 和 utilities.cpp 到可执行文件
add_executable(dead_store main.cpp)

# Set no RTTI for the specific target
target_compile_options(dead_store PRIVATE -fno-rtti)

# Include directories for LLVM and Clang
target_include_directories(dead_store PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})

# 如果 utilities.h 在单独的 include 目录中，比如 include 目录
# target_include_directories(dead_store PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link libraries for LLVM and Clang
target_link_libraries(dead_store PRIVATE LLVM clangTooling clangFrontend clangAST clangLex)

message(STATUS "LLVM Libraries: ${llvm_libs}")
message(STATUS "Clang Libraries: ${clang_libs}")
