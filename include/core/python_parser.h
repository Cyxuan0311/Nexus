#ifndef PYTHON_PARSER_H
#define PYTHON_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <QRegExp>
#include <QStringList>

struct PythonParameter {
    std::string name;
    std::string type;  // Python typing annotation
    std::string defaultValue;
    bool isOptional;
};

struct PythonFunction {
    std::string name;
    std::string returnType;  // Python typing annotation
    std::vector<PythonParameter> parameters;
    std::vector<std::string> calledFunctions;
    int lineNumber;
    std::string className;  // If it's a method
    bool isAsync;
    bool isStaticMethod;
    bool isClassMethod;
    bool isPrivate;  // starts with underscore
    std::string decorator;
    std::string docstring;
};

struct PythonClass {
    std::string name;
    std::vector<std::string> baseClasses;
    std::vector<PythonFunction> methods;
    std::vector<std::string> attributes;
    int lineNumber;
    std::string docstring;
};

class PythonParser {
public:
    PythonParser();
    ~PythonParser();
    
    // Parse Python file
    bool parseFile(const std::string& content);
    
    // Get parsing results
    const std::vector<PythonFunction>& getFunctions() const { return functions_; }
    const std::vector<PythonClass>& getClasses() const { return classes_; }
    const std::map<std::string, std::vector<std::string>>& getFunctionCalls() const { return functionCalls_; }
    
    // Helper functions
    std::string getFunctionSignature(const PythonFunction& func) const;
    std::vector<std::string> getCalledFunctions(const std::string& functionName) const;
    std::vector<std::string> getCallingFunctions(const std::string& functionName) const;
    
    // Clear parsing results
    void clear();

private:
    std::vector<PythonFunction> functions_;
    std::vector<PythonClass> classes_;
    std::map<std::string, std::vector<std::string>> functionCalls_;  // function -> list of called functions
    
    // Parsing helper functions
    void parseFunctions(const std::string& content);
    void parseClasses(const std::string& content);
    void parseFunctionCalls(const std::string& content);
    
    PythonFunction parseFunctionDefinition(const std::vector<std::string>& lines, int startLine);
    std::vector<PythonParameter> parseParameters(const std::string& paramStr);
    std::string removeComments(const std::string& content);
    std::string normalizeWhitespace(const std::string& text);
    std::vector<std::string> splitLines(const std::string& content);
    std::string extractDocstring(const std::vector<std::string>& lines, int startLine);
    int getIndentLevel(const std::string& line);
    bool isBlankOrComment(const std::string& line);
    
    // Regular expression patterns
    QRegExp functionPattern_;
    QRegExp asyncFunctionPattern_;
    QRegExp classPattern_;
    QRegExp functionCallPattern_;
    QRegExp parameterPattern_;
    QRegExp decoratorPattern_;
};

#endif // PYTHON_PARSER_H 