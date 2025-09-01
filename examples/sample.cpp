#include <iostream>
#include <vector>
#include <string>

// Sample C++ file to demonstrate parsing capabilities

class Calculator {
private:
    std::vector<double> history_;
    
public:
    Calculator() = default;
    
    // Basic arithmetic operations
    double add(double a, double b) {
        double result = a + b;
        history_.push_back(result);
        return result;
    }
    
    double subtract(double a, double b) {
        double result = a - b;
        history_.push_back(result);
        return result;
    }
    
    double multiply(double a, double b) {
        double result = a * b;
        history_.push_back(result);
        return result;
    }
    
    double divide(double a, double b) {
        if (b == 0) {
            throw std::invalid_argument("Division by zero");
        }
        double result = a / b;
        history_.push_back(result);
        return result;
    }
    
    // Utility functions
    void clearHistory() {
        history_.clear();
    }
    
    std::vector<double> getHistory() const {
        return history_;
    }
    
    double getLastResult() const {
        if (history_.empty()) {
            return 0.0;
        }
        return history_.back();
    }
};

// Free functions
void printResult(const std::string& operation, double result) {
    std::cout << operation << " = " << result << std::endl;
}

double calculateAverage(const std::vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (double value : values) {
        sum += value;
    }
    
    return sum / values.size();
}

// Template function example
template<typename T>
T findMaximum(const std::vector<T>& values) {
    if (values.empty()) {
        throw std::runtime_error("Empty vector");
    }
    
    T max = values[0];
    for (const T& value : values) {
        if (value > max) {
            max = value;
        }
    }
    
    return max;
}

// Main function demonstrating usage
int main() {
    Calculator calc;
    
    try {
        // Perform calculations
        double result1 = calc.add(10.5, 5.3);
        printResult("10.5 + 5.3", result1);
        
        double result2 = calc.multiply(4.0, 2.5);
        printResult("4.0 * 2.5", result2);
        
        double result3 = calc.divide(20.0, 4.0);
        printResult("20.0 / 4.0", result3);
        
        // Get history and calculate statistics
        std::vector<double> history = calc.getHistory();
        double average = calculateAverage(history);
        printResult("Average", average);
        
        double maxValue = findMaximum(history);
        printResult("Maximum", maxValue);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 