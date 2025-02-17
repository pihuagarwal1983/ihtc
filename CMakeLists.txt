# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(IHTC LANGUAGES C)

# Set the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define the source and include directories
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

# Add source files (ihtc.c, ihtc_ga.c, ihtc_improved_heuristic.c, and cJSON.c)
set(SOURCES
    ${SRC_DIR}/ihtc.c                    # Main source file
    ${SRC_DIR}/ihtc_ga.c                  # Genetic Algorithm implementation
    ${SRC_DIR}/ihtc_ga_optional.c
    ${SRC_DIR}/ihtc_improved_heuristic.c  # Improved heuristic implementation
    ${LIB_DIR}/cJSON/cJSON.c              # cJSON implementation
)

# Define the executable target
add_executable(IHTC ${SOURCES})

# Ensure the `data/instances` and `data/testInstances` directories are copied to the build directory
file(COPY ${DATA_DIR}/instances DESTINATION ${CMAKE_BINARY_DIR}/data)
file(COPY ${DATA_DIR}/testInstances DESTINATION ${CMAKE_BINARY_DIR}/data)

# Ensure the `output` directory exists in the build directory
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# Compiler warnings (optional, to enforce good coding practices)
if(MSVC)
    target_compile_options(IHTC PRIVATE /W4) # For Visual Studio
else()
    target_compile_options(IHTC PRIVATE -Wall -Wextra -pedantic) # For GCC/Clang
endif()

# Print helpful messages
message(STATUS "Source Directory: ${SRC_DIR}")
message(STATUS "Include Directory: ${INCLUDE_DIR}")
message(STATUS "Library Directory: ${LIB_DIR}")
message(STATUS "Data Directory: ${DATA_DIR}")
message(STATUS "Build Output Directory: ${OUTPUT_DIR}")

# Optional: Add a target for testing (e.g., test_json.c)
add_executable(TestRunner
    ${TEST_DIR}/test_json.c   # Test source file in the tests/ directory
    ${LIB_DIR}/cJSON/cJSON.c  # cJSON implementation
)
