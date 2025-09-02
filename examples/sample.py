#!/usr/bin/env python3
"""
Sample Python file to demonstrate parsing capabilities
This module contains various Python constructs for testing the parser.
"""

import math
import os
from typing import List, Dict, Optional
from dataclasses import dataclass


@dataclass
class Point:
    """Represents a 2D point."""
    x: float
    y: float
    
    def distance_to_origin(self) -> float:
        """Calculate distance from point to origin."""
        return math.sqrt(self.x ** 2 + self.y ** 2)
    
    def distance_to(self, other: 'Point') -> float:
        """Calculate distance to another point."""
        dx = self.x - other.x
        dy = self.y - other.y
        return math.sqrt(dx ** 2 + dy ** 2)


class Calculator:
    """A simple calculator class with basic operations."""
    
    def __init__(self):
        """Initialize calculator with empty history."""
        self.history: List[float] = []
        self._precision = 2
    
    def add(self, a: float, b: float) -> float:
        """Add two numbers."""
        result = a + b
        self._record_result(result)
        return result
    
    def subtract(self, a: float, b: float) -> float:
        """Subtract b from a."""
        result = a - b
        self._record_result(result)
        return result
    
    def multiply(self, a: float, b: float) -> float:
        """Multiply two numbers."""
        result = a * b
        self._record_result(result)
        return result
    
    def divide(self, a: float, b: float) -> float:
        """Divide a by b."""
        if b == 0:
            raise ValueError("Division by zero is not allowed")
        result = a / b
        self._record_result(result)
        return result
    
    def power(self, base: float, exponent: float) -> float:
        """Calculate base raised to the power of exponent."""
        result = math.pow(base, exponent)
        self._record_result(result)
        return result
    
    def _record_result(self, result: float) -> None:
        """Record calculation result in history."""
        rounded_result = round(result, self._precision)
        self.history.append(rounded_result)
    
    def get_history(self) -> List[float]:
        """Get calculation history."""
        return self.history.copy()
    
    def clear_history(self) -> None:
        """Clear calculation history."""
        self.history.clear()
    
    @staticmethod
    def factorial(n: int) -> int:
        """Calculate factorial of n."""
        if n < 0:
            raise ValueError("Factorial is not defined for negative numbers")
        if n == 0 or n == 1:
            return 1
        return n * Calculator.factorial(n - 1)
    
    @classmethod
    def create_with_precision(cls, precision: int) -> 'Calculator':
        """Create calculator with specified precision."""
        calc = cls()
        calc._precision = precision
        return calc


class ScientificCalculator(Calculator):
    """Extended calculator with scientific functions."""
    
    def __init__(self):
        """Initialize scientific calculator."""
        super().__init__()
        self._angle_mode = 'radians'
    
    def sine(self, angle: float) -> float:
        """Calculate sine of angle."""
        if self._angle_mode == 'degrees':
            angle = math.radians(angle)
        result = math.sin(angle)
        self._record_result(result)
        return result
    
    def cosine(self, angle: float) -> float:
        """Calculate cosine of angle."""
        if self._angle_mode == 'degrees':
            angle = math.radians(angle)
        result = math.cos(angle)
        self._record_result(result)
        return result
    
    def tangent(self, angle: float) -> float:
        """Calculate tangent of angle."""
        if self._angle_mode == 'degrees':
            angle = math.radians(angle)
        result = math.tan(angle)
        self._record_result(result)
        return result
    
    def logarithm(self, value: float, base: Optional[float] = None) -> float:
        """Calculate logarithm of value."""
        if value <= 0:
            raise ValueError("Logarithm is not defined for non-positive numbers")
        
        if base is None:
            result = math.log(value)  # Natural logarithm
        else:
            result = math.log(value, base)
        
        self._record_result(result)
        return result
    
    def set_angle_mode(self, mode: str) -> None:
        """Set angle mode to 'radians' or 'degrees'."""
        if mode not in ['radians', 'degrees']:
            raise ValueError("Angle mode must be 'radians' or 'degrees'")
        self._angle_mode = mode


# Utility functions
def print_result(operation: str, result: float) -> None:
    """Print calculation result with formatting."""
    print(f"{operation} = {result:.4f}")


def calculate_statistics(values: List[float]) -> Dict[str, float]:
    """Calculate basic statistics for a list of values."""
    if not values:
        return {'count': 0, 'sum': 0, 'mean': 0, 'min': 0, 'max': 0}
    
    count = len(values)
    total = sum(values)
    mean = total / count
    minimum = min(values)
    maximum = max(values)
    
    return {
        'count': count,
        'sum': total,
        'mean': mean,
        'min': minimum,
        'max': maximum
    }


async def async_calculation(calc: Calculator, operations: List[tuple]) -> List[float]:
    """Perform multiple calculations asynchronously."""
    results = []
    
    for operation, a, b in operations:
        if operation == 'add':
            result = calc.add(a, b)
        elif operation == 'subtract':
            result = calc.subtract(a, b)
        elif operation == 'multiply':
            result = calc.multiply(a, b)
        elif operation == 'divide':
            result = calc.divide(a, b)
        else:
            raise ValueError(f"Unknown operation: {operation}")
        
        results.append(result)
        print_result(f"{operation}({a}, {b})", result)
    
    return results


def main():
    """Main function demonstrating calculator usage."""
    # Create calculators
    basic_calc = Calculator()
    sci_calc = ScientificCalculator()
    
    try:
        # Basic calculations
        result1 = basic_calc.add(10.5, 5.3)
        print_result("10.5 + 5.3", result1)
        
        result2 = basic_calc.multiply(4.0, 2.5)
        print_result("4.0 * 2.5", result2)
        
        result3 = basic_calc.divide(20.0, 4.0)
        print_result("20.0 / 4.0", result3)
        
        # Scientific calculations
        sci_calc.set_angle_mode('degrees')
        sin_result = sci_calc.sine(30)
        print_result("sin(30°)", sin_result)
        
        log_result = sci_calc.logarithm(math.e)
        print_result("ln(e)", log_result)
        
        # Static method usage
        factorial_result = Calculator.factorial(5)
        print_result("5!", factorial_result)
        
        # Statistics
        history = basic_calc.get_history()
        stats = calculate_statistics(history)
        print(f"Statistics: {stats}")
        
        # Point calculations
        p1 = Point(3.0, 4.0)
        p2 = Point(0.0, 0.0)
        
        distance1 = p1.distance_to_origin()
        print_result("Distance to origin", distance1)
        
        distance2 = p1.distance_to(p2)
        print_result("Distance between points", distance2)
        
    except ValueError as e:
        print(f"Error: {e}")
        return 1
    except Exception as e:
        print(f"Unexpected error: {e}")
        return 1
    
    return 0


if __name__ == "__main__":
    exit_code = main()
    exit(exit_code) 