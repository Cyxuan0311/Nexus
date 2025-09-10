#include "python_parser.h"
#include <algorithm>
#include <sstream>
#include <QRegExp>
#include <QString>

PythonParser::PythonParser() {
    // Function definition pattern: def function_name(parameters):
    functionPattern_ = QRegExp("^\\s*def\\s+(\\w+)\\s*\\(([^)]*)\\)\\s*(?:\\s*->\\s*([^:]+))?:");
    
    // Async function definition pattern: async def function_name(parameters):
    asyncFunctionPattern_ = QRegExp("^\\s*async\\s+def\\s+(\\w+)\\s*\\(([^)]*)\\)\\s*(?:\\s*->\\s*([^:]+))?:");
    
    // Class definition pattern: class ClassName(BaseClass):
    classPattern_ = QRegExp("^\\s*class\\s+(\\w+)(?:\\s*\\(([^)]*)\\))?\\s*:");
    
    // Function call pattern: function_name(
    functionCallPattern_ = QRegExp("(\\w+)\\s*\\(");
    
    // Parameter pattern with type annotations
    parameterPattern_ = QRegExp("(\\w+)(?:\\s*:\\s*([^=,)]+))?(?:\\s*=\\s*([^,)]+))?");
    
    // Decorator pattern: @decorator_name
    decoratorPattern_ = QRegExp("^\\s*@(\\w+)");
}

PythonParser::~PythonParser() {
    clear();
}

bool PythonParser::parseFile(const std::string& content) {
    clear();
    
    if (content.empty()) {
        return false;
    }
    
    try {
        // Remove excessive comments but keep docstrings
        std::string cleanContent = removeComments(content);
        
        // Parse classes
        parseClasses(cleanContent);
        
        // Parse functions
        parseFunctions(cleanContent);
        
        // Parse function calls
        parseFunctionCalls(cleanContent);
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void PythonParser::parseFunctions(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line);
        
        // Skip blank lines and pure comments
        if (isBlankOrComment(line)) {
            continue;
        }
        
        // Check for function definition
        if (functionPattern_.indexIn(qline) != -1 || asyncFunctionPattern_.indexIn(qline) != -1) {
            PythonFunction func = parseFunctionDefinition(lines, i);
            if (!func.name.empty()) {
                functions_.push_back(func);
            }
        }
    }
}

void PythonParser::parseClasses(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line);
        
        if (classPattern_.indexIn(qline) != -1) {
            PythonClass cls;
            cls.name = classPattern_.cap(1).toStdString();
            cls.lineNumber = i + 1;
            
            // Parse base classes
            if (!classPattern_.cap(2).isEmpty()) {
                QString baseClasses = classPattern_.cap(2);
                QStringList bases = baseClasses.split(",");
                for (const QString& base : bases) {
                    QString trimmed = base.trimmed();
                    cls.baseClasses.push_back(trimmed.toStdString());
                }
            }
            
            // Extract docstring
            cls.docstring = extractDocstring(lines, i + 1);
            
            classes_.push_back(cls);
        }
    }
}

void PythonParser::parseFunctionCalls(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    std::string currentFunction;
    int currentIndent = -1;
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line);
        
        // Check if this is a function definition
        if (functionPattern_.indexIn(qline) != -1) {
            currentFunction = functionPattern_.cap(1).toStdString();
            currentIndent = getIndentLevel(line);
        } else if (asyncFunctionPattern_.indexIn(qline) != -1) {
            currentFunction = asyncFunctionPattern_.cap(1).toStdString();
            currentIndent = getIndentLevel(line);
        }
        
        // If we're inside a function, look for function calls
        if (!currentFunction.empty()) {
            int lineIndent = getIndentLevel(line);
            
            // If we've reached a line with same or less indentation than function definition,
            // we're no longer in the function (unless it's a blank/comment line)
            if (lineIndent <= currentIndent && !isBlankOrComment(line) && 
                functionPattern_.indexIn(qline) == -1 && asyncFunctionPattern_.indexIn(qline) == -1) {
                currentFunction.clear();
                currentIndent = -1;
                continue;
            }
            
            // Look for function calls in this line
            int pos = 0;
            while ((pos = functionCallPattern_.indexIn(qline, pos)) != -1) {
                QString calledFunc = functionCallPattern_.cap(1);
                std::string calledFuncStr = calledFunc.toStdString();
                
                // Avoid Python keywords and built-ins
                if (calledFuncStr != "if" && calledFuncStr != "while" && 
                    calledFuncStr != "for" && calledFuncStr != "try" &&
                    calledFuncStr != "with" && calledFuncStr != "print" &&
                    calledFuncStr != "len" && calledFuncStr != "range" &&
                    calledFuncStr != "str" && calledFuncStr != "int" &&
                    calledFuncStr != "float" && calledFuncStr != "list" &&
                    calledFuncStr != "dict" && calledFuncStr != "set" &&
                    calledFuncStr != "tuple" && calledFuncStr != "bool") {
                    functionCalls_[currentFunction].push_back(calledFuncStr);
                }
                
                pos += functionCallPattern_.matchedLength();
            }
        }
    }
}

