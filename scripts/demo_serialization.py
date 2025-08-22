#!/usr/bin/env python3
"""
Cxml Serialization Demo Script
This script demonstrates the serialization capabilities of the Cxml project
"""

import os
import sys
import subprocess
from pathlib import Path


def run_demo():
    """Run the serialization demo"""
    print("=== Cxml Serialization Demo ===")
    
    # Get project root
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    
    # Check if build exists
    build_dir = project_root / "build"
    if not build_dir.exists():
        print("❌ Build directory not found. Please build the project first:")
        print("  python3 scripts/build.py")
        return
    
    # Check if examples exist
    examples_dir = project_root / "examples"
    if not examples_dir.exists():
        print("❌ Examples directory not found")
        return
    
    print("\n📁 Available example files:")
    for example_file in examples_dir.glob("*.xml"):
        print(f"  - {example_file.name}")
    
    print("\n🚀 Running serialization demo...")
    
    # TODO: This would require implementing a C++ interface or using the built executable
    # For now, we'll show what the demo would do
    
    print("\n📋 Demo Features:")
    print("1. XML → JSON conversion")
    print("2. XML → YAML conversion") 
    print("3. XML → CSV conversion (for table data)")
    print("4. JSON → XML conversion")
    print("5. YAML → XML conversion")
    print("6. Format validation")
    print("7. Pretty printing")
    
    print("\n💡 To use these features in the GUI:")
    print("1. Open an XML file in Cxml")
    print("2. Use the 'Export' menu to convert to different formats")
    print("3. Use the 'Import' menu to load different formats")
    
    print("\n🔧 To use programmatically:")
    print("```cpp")
    print("#include \"xml_serializer.h\"")
    print("")
    print("XmlSerializer serializer;")
    print("auto node = parser.parseFile(\"example.xml\");")
    print("")
    print("// Convert to JSON")
    print("std::string json = serializer.serializeToJson(node);")
    print("")
    print("// Convert to YAML")
    print("std::string yaml = serializer.serializeToYaml(node);")
    print("")
    print("// Convert from JSON")
    print("auto nodeFromJson = serializer.deserializeFromJson(jsonContent);")
    print("```")
    
    print("\n✅ Demo completed!")


def main():
    """Main function"""
    run_demo()


if __name__ == "__main__":
    main() 