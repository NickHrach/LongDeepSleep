
cmake_minimum_required(VERSION 3.10)
project(LongDeepSleepUnitTests)

# Verwende C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-D_MOCK_TEST_)

# Hier: Pfade zum src-Ordner und Mocks
include_directories(
    ${CMAKE_SOURCE_DIR}/../src
    ${CMAKE_SOURCE_DIR}
)

# GoogleTest-Quellen einbinden (Pfad zu deinem lokalen GTest)
#add_subdirectory("C:/libs/googletest" googletest-build)

# Testquelle und Produktionsquelle
set(SRC_FILES
    ${CMAKE_SOURCE_DIR}/../src/LongDeepSleep.cpp
    main.cpp
)

# Mocks (optional: falls du sie als extra Quellen brauchst)
file(GLOB MOCK_HEADERS
    ${CMAKE_SOURCE_DIR}/Mock*.h
)

# Test-Executable
add_executable(run_tests
    ${SRC_FILES}
    ${MOCK_HEADERS}
)

# Verlinkung mit GoogleTest-Bibliotheken
target_link_libraries(run_tests gtest gtest_main)

# Ausgabeverzeichnis setzen
set_target_properties(run_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
