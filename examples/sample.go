package main

import (
	"fmt"
	"math"
	"sort"
	"strconv"
)

// Point represents a 2D point in space
type Point struct {
	X float64 `json:"x"`
	Y float64 `json:"y"`
}

// DistanceToOrigin calculates the distance from point to origin
func (p Point) DistanceToOrigin() float64 {
	return math.Sqrt(p.X*p.X + p.Y*p.Y)
}

// DistanceTo calculates the distance to another point
func (p Point) DistanceTo(other Point) float64 {
	dx := p.X - other.X
	dy := p.Y - other.Y
	return math.Sqrt(dx*dx + dy*dy)
}

// String returns string representation of the point
func (p Point) String() string {
	return fmt.Sprintf("Point(%.2f, %.2f)", p.X, p.Y)
}

// Calculator provides basic arithmetic operations
type Calculator struct {
	history []float64
	name    string
}

// NewCalculator creates a new calculator instance
func NewCalculator(name string) *Calculator {
	return &Calculator{
		history: make([]float64, 0),
		name:    name,
	}
}

// Add performs addition and records the result
func (c *Calculator) Add(a, b float64) float64 {
	result := a + b
	c.recordResult(result)
	return result
}

// Subtract performs subtraction and records the result
func (c *Calculator) Subtract(a, b float64) float64 {
	result := a - b
	c.recordResult(result)
	return result
}

// Multiply performs multiplication and records the result
func (c *Calculator) Multiply(a, b float64) float64 {
	result := a * b
	c.recordResult(result)
	return result
}

// Divide performs division and records the result
func (c *Calculator) Divide(a, b float64) (float64, error) {
	if b == 0 {
		return 0, fmt.Errorf("division by zero")
	}
	result := a / b
	c.recordResult(result)
	return result, nil
}

// Power calculates a raised to the power of b
func (c *Calculator) Power(a, b float64) float64 {
	result := math.Pow(a, b)
	c.recordResult(result)
	return result
}

// recordResult adds a result to the calculation history
func (c *Calculator) recordResult(result float64) {
	c.history = append(c.history, result)
}

// GetHistory returns a copy of the calculation history
func (c *Calculator) GetHistory() []float64 {
	historyCopy := make([]float64, len(c.history))
	copy(historyCopy, c.history)
	return historyCopy
}

// ClearHistory clears the calculation history
func (c *Calculator) ClearHistory() {
	c.history = c.history[:0]
}

// GetLastResult returns the last calculation result
func (c *Calculator) GetLastResult() (float64, bool) {
	if len(c.history) == 0 {
		return 0, false
	}
	return c.history[len(c.history)-1], true
}

// StatCalculator extends Calculator with statistical functions
type StatCalculator struct {
	*Calculator
	precision int
}

// NewStatCalculator creates a new statistical calculator
func NewStatCalculator(name string, precision int) *StatCalculator {
	return &StatCalculator{
		Calculator: NewCalculator(name),
		precision:  precision,
	}
}

// Mean calculates the mean of given values
func (sc *StatCalculator) Mean(values []float64) float64 {
	if len(values) == 0 {
		return 0
	}

	total := sum(values)
	result := total / float64(len(values))
	sc.recordResult(result)
	return result
}

// Median calculates the median of given values
func (sc *StatCalculator) Median(values []float64) float64 {
	if len(values) == 0 {
		return 0
	}

	sorted := make([]float64, len(values))
	copy(sorted, values)
	sort.Float64s(sorted)

	n := len(sorted)
	var result float64
	if n%2 == 0 {
		result = (sorted[n/2-1] + sorted[n/2]) / 2
	} else {
		result = sorted[n/2]
	}

	sc.recordResult(result)
	return result
}

// StandardDeviation calculates the standard deviation
func (sc *StatCalculator) StandardDeviation(values []float64) float64 {
	if len(values) <= 1 {
		return 0
	}

	mean := sc.Mean(values)
	sumSquaredDiffs := 0.0

	for _, value := range values {
		diff := value - mean
		sumSquaredDiffs += diff * diff
	}

	variance := sumSquaredDiffs / float64(len(values)-1)
	result := math.Sqrt(variance)
	sc.recordResult(result)
	return result
}

// Shape interface defines methods for geometric shapes
type Shape interface {
	Area() float64
	Perimeter() float64
	String() string
}

// Circle implements the Shape interface
type Circle struct {
	Radius float64
	Center Point
}

// Area calculates the area of the circle
func (c Circle) Area() float64 {
	return math.Pi * c.Radius * c.Radius
}

// Perimeter calculates the perimeter of the circle
func (c Circle) Perimeter() float64 {
	return 2 * math.Pi * c.Radius
}

// String returns string representation of the circle
func (c Circle) String() string {
	return fmt.Sprintf("Circle(radius=%.2f, center=%s)", c.Radius, c.Center.String())
}

// Rectangle implements the Shape interface
type Rectangle struct {
	Width  float64
	Height float64
	Origin Point
}

