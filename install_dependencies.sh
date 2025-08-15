#!/bin/bash

# Cxml Dependencies Installation Script
# This script installs the required dependencies for building Cxml

set -e  # Exit on any error

echo "=== Cxml Dependencies Installation ==="

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
else
    echo "Error: Cannot detect OS"
    exit 1
fi

echo "Detected OS: $OS"

# Install dependencies based on OS
if [[ "$OS" == *"Ubuntu"* ]] || [[ "$OS" == *"Debian"* ]]; then
    echo "Installing dependencies for Ubuntu/Debian..."
    sudo apt update
    sudo apt install -y build-essential cmake qt5-default libgtest-dev
    
elif [[ "$OS" == *"CentOS"* ]] || [[ "$OS" == *"Red Hat"* ]]; then
    echo "Installing dependencies for CentOS/RHEL..."
    sudo yum install -y gcc-c++ cmake qt5-devel gtest-devel
    
elif [[ "$OS" == *"Fedora"* ]]; then
    echo "Installing dependencies for Fedora..."
    sudo dnf install -y gcc-c++ cmake qt5-devel gtest-devel
    
elif [[ "$OS" == *"Arch"* ]]; then
    echo "Installing dependencies for Arch Linux..."
    sudo pacman -S --needed base-devel cmake qt5-base gtest
    
else
    echo "Unsupported OS: $OS"
    echo "Please install the following packages manually:"
    echo "  - build-essential (or equivalent)"
    echo "  - cmake (3.16+)"
    echo "  - qt5-default or qt5-devel"
    echo "  - libgtest-dev or gtest-devel"
    exit 1
fi

echo "=== Dependencies installed successfully! ==="
echo ""
echo "You can now build the project with:"
echo "  ./build.sh" 