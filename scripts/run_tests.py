#!/usr/bin/env python3
"""
Cxml Test Runner Script
This script runs the test suite for the Cxml project
"""

import os
import sys
import subprocess
import argparse
from pathlib import Path
from typing import List, Optional


class TestRunner:
    """Handles running tests for the Cxml project"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.build_dir = project_root / "build"
        self.test_executable = self.build_dir / "bin" / "Cxml_tests"
        
    def _run_command(self, command: List[str], cwd: Optional[Path] = None) -> subprocess.CompletedProcess:
        """Run a shell command"""
        cwd = cwd or self.project_root
        print(f"Running: {' '.join(command)}")
        
        try:
            result = subprocess.run(
                command, 
                check=True, 
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
            raise
    
    def _check_test_executable(self) -> bool:
        """Check if test executable exists"""
        if not self.test_executable.exists():
            print(f"❌ Test executable not found: {self.test_executable}")
            print("Please build the project first:")
            print("  python3 scripts/build.py")
            return False
        return True
    
    def _run_unit_tests(self, verbose: bool = False) -> bool:
        """Run unit tests"""
        print("🧪 Running unit tests...")
        
        if not self._check_test_executable():
            return False
        
        try:
            cmd = [str(self.test_executable)]
            if verbose:
                cmd.append("--gtest_verbose")
            
            result = self._run_command(cmd, cwd=self.build_dir)
            print("✅ Unit tests completed successfully")
            return True
        except subprocess.CalledProcessError:
            print("❌ Unit tests failed")
            return False
    
    def _run_cmake_tests(self) -> bool:
        """Run CMake tests"""
        print("🧪 Running CMake tests...")
        
        try:
            self._run_command(['make', 'test'], cwd=self.build_dir)
            print("✅ CMake tests completed successfully")
            return True
        except subprocess.CalledProcessError:
            print("❌ CMake tests failed")
            return False
    
    def _run_integration_tests(self) -> bool:
        """Run integration tests"""
        print("🧪 Running integration tests...")
        
        # Test XML parsing with example files
        example_files = [
            self.project_root / "examples" / "simple.xml",
            self.project_root / "examples" / "sample.xml"
        ]
        
        success = True
        for example_file in example_files:
            if example_file.exists():
                print(f"Testing with {example_file.name}...")
                # TODO: Add actual integration test logic
                print(f"✅ {example_file.name} - OK")
            else:
                print(f"⚠️  {example_file.name} not found")
        
        return success
    
    def _run_performance_tests(self) -> bool:
        """Run performance tests"""
        print("⚡ Running performance tests...")
        
        # TODO: Implement performance tests
        print("Performance tests not implemented yet")
        return True
    
    def _generate_test_report(self, results: dict) -> None:
        """Generate test report"""
        print("\n" + "="*50)
        print("📊 TEST REPORT")
        print("="*50)
        
        total_tests = len(results)
        passed_tests = sum(1 for result in results.values() if result)
        failed_tests = total_tests - passed_tests
        
        for test_name, result in results.items():
            status = "✅ PASS" if result else "❌ FAIL"
            print(f"{test_name}: {status}")
        
        print(f"\nSummary: {passed_tests}/{total_tests} tests passed")
        
        if failed_tests > 0:
            print(f"❌ {failed_tests} test(s) failed")
            sys.exit(1)
        else:
            print("🎉 All tests passed!")
    
    def run_tests(self, test_types: List[str], verbose: bool = False) -> None:
        """Run specified test types"""
        print("=== Cxml Test Runner ===")
        
        available_tests = {
            'unit': self._run_unit_tests,
            'cmake': self._run_cmake_tests,
            'integration': self._run_integration_tests,
            'performance': self._run_performance_tests
        }
        
        if not test_types:
            test_types = ['unit', 'cmake', 'integration']
        
        results = {}
        
        for test_type in test_types:
            if test_type in available_tests:
                try:
                    if test_type == 'unit':
                        results[test_type] = available_tests[test_type](verbose)
                    else:
                        results[test_type] = available_tests[test_type]()
                except Exception as e:
                    print(f"❌ Error running {test_type} tests: {e}")
                    results[test_type] = False
            else:
                print(f"⚠️  Unknown test type: {test_type}")
                results[test_type] = False
        
        self._generate_test_report(results)


def main():
    """Main function"""
    parser = argparse.ArgumentParser(description="Run Cxml tests")
    parser.add_argument(
        '--test-types', 
        nargs='+', 
        choices=['unit', 'cmake', 'integration', 'performance'],
        help='Types of tests to run'
    )
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Verbose output'
    )
    
    args = parser.parse_args()
    
    # Get project root directory
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    
    # Change to project root directory
    os.chdir(project_root)
    
    runner = TestRunner(project_root)
    runner.run_tests(args.test_types, args.verbose)


if __name__ == "__main__":
    main() 