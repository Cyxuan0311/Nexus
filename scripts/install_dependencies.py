#!/usr/bin/env python3
"""
Cxml Dependencies Installation Script
This script installs the required dependencies for building Cxml
"""

import os
import sys
import subprocess
import platform
from typing import List, Dict


class DependencyInstaller:
    """Handles dependency installation for different operating systems"""
    
    def __init__(self):
        self.os_info = self._get_os_info()
        self.package_managers = {
            'ubuntu': self._install_ubuntu_debian,
            'debian': self._install_ubuntu_debian,
            'centos': self._install_centos_rhel,
            'redhat': self._install_centos_rhel,
            'fedora': self._install_fedora,
            'arch': self._install_arch,
            'darwin': self._install_macos
        }
    
    def _get_os_info(self) -> Dict[str, str]:
        """Get operating system information"""
        system = platform.system().lower()
        if system == 'linux':
            # Try to read /etc/os-release
            try:
                with open('/etc/os-release', 'r') as f:
                    lines = f.readlines()
                    os_info = {}
                    for line in lines:
                        if '=' in line:
                            key, value = line.strip().split('=', 1)
                            os_info[key.lower()] = value.strip('"')
                    return os_info
            except FileNotFoundError:
                return {'name': 'unknown'}
        elif system == 'darwin':
            return {'name': 'macos'}
        else:
            return {'name': system}
    
    def _run_command(self, command: List[str], check: bool = True) -> subprocess.CompletedProcess:
        """Run a shell command"""
        print(f"Running: {' '.join(command)}")
        try:
            result = subprocess.run(command, check=check, capture_output=True, text=True)
            if result.stdout:
                print(result.stdout)
            return result
        except subprocess.CalledProcessError as e:
            print(f"Error running command: {' '.join(command)}")
            print(f"Error: {e.stderr}")
            if check:
                raise
            return e
    
    def _install_ubuntu_debian(self):
        """Install dependencies for Ubuntu/Debian"""
        print("Installing dependencies for Ubuntu/Debian...")
        commands = [
            ['sudo', 'apt', 'update'],
            ['sudo', 'apt', 'install', '-y', 'build-essential', 'cmake', 'qt5-default', 'libgtest-dev']
        ]
        
        for cmd in commands:
            self._run_command(cmd)
    
    def _install_centos_rhel(self):
        """Install dependencies for CentOS/RHEL"""
        print("Installing dependencies for CentOS/RHEL...")
        commands = [
            ['sudo', 'yum', 'install', '-y', 'gcc-c++', 'cmake', 'qt5-devel', 'gtest-devel']
        ]
        
        for cmd in commands:
            self._run_command(cmd)
    
    def _install_fedora(self):
        """Install dependencies for Fedora"""
        print("Installing dependencies for Fedora...")
        commands = [
            ['sudo', 'dnf', 'install', '-y', 'gcc-c++', 'cmake', 'qt5-devel', 'gtest-devel']
        ]
        
        for cmd in commands:
            self._run_command(cmd)
    
    def _install_arch(self):
        """Install dependencies for Arch Linux"""
        print("Installing dependencies for Arch Linux...")
        commands = [
            ['sudo', 'pacman', '-S', '--needed', 'base-devel', 'cmake', 'qt5-base', 'gtest']
        ]
        
        for cmd in commands:
            self._run_command(cmd)
    
    def _install_macos(self):
        """Install dependencies for macOS"""
        print("Installing dependencies for macOS...")
        commands = [
            ['brew', 'install', 'cmake', 'qt5', 'gtest']
        ]
        
        for cmd in commands:
            self._run_command(cmd)
    
    def install_dependencies(self):
        """Install dependencies based on detected OS"""
        os_name = self.os_info.get('name', '').lower()
        print(f"Detected OS: {os_name}")
        
        # Map OS names to package manager functions
        os_mapping = {
            'ubuntu': 'ubuntu',
            'debian': 'debian',
            'centos': 'centos',
            'red hat enterprise linux': 'redhat',
            'fedora': 'fedora',
            'arch': 'arch',
            'macos': 'darwin'
        }
        
        # Find matching OS
        matched_os = None
        for key, value in os_mapping.items():
            if key in os_name:
                matched_os = value
                break
        
        if matched_os and matched_os in self.package_managers:
            try:
                self.package_managers[matched_os]()
                print("✅ Dependencies installed successfully!")
                print("\nYou can now build the project with:")
                print("  python3 scripts/build.py")
            except Exception as e:
                print(f"❌ Failed to install dependencies: {e}")
                sys.exit(1)
        else:
            print(f"❌ Unsupported OS: {os_name}")
            print("Please install the following packages manually:")
            print("  - build-essential (or equivalent)")
            print("  - cmake (3.16+)")
            print("  - qt5-default or qt5-devel")
            print("  - libgtest-dev or gtest-devel")
            sys.exit(1)


def main():
    """Main function"""
    print("=== Cxml Dependencies Installation ===")
    
    # Check if running as root (not recommended for security)
    if os.geteuid() == 0:
        print("⚠️  Warning: Running as root is not recommended for security reasons.")
        response = input("Continue anyway? (y/N): ")
        if response.lower() != 'y':
            print("Installation cancelled.")
            sys.exit(0)
    
    installer = DependencyInstaller()
    installer.install_dependencies()


if __name__ == "__main__":
    main() 