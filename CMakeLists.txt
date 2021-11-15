CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

# project name + language used
PROJECT(IPE C CXX)

# debug/release
SET(CMAKE_RELEASE_POSTFIX "")
SET(CMAKE_DEBUG_POSTFIX "-debug")

# specify where compiled files must go
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# add cmake FindXXX script to find pre-compiled libraries if any
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# C++ Standard
# ====================================

SET(CMAKE_CXX_STANDARD 20)
SET(CXX_STANDARD_REQUIRED ON)

# Project's Options
# ====================================

OPTION(BUILD_TESTING        "Enable tests"       OFF)
OPTION(BUILD_BENCHMARK      "Build Benchmarks"   OFF)
OPTION(BUILD_EXAMPLES       "Build Examples"     OFF)
OPTION(BUILD_DOCUMENTATION  "Build docs"         OFF)

# Binary/pre-compiled Dependencies
# ====================================
FIND_PACKAGE(Git REQUIRED)


# Configuration
# ====================================

# Find revision ID and hash of the sourcetree
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE(genrev)
INCLUDE(CodeCoverage)

append_coverage_compiler_flags()

# Compiler-flag
IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    MESSAGE(STATUS "Building Debug Version")
    ADD_DEFINITIONS(-O0)
    ADD_LINK_OPTIONS(-rdynamic)         # For stack trace with symbols
ELSE()
    MESSAGE(STATUS "Building Release Version")
    ADD_DEFINITIONS(-O2)
ENDIF()

# Subdirectories
# ====================================

# Dependencies to be compiled
ADD_SUBDIRECTORY(dependencies)

# How to compile main lib
ADD_SUBDIRECTORY(src)

# Compile tests ?
IF(BUILD_TESTING)
    ENABLE_TESTING()
    MESSAGE(STATUS "Building tests")
    ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)

IF (BUILD_EXAMPLES)
    MESSAGE(STATUS "Building examples")
    ADD_SUBDIRECTORY(examples)
ENDIF (BUILD_EXAMPLES)

IF (BUILD_BENCHMARK)
    MESSAGE(STATUS "Building Benchmarks")
    ADD_SUBDIRECTORY(benchmark)
ENDIF(BUILD_BENCHMARK)

# Gen doc ?
IF (BUILD_DOCUMENTATION)
    MESSAGE(STATUS "Documentation building is enabled")
    ADD_SUBDIRECTORY(docs)
ENDIF (BUILD_DOCUMENTATION)


# Helper commands
# ====================================
# so you do not have to leave the command line
# or remember the exact arguments

ADD_CUSTOM_TARGET(enable-examples
    cmake -DBUILD_EXAMPLES=ON ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(disable-examples
    cmake -DBUILD_EXAMPLES=OFF ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(enable-release
    cmake -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(enable-doc
    cmake -DBUILD_DOCUMENTATION=ON ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(enable-bench
    cmake -DBUILD_BENCHMARK=ON ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(enable-debug
    cmake -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(enable-test
    cmake -DBUILD_TESTING=ON ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(compile-db DEBUG
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ${CMAKE_SOURCE_DIR})

ADD_CUSTOM_TARGET(clang-tidy DEBUG
    clang-tidy -p= ${CMAKE_BINARY_DIR} -header-filter=.*
        ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)


# Resource Files
# ====================================
# Some IDEs need the files to be inside a cmake target to be displayed

ADD_CUSTOM_TARGET(zothers ALL SOURCES
    README.md
    .travis.yml
    .gitignore
    .gitmodules
    .clang-format
)
