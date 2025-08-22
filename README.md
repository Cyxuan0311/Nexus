# Cxml - XML Visualizer

[![C++](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-5.12+-green.svg?style=flat&logo=qt)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-red.svg?style=flat&logo=cmake)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat)](LICENSE)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg?style=flat)](https://github.com/yourusername/Cxml)
[![Tests](https://img.shields.io/badge/Tests-Passing-brightgreen.svg?style=flat)](https://github.com/yourusername/Cxml)

A Qt-based XML file parser and visualizer with a VSCode-like interface, built with C++17, Qt5, and CMake.

## ✨ Features

- **Real-time XML Structure Visualization**: Parse XML files and display their hierarchical structure in a tree view
- **XML Text Editor with Syntax Highlighting**: Edit XML content directly with syntax highlighting and validation
- **Search and Replace**: Advanced search functionality with regex support and multi-scope search
- **Code Folding**: XML structure folding with visual indicators and keyboard shortcuts
- **Multi-format Serialization**: Support for XML, JSON, YAML, and CSV serialization/deserialization
- **VSCode-like Interface**: Dark theme with green accents, similar to Visual Studio Code
- **Node Details Display**: Click on any node to see detailed information including attributes, path, and depth
- **File Operations**: Open, parse, edit, and save XML files
- **Format Conversion**: Convert between different data formats (XML ↔ JSON, XML ↔ YAML, etc.)
- **Comprehensive Testing**: Full unit test coverage using Google Test
- **Automated Build System**: Python-based build and test automation scripts

## Screenshots

The application features a split-pane interface:
- **Left Panel**: File controls and XML structure tree view
- **Right Panel**: Detailed node information display

## 🛠️ Tech Stack

![C++](https://img.shields.io/badge/C++-17-blue?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![Qt](https://img.shields.io/badge/Qt-5.12+-green?style=for-the-badge&logo=qt&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.16+-red?style=for-the-badge&logo=cmake&logoColor=white)
![Google Test](https://img.shields.io/badge/Google_Test-1.11+-orange?style=for-the-badge&logo=google&logoColor=white)

## 📋 Requirements

- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **Qt5** (Core, Widgets)
- **CMake** 3.16+
- **Google Test** for unit testing

## 🚀 Building

### 📦 Prerequisites

Install the required dependencies:

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake qt5-default libgtest-dev
```

**CentOS/RHEL/Fedora:**
```bash
sudo yum install gcc-c++ cmake qt5-devel gtest-devel
# or for Fedora:
sudo dnf install gcc-c++ cmake qt5-devel gtest-devel
```

**macOS:**
```bash
brew install cmake qt5 gtest
```

**Or use our automated script:**
```bash
python3 ./scripts/install_dependencies.py
```

### 🔨 Build Instructions

1. **Clone the repository:**
```bash
git clone <repository-url>
cd Cxml
```

2. **Create build directory:**
```bash
mkdir build
cd build
```

3. **Configure with CMake:**
```bash
cmake ..
```

4. **Build the project:**
```bash
make -j$(nproc)
```

5. **Run tests:**
```bash
make test
# or run tests directly:
./bin/Cxml_tests
```

6. **Run the application:**
```bash
./bin/Cxml
```

**Or use our automated build script:**
```bash
python3 scripts/build.py
```

**Run tests with:**
```bash
python3 scripts/run_tests.py
```

## 🎯 Usage

1. **Open an XML file**: Click "Open XML File" or use File → Open
2. **Parse the XML**: Click "Parse XML" to generate the visual structure
3. **Explore the structure**: Click on nodes in the tree view to see details
4. **Edit XML content**: Use the built-in XML editor with syntax highlighting
5. **Search and replace**: Use Ctrl+F to find and replace text in XML
6. **Code folding**: Use Ctrl+Shift+[ to fold all, Ctrl+Shift+] to unfold all
7. **Save modifications**: Use File → Save As to save the parsed XML

## 📁 Project Structure

```
Cxml/
├── CMakeLists.txt          # Main CMake configuration
├── include/                # Header files
│   ├── xml_node.h         # XML node class definition
│   ├── xml_parser.h       # XML parser class definition
│   ├── xml_serializer.h   # XML serializer class definition
│   ├── xml_highlighter.h  # XML syntax highlighter
│   ├── search_dialog.h    # Search and replace dialog
│   ├── code_folding.h     # Code folding functionality
│   └── main_window.h      # Main window class definition
├── src/                   # Source files
│   ├── xml_node.cpp       # XML node implementation
│   ├── xml_parser.cpp     # XML parser implementation
│   ├── xml_serializer.cpp # XML serializer implementation
│   ├── xml_highlighter.cpp # XML syntax highlighter implementation
│   ├── search_dialog.cpp  # Search and replace dialog implementation
│   ├── code_folding.cpp   # Code folding implementation
│   ├── main_window.cpp    # Main window implementation
│   └── main.cpp          # Application entry point
├── test/                  # Test files
│   ├── main.cpp          # Test entry point
│   ├── xml_parser_test.cpp # XML parser unit tests
│   ├── xml_serializer_test.cpp # XML serializer unit tests
│   ├── search_test.cpp    # Search functionality tests
│   └── code_folding_test.cpp # Code folding tests
├── scripts/               # Build and utility scripts
│   ├── build.py          # Automated build script
│   ├── install_dependencies.py # Dependency installation script
│   └── run_tests.py      # Test runner script
└── examples/              # Sample XML files
    ├── sample.xml        # Complex XML example
    └── simple.xml        # Simple XML example
```

## 🧪 Testing

The project includes comprehensive unit tests covering:

- XML parsing functionality
- Node creation and manipulation
- Attribute handling
- Nested element parsing
- Error handling
- XML entity escaping

Run tests with:
```bash
cd build
make test
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built with Qt5 for the user interface
- Uses Google Test for unit testing
- Inspired by Visual Studio Code's interface design 