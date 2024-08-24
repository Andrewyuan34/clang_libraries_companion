cmake_minimum_required(VERSION 3.14)

project(PragmaFinder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})

llvm_map_components_to_libnames(llvm_libs support core irreader)
set(clang_libs clangTooling clangBasic clangFrontend clangLex clangAST clangSerialization clangParse clangSema clangAnalysis clangDriver)

add_executable(FindPragmas main.cpp)

target_link_libraries(FindPragmas PRIVATE ${llvm_libs} ${clang_libs} clangTooling clangFrontend clangAST clangLex)

message(STATUS "LLVM Libraries: ${llvm_libs}")
message(STATUS "Clang Libraries: ${clang_libs}")
