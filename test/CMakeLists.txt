cmake_minimum_required(VERSION 3.10)
project(LongDeepSleepUnitTests)

# C++11 aktivieren und UNIT_TEST definieren
set(CMAKE_CXX_STANDARD 11)
add_definitions(-D_MOCK_TEST_)

# Include-Pfade setzen: src/ für die Produktiv-Klassen, ./ für Mocks
include_directories(
    ${CMAKE_SOURCE_DIR}/../src
    ${CMAKE_SOURCE_DIR}
)

# Alle Quellen für das Test-Executable
set(SRC_FILES
    ${CMAKE_SOURCE_DIR}/../src/LongDeepSleep.cpp
    main.cpp
)

# Füge alle Mock-Header ein (passt, wenn sie den Präfix "Mock" haben)
file(GLOB MOCK_HEADERS 
    ${CMAKE_SOURCE_DIR}/Mock*.h
)

# Executable definieren
add_executable(run_tests
    ${SRC_FILES}
    ${MOCK_HEADERS}
)

# Damit die Ausgabedatei in diesem Ordner landet:
set_target_properties(run_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
