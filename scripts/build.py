#!/usr/bin/env python3
"""
Cxml Build Script
This script builds the Cxml XML visualizer project
"""

import os
import sys
import subprocess
import multiprocessing
from pathlib import Path
from typing import List, Optional


class ProjectBuilder:
    """Handles building the Cxml project"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.build_dir = project_root / "build"
        self.bin_dir = self.build_dir / "bin"
        
    def _run_command(self, command: List[str], cwd: Optional[Path] = None, 
                    check: bool = True) -> subprocess.CompletedProcess:
        """Run a shell command"""
        cwd = cwd or self.project_root
        print(f"Running: {' '.join(command)}")
        print(f"Working directory: {cwd}")
        
        try:
            result = subprocess.run(
                command, 
                check=check, 
                cwd=cwd,
                capture_output=True, 
                text=True
            )
            if result.stdout:
                print(result.stdout)
            return result
        except subprocess.CalledProcessError as e:
            print(f"❌ Error running command: {' '.join(command)}")
            if e.stderr:
                print(f"Error output: {e.stderr}")
            if check:
                raise
            return e
    
    def _check_prerequisites(self) -> bool:
        """Check if required tools are available"""
        print("🔍 Checking prerequisites...")
        
        required_tools = ['cmake', 'make', 'g++']
        missing_tools = []
        
        for tool in required_tools:
            try:
                result = subprocess.run([tool, '--version'], 
                                      capture_output=True, text=True, check=True)
                print(f"✅ {tool}: Found")
            except (subprocess.CalledProcessError, FileNotFoundError):
                print(f"❌ {tool}: Not found")
                missing_tools.append(tool)
        
        if missing_tools:
            print(f"\n❌ Missing required tools: {', '.join(missing_tools)}")
            print("Please install dependencies first:")
            print("  python3 scripts/install_dependencies.py")
            return False
        
        return True
    
    def _check_cmake_lists(self) -> bool:
        """Check if CMakeLists.txt exists"""
        cmake_file = self.project_root / "CMakeLists.txt"
        if not cmake_file.exists():
            print(f"❌ CMakeLists.txt not found at {cmake_file}")
            print("Please run this script from the project root directory.")
            return False
        print("✅ CMakeLists.txt found")
        return True
    
    def _create_build_directory(self):
        """Create build directory if it doesn't exist"""
        print(f"📁 Creating build directory: {self.build_dir}")
        self.build_dir.mkdir(exist_ok=True)
    
    def _configure_cmake(self):
        """Configure the project with CMake"""
        print("⚙️  Configuring with CMake...")
        self._run_command(['cmake', '..'], cwd=self.build_dir)
    
    def _build_project(self):
        """Build the project using make"""
        print("🔨 Building the project...")
        
        # Get number of CPU cores for parallel compilation
        cpu_count = multiprocessing.cpu_count()
        print(f"Using {cpu_count} CPU cores for parallel compilation")
        
        self._run_command(['make', '-j', str(cpu_count)], cwd=self.build_dir)
    
    def _run_tests(self):
        """Run the test suite"""
        print("🧪 Running tests...")
        try:
            self._run_command(['make', 'test'], cwd=self.build_dir)
            print("✅ Tests completed successfully")
        except subprocess.CalledProcessError:
            print("⚠️  Some tests failed, but build completed")
    
    def _check_build_outputs(self):
        """Check if build outputs were created"""
        print("🔍 Checking build outputs...")
        
        expected_files = [
            self.bin_dir / "Cxml",
            self.bin_dir / "Cxml_tests"
        ]
        
        for file_path in expected_files:
            if file_path.exists():
                print(f"✅ {file_path.name}: Created")
            else:
                print(f"⚠️  {file_path.name}: Not found")
    
    def build(self):
        """Main build process"""
        print("=== Cxml Build Script ===")
        
        # Check prerequisites
        if not self._check_prerequisites():
            sys.exit(1)
        
        # Check CMakeLists.txt
        if not self._check_cmake_lists():
            sys.exit(1)
        
        try:
            # Create build directory
            self._create_build_directory()
            
            # Configure with CMake
            self._configure_cmake()
            
            # Build the project
            self._build_project()
            
            # Run tests
            self._run_tests()
            
            # Check outputs
            self._check_build_outputs()
            
            print("\n=== Build completed successfully! ===")
            print(f"\n📁 Build outputs located in: {self.bin_dir}")
            print("\n🚀 To run the application:")
            print(f"  {self.bin_dir}/Cxml")
            print("\n🧪 To run tests:")
            print(f"  {self.bin_dir}/Cxml_tests")
            print("\n📋 To run tests with CMake:")
            print("  cd build && make test")
            
        except subprocess.CalledProcessError as e:
            print(f"\n❌ Build failed with error code: {e.returncode}")
            sys.exit(1)
        except Exception as e:
            print(f"\n❌ Unexpected error: {e}")
            sys.exit(1)


def main():
    """Main function"""
    # Get project root directory
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    
    # Change to project root directory
    os.chdir(project_root)
    
    builder = ProjectBuilder(project_root)
    builder.build()


if __name__ == "__main__":
    main() 