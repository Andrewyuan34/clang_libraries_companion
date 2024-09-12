cmake_minimum_required(VERSION 3.14)

project(if-stmt-counter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# 添加 main.cpp 和 utilities.cpp 到可执行文件
add_executable(if-stmt-counter main.cpp)

# Set no RTTI for the specific target
target_compile_options(if-stmt-counter PRIVATE -fno-rtti)

# Include directories for LLVM and Clang
target_include_directories(if-stmt-counter PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})

# 如果 utilities.h 在单独的 include 目录中，比如 include 目录
# target_include_directories(if-stmt-counter PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link libraries for LLVM and Clang
target_link_libraries(if-stmt-counter PRIVATE LLVM clangTooling clangFrontend clangAST clangLex)

message(STATUS "LLVM Libraries: ${llvm_libs}")
message(STATUS "Clang Libraries: ${clang_libs}")
