cmake_minimum_required(VERSION 3.15)
project(wasm-interpreter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/types.cpp
    src/module.cpp
    src/decoder.cpp
    src/stack.cpp
    src/memory.cpp
    src/interpreter.cpp
    src/instructions.cpp
)

set(HEADERS
    include/types.h
    include/module.h
    include/decoder.h
    include/stack.h
    include/memory.h
    include/interpreter.h
    include/instructions.h
)

add_executable(wasm-interpreter ${SOURCES} ${HEADERS})

if(WIN32)
    target_compile_definitions(wasm-interpreter PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS wasm-interpreter DESTINATION bin)

# Test sources (excluding main.cpp)
set(TEST_SOURCES
    src/types.cpp
    src/module.cpp
    src/decoder.cpp
    src/stack.cpp
    src/memory.cpp
    src/interpreter.cpp
    src/instructions.cpp
)

# Test executables
add_executable(test_runner tests/test_runner.cpp ${TEST_SOURCES})
target_include_directories(test_runner PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_executable(test_runner_02 tests/test_runner_02.cpp ${TEST_SOURCES})
target_include_directories(test_runner_02 PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_executable(test_runner_03 tests/test_runner_03.cpp ${TEST_SOURCES})
target_include_directories(test_runner_03 PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Unified test runner that runs all tests
add_executable(run_all_tests tests/run_all_tests.cpp ${TEST_SOURCES})
target_include_directories(run_all_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)

message(STATUS "")
message(STATUS "Build Configuration:")
message(STATUS "  Project:      ${PROJECT_NAME}")
message(STATUS "  Version:      ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:   ${CMAKE_BUILD_TYPE}")
message(STATUS "  System:       ${CMAKE_SYSTEM_NAME}")
message(STATUS "")
message(STATUS "Test Executables:")
message(STATUS "  test_runner      - Test suite 01 (i32 operations)")
message(STATUS "  test_runner_02   - Test suite 02 (floats & functions)")
message(STATUS "  test_runner_03   - Test suite 03 (i64 & tables)")
message(STATUS "  run_all_tests    - Unified test runner (all 167 tests)")
message(STATUS "")