PythonFunction PythonParser::parseFunctionDefinition(const std::vector<std::string>& lines, int startLine) {
    PythonFunction func;
    const std::string& line = lines[startLine];
    QString qline = QString::fromStdString(line);
    
    // Check for async function
    if (asyncFunctionPattern_.indexIn(qline) != -1) {
        func.name = asyncFunctionPattern_.cap(1).toStdString();
        func.isAsync = true;
        func.returnType = asyncFunctionPattern_.cap(3).toStdString();
        
        // Parse parameters
        QString paramStr = asyncFunctionPattern_.cap(2);
        func.parameters = parseParameters(paramStr.toStdString());
    } else if (functionPattern_.indexIn(qline) != -1) {
        func.name = functionPattern_.cap(1).toStdString();
        func.isAsync = false;
        func.returnType = functionPattern_.cap(3).toStdString();
        
        // Parse parameters
        QString paramStr = functionPattern_.cap(2);
        func.parameters = parseParameters(paramStr.toStdString());
    }
    
    func.lineNumber = startLine + 1;
    
    // Check if function is private (starts with underscore)
    func.isPrivate = func.name.length() > 0 && func.name[0] == '_';
    
    // Look for decorators before the function
    for (int i = startLine - 1; i >= 0; --i) {
        const std::string& prevLine = lines[i];
        QString qPrevLine = QString::fromStdString(prevLine);
        
        if (decoratorPattern_.indexIn(qPrevLine) != -1) {
            std::string decorator = decoratorPattern_.cap(1).toStdString();
            if (func.decorator.empty()) {
                func.decorator = decorator;
            } else {
                func.decorator = decorator + ", " + func.decorator;
            }
            
            // Check for special decorators
            if (decorator == "staticmethod") {
                func.isStaticMethod = true;
            } else if (decorator == "classmethod") {
                func.isClassMethod = true;
            }
        } else if (!isBlankOrComment(prevLine)) {
            break; // Stop if we hit a non-decorator, non-blank line
        }
    }
    
    // Extract docstring
    func.docstring = extractDocstring(lines, startLine + 1);
    
    return func;
}

std::vector<PythonParameter> PythonParser::parseParameters(const std::string& paramStr) {
    std::vector<PythonParameter> parameters;
    
    if (paramStr.empty()) {
        return parameters;
    }
    
    QString qParamStr = QString::fromStdString(paramStr);
    QStringList paramList = qParamStr.split(",");
    
    for (const QString& param : paramList) {
        QString trimmed = param.trimmed();
        if (trimmed.isEmpty()) continue;
        
        PythonParameter p;
        p.isOptional = false;
        
        if (parameterPattern_.indexIn(trimmed) != -1) {
            p.name = parameterPattern_.cap(1).toStdString();
            p.type = parameterPattern_.cap(2).toStdString();
            p.defaultValue = parameterPattern_.cap(3).toStdString();
            p.isOptional = !p.defaultValue.empty();
        } else {
            // Simple parameter name without type annotation
            p.name = trimmed.toStdString();
        }
        
        if (!p.name.empty()) {
            parameters.push_back(p);
        }
    }
    
    return parameters;
}

std::string PythonParser::removeComments(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    std::string result;
    
    for (const std::string& line : lines) {
        std::string processedLine = line;
        
        // Find # that's not inside a string
        bool inString = false;
        char stringChar = '\0';
        bool inTripleString = false;
        
        for (size_t i = 0; i < processedLine.length(); ++i) {
            char c = processedLine[i];
            
            if (!inString && !inTripleString) {
                if (c == '\'' || c == '"') {
                    // Check for triple quotes
                    if (i + 2 < processedLine.length() && 
                        processedLine.substr(i, 3) == std::string(3, c)) {
                        inTripleString = true;
                        stringChar = c;
                        i += 2; // Skip next two characters
                        continue;
                    } else {
                        inString = true;
                        stringChar = c;
                    }
                } else if (c == '#') {
                    // Found comment, truncate line here
                    processedLine = processedLine.substr(0, i);
                    break;
                }
            } else if (inTripleString) {
                if (i + 2 < processedLine.length() && 
                    processedLine.substr(i, 3) == std::string(3, stringChar)) {
                    inTripleString = false;
                    stringChar = '\0';
                    i += 2;
                }
            } else if (inString) {
                if (c == stringChar && (i == 0 || processedLine[i-1] != '\\')) {
                    inString = false;
                    stringChar = '\0';
                }
            }
        }
        
        result += processedLine + "\n";
    }
    
    return result;
}