// Area calculates the area of the rectangle
func (r Rectangle) Area() float64 {
	return r.Width * r.Height
}

// Perimeter calculates the perimeter of the rectangle
func (r Rectangle) Perimeter() float64 {
	return 2 * (r.Width + r.Height)
}

// String returns string representation of the rectangle
func (r Rectangle) String() string {
	return fmt.Sprintf("Rectangle(%.2fx%.2f at %s)", r.Width, r.Height, r.Origin.String())
}

// Utility functions

// sum calculates the sum of a slice of float64 values
func sum(values []float64) float64 {
	total := 0.0
	for _, value := range values {
		total += value
	}
	return total
}

// max returns the maximum value from a slice of float64 values
func max(values []float64) float64 {
	if len(values) == 0 {
		return 0
	}

	maxValue := values[0]
	for _, value := range values[1:] {
		if value > maxValue {
			maxValue = value
		}
	}
	return maxValue
}

// min returns the minimum value from a slice of float64 values
func min(values []float64) float64 {
	if len(values) == 0 {
		return 0
	}

	minValue := values[0]
	for _, value := range values[1:] {
		if value < minValue {
			minValue = value
		}
	}
	return minValue
}

// formatFloat formats a float64 value to a string with specified precision
func formatFloat(value float64, precision int) string {
	return strconv.FormatFloat(value, 'f', precision, 64)
}

// printResult prints a calculation result with formatting
func printResult(operation string, result float64) {
	fmt.Printf("%s = %.4f\n", operation, result)
}

// printShapeInfo prints information about a shape
func printShapeInfo(shape Shape) {
	fmt.Printf("Shape: %s\n", shape.String())
	fmt.Printf("Area: %.2f\n", shape.Area())
	fmt.Printf("Perimeter: %.2f\n", shape.Perimeter())
	fmt.Println("---")
}

// demonstrateCalculator shows basic calculator usage
func demonstrateCalculator() {
	fmt.Println("=== Calculator Demo ===")

	calc := NewCalculator("Basic Calculator")

	// Basic operations
	result1 := calc.Add(10.5, 5.3)
	printResult("10.5 + 5.3", result1)

	result2 := calc.Multiply(4.0, 2.5)
	printResult("4.0 * 2.5", result2)

	result3, err := calc.Divide(20.0, 4.0)
	if err != nil {
		fmt.Printf("Error: %v\n", err)
	} else {
		printResult("20.0 / 4.0", result3)
	}

	// Power operation
	result4 := calc.Power(2.0, 8.0)
	printResult("2^8", result4)

	// Show history
	history := calc.GetHistory()
	fmt.Printf("Calculation history: %v\n", history)

	// Statistics
	statCalc := NewStatCalculator("Stat Calculator", 2)
	mean := statCalc.Mean(history)
	printResult("Mean of history", mean)

	median := statCalc.Median(history)
	printResult("Median of history", median)

	fmt.Println()
}

// demonstrateShapes shows geometry functionality
func demonstrateShapes() {
	fmt.Println("=== Shapes Demo ===")

	// Create shapes
	circle := Circle{
		Radius: 5.0,
		Center: Point{X: 0, Y: 0},
	}

	rectangle := Rectangle{
		Width:  4.0,
		Height: 6.0,
		Origin: Point{X: 1, Y: 1},
	}

	// Store shapes in slice
	shapes := []Shape{circle, rectangle}

	// Print information for each shape
	for _, shape := range shapes {
		printShapeInfo(shape)
	}

	// Point operations
	p1 := Point{X: 3.0, Y: 4.0}
	p2 := Point{X: 0.0, Y: 0.0}

	distance1 := p1.DistanceToOrigin()
	printResult("Distance to origin", distance1)

	distance2 := p1.DistanceTo(p2)
	printResult("Distance between points", distance2)

	fmt.Println()
}

// main function demonstrates the usage of all components
func main() {
	fmt.Println("Go Calculator & Geometry Demo")
	fmt.Println("=============================")

	demonstrateCalculator()
	demonstrateShapes()

	// Advanced usage example
	fmt.Println("=== Advanced Usage ===")

	// Create data for statistical analysis
	data := []float64{1.2, 3.4, 5.6, 7.8, 9.1, 2.3, 4.5, 6.7, 8.9, 1.1}

	statCalc := NewStatCalculator("Advanced Stats", 3)

	mean := statCalc.Mean(data)
	median := statCalc.Median(data)
	stdDev := statCalc.StandardDeviation(data)

	fmt.Printf("Data: %v\n", data)
	printResult("Mean", mean)
	printResult("Median", median)
	printResult("Standard Deviation", stdDev)

	maxVal := max(data)
	minVal := min(data)
	totalSum := sum(data)

	printResult("Maximum", maxVal)
	printResult("Minimum", minVal)
	printResult("Sum", totalSum)

	// Format and display
	formattedMean := formatFloat(mean, 2)
	fmt.Printf("Formatted mean: %s\n", formattedMean)

	fmt.Println("\nDemo completed successfully!")
}
