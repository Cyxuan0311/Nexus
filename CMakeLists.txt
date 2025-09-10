cmake_minimum_required(VERSION 3.16)
project(Nexus VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Find Qt5
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# Find GTest
find_package(GTest REQUIRED)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set icon directory
set(ICON_DIR ${CMAKE_BINARY_DIR}/bin/icon)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/ui
    ${CMAKE_SOURCE_DIR}/include/core
    ${CMAKE_SOURCE_DIR}/include/syntax
    ${CMAKE_SOURCE_DIR}/include/features
)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Group files by feature for IDEs
source_group("UI" FILES 
    src/ui/main_window.cpp include/ui/main_window.h 
    src/ui/search_dialog.cpp include/ui/search_dialog.h 
    src/ui/code_folding.cpp include/ui/code_folding.h)
source_group("Core/Parsing" FILES 
    src/core/xml_parser.cpp include/core/xml_parser.h 
    src/core/xml_node.cpp include/core/xml_node.h)
source_group("Core/Serialization" FILES 
    src/core/xml_serializer.cpp include/core/xml_serializer.h)
source_group("Syntax/XML" FILES 
    src/syntax/xml_highlighter.cpp include/syntax/xml_highlighter.h)
source_group("Syntax/Markdown" FILES 
    src/syntax/markdown_highlighter.cpp include/syntax/markdown_highlighter.h)
source_group("Syntax/CPP" FILES 
    src/syntax/cpp_highlighter.cpp include/syntax/cpp_highlighter.h)
source_group("Syntax/Python" FILES 
    src/syntax/python_highlighter.cpp include/syntax/python_highlighter.h)
source_group("Syntax/Go" FILES 
    src/syntax/go_highlighter.cpp include/syntax/go_highlighter.h)
source_group("Core/CPP" FILES 
    src/core/cpp_parser.cpp include/core/cpp_parser.h)
source_group("Core/Python" FILES 
    src/core/python_parser.cpp include/core/python_parser.h)
source_group("Core/Go" FILES 
    src/core/go_parser.cpp include/core/go_parser.h)
source_group("UI/Graph" FILES 
    src/ui/function_graph_view.cpp include/ui/function_graph_view.h)
source_group("App" FILES src/app/main.cpp)
source_group("Tests" FILES 
    test/main.cpp test/xml_parser_test.cpp test/xml_serializer_test.cpp test/search_test.cpp test/code_folding_test.cpp)

# Create main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    Qt5::Core 
    Qt5::Widgets
)

# Copy icon files to build directory
file(MAKE_DIRECTORY ${ICON_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/icon/ DESTINATION ${ICON_DIR})

# Add custom target to ensure icons are copied after build
add_custom_target(copy_icons ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ICON_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/icon ${ICON_DIR}
    COMMENT "Copying icon files to build directory"
)
add_dependencies(${PROJECT_NAME} copy_icons)

# Enable testing (disabled for now due to Qt MOC issues)
# enable_testing()

# Test executable - exclude main.cpp to avoid main function conflict
# Commented out due to Qt MOC linking issues
# file(GLOB_RECURSE TEST_SOURCES "src/*.cpp")
# list(REMOVE_ITEM TEST_SOURCES "${CMAKE_SOURCE_DIR}/src/app/main.cpp")

# add_executable(${PROJECT_NAME}_tests 
#     "test/main.cpp" 
#     "test/xml_parser_test.cpp" 
#     "test/xml_serializer_test.cpp" 
#     "test/search_test.cpp" 
#     "test/code_folding_test.cpp"
#     ${TEST_SOURCES}
# )

# Set Qt properties for test executable
# set_target_properties(${PROJECT_NAME}_tests PROPERTIES
#     AUTOMOC ON
#     AUTOUIC ON
#     AUTORCC ON
# )

# target_link_libraries(${PROJECT_NAME}_tests 
#     GTest::gtest 
#     GTest::gtest_main
#     Qt5::Core 
#     Qt5::Widgets
# )

# Add compile definitions for tests
# target_compile_definitions(${PROJECT_NAME}_tests PRIVATE QT_TESTCASE_BUILDDIR="${CMAKE_BINARY_DIR}")

# Add tests
# add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)

# Set compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )
endif()

# Set additional compiler flags for MSVC
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()

# Qt MOC
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# Installation rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install icon files
install(DIRECTORY ${CMAKE_SOURCE_DIR}/icon/
    DESTINATION bin/icon
    FILES_MATCHING PATTERN "*.svg" PATTERN "*.png" PATTERN "*.ico"
)

# Create a simple run script for convenience
if(UNIX)
    file(WRITE ${CMAKE_BINARY_DIR}/bin/run_nexus.sh
        "#!/bin/bash\n"
        "cd \"$(dirname \"$0\")\"\n"
        "./Nexus \"$@\"\n"
    )
    file(COPY ${CMAKE_BINARY_DIR}/bin/run_nexus.sh 
         DESTINATION ${CMAKE_BINARY_DIR}/bin/
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif() 