std::string PythonParser::normalizeWhitespace(const std::string& text) {
    std::string result;
    bool inWhitespace = false;
    
    for (char c : text) {
        if (std::isspace(c)) {
            if (!inWhitespace) {
                result += ' ';
                inWhitespace = true;
            }
        } else {
            result += c;
            inWhitespace = false;
        }
    }
    
    return result;
}

std::vector<std::string> PythonParser::splitLines(const std::string& content) {
    std::vector<std::string> lines;
    std::stringstream ss(content);
    std::string line;
    
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    
    return lines;
}

std::string PythonParser::extractDocstring(const std::vector<std::string>& lines, int startLine) {
    if (startLine >= (int)lines.size()) {
        return "";
    }
    
    // Look for docstring starting after the function definition
    for (int i = startLine; i < (int)lines.size(); ++i) {
        const std::string& line = lines[i];
        
        if (isBlankOrComment(line)) {
            continue;
        }
        
        // Check if this line contains a string literal (potential docstring)
        QString qline = QString::fromStdString(line);
        qline = qline.trimmed();
        
        if (qline.startsWith("\"\"\"") || qline.startsWith("'''")) {
            // Found start of docstring
            std::string docstring;
            std::string delimiter = qline.startsWith("\"\"\"") ? "\"\"\"" : "'''";
            
            // Check if it's a single-line docstring
            if (qline.endsWith(QString::fromStdString(delimiter)) && qline.length() > 6) {
                docstring = qline.mid(3, qline.length() - 6).toStdString();
                return docstring;
            } else {
                // Multi-line docstring
                docstring = qline.mid(3).toStdString();
                
                for (int j = i + 1; j < (int)lines.size(); ++j) {
                    const std::string& docLine = lines[j];
                    QString qDocLine = QString::fromStdString(docLine);
                    
                    if (qDocLine.contains(QString::fromStdString(delimiter))) {
                        // Found end of docstring
                        int endPos = qDocLine.indexOf(QString::fromStdString(delimiter));
                        if (endPos > 0) {
                            docstring += "\n" + qDocLine.left(endPos).toStdString();
                        }
                        break;
                    } else {
                        docstring += "\n" + docLine;
                    }
                }
                
                return docstring;
            }
        } else {
            // No docstring found
            break;
        }
    }
    
    return "";
}

int PythonParser::getIndentLevel(const std::string& line) {
    int indent = 0;
    for (char c : line) {
        if (c == ' ') {
            indent++;
        } else if (c == '\t') {
            indent += 4; // Assume tab = 4 spaces
        } else {
            break;
        }
    }
    return indent;
}

bool PythonParser::isBlankOrComment(const std::string& line) {
    QString qline = QString::fromStdString(line).trimmed();
    return qline.isEmpty() || qline.startsWith("#");
}

std::string PythonParser::getFunctionSignature(const PythonFunction& func) const {
    std::string signature;
    
    if (func.isAsync) {
        signature += "async ";
    }
    
    signature += "def " + func.name + "(";
    
    for (size_t i = 0; i < func.parameters.size(); ++i) {
        if (i > 0) signature += ", ";
        
        signature += func.parameters[i].name;
        
        if (!func.parameters[i].type.empty()) {
            signature += ": " + func.parameters[i].type;
        }
        
        if (!func.parameters[i].defaultValue.empty()) {
            signature += " = " + func.parameters[i].defaultValue;
        }
    }
    
    signature += ")";
    
    if (!func.returnType.empty()) {
        signature += " -> " + func.returnType;
    }
    
    return signature;
}

std::vector<std::string> PythonParser::getCalledFunctions(const std::string& functionName) const {
    auto it = functionCalls_.find(functionName);
    if (it != functionCalls_.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::string> PythonParser::getCallingFunctions(const std::string& functionName) const {
    std::vector<std::string> callers;
    
    for (const auto& pair : functionCalls_) {
        const auto& calledFunctions = pair.second;
        if (std::find(calledFunctions.begin(), calledFunctions.end(), functionName) != calledFunctions.end()) {
            callers.push_back(pair.first);
        }
    }
    
    return callers;
}

void PythonParser::clear() {
    functions_.clear();
    classes_.clear();
    functionCalls_.clear();
} 