#!/bin/bash

# Cxml Build Script
# This script builds the Cxml XML visualizer project

set -e  # Exit on any error

echo "=== Cxml Build Script ==="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the project root directory."
    exit 1
fi

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build the project
echo "Building the project..."
make -j$(nproc)

echo "=== Build completed successfully! ==="
echo ""
echo "To run the application:"
echo "  ./bin/Cxml"
echo ""
echo "To run tests:"
echo "  ./bin/Cxml_tests"
echo ""
echo "To run tests with CMake:"
echo "  make test" 