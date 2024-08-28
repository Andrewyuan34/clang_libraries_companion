cmake_minimum_required(VERSION 3.14)

project(IncludeFinder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

add_executable(IncludeFinder main.cpp)

# Set no RTTI for the specific target
target_compile_options(IncludeFinder PRIVATE -fno-rtti)

# Include directories for LLVM and Clang
target_include_directories(IncludeFinder PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})

# Link libraries for LLVM and Clang
target_link_libraries(IncludeFinder PRIVATE LLVM clangTooling clangFrontend clangAST clangLex)

message(STATUS "LLVM Libraries: ${llvm_libs}")
message(STATUS "Clang Libraries: ${clang_libs}")
