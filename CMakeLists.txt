cmake_minimum_required(VERSION 3.16)
project(Cxml VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt5
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# Find GTest
find_package(GTest REQUIRED)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Create main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    Qt5::Core 
    Qt5::Widgets
)

# Enable testing
enable_testing()

# Test executable
add_executable(${PROJECT_NAME}_tests "test/main.cpp" "test/xml_parser_test.cpp")
target_link_libraries(${PROJECT_NAME}_tests 
    GTest::gtest 
    GTest::gtest_main
    Qt5::Core 
    Qt5::Widgets
)

# Add tests
add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)

# Set compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -O2)
    target_compile_options(${PROJECT_NAME}_tests PRIVATE -Wall -Wextra -O2)
endif()

# Qt MOC
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

set_target_properties(${PROJECT_NAME}_tests PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
) 