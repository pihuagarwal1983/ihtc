# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(IHTC LANGUAGES C)

# Set the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)          # Source files
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)  # Header files
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)          # Libraries
set(DATA_DIR ${CMAKE_SOURCE_DIR}/data)        # JSON input files
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/output)    # Output directory for generated files
set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)       # Test directory

# Include directories (cJSON and custom headers)
include_directories(
    ${INCLUDE_DIR}            # Custom headers
    ${LIB_DIR}/cJSON          # cJSON library
)

# Add source files
set(SOURCES
    ${SRC_DIR}/ihtc.c
    ${SRC_DIR}/ihtc_ga.c
    ${SRC_DIR}/ihtc_ga_optional.c
    ${SRC_DIR}/ihtc_ga_parallel.c
    ${SRC_DIR}/new_ga.c
    ${SRC_DIR}/newMutationGA.c
    ${SRC_DIR}/nurseGA.c
    ${LIB_DIR}/cJSON/cJSON.c
)

# Define the executable target
add_executable(IHTC ${SOURCES})

# Set output directories
set_target_properties(IHTC PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Ensure required directories exist before copying
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# Copy data/instances and data/testInstances safely
if(EXISTS ${DATA_DIR}/instances)
    file(COPY ${DATA_DIR}/instances DESTINATION ${CMAKE_BINARY_DIR}/data)
endif()

if(EXISTS ${DATA_DIR}/testInstances)
    file(COPY ${DATA_DIR}/testInstances DESTINATION ${CMAKE_BINARY_DIR}/data)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(IHTC PRIVATE /W4)
else()
    target_compile_options(IHTC PRIVATE -Wall -Wextra -pedantic)
endif()

# Print directories
message(STATUS "Source Directory: ${SRC_DIR}")
message(STATUS "Include Directory: ${INCLUDE_DIR}")
message(STATUS "Library Directory: ${LIB_DIR}")
message(STATUS "Data Directory: ${DATA_DIR}")
message(STATUS "Build Output Directory: ${OUTPUT_DIR}")

# Optional: Add a test target
add_executable(TestRunner
    ${TEST_DIR}/test_json.c
    ${LIB_DIR}/cJSON/cJSON.c
)

# Ensure TestRunner uses same include directories
target_include_directories(TestRunner PRIVATE ${INCLUDE_DIR} ${LIB_DIR}/cJSON)

# Set output directories for TestRunner
set_target_properties(TestRunner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

