#include "go_parser.h"
#include <algorithm>
#include <sstream>
#include <QRegExp>
#include <QString>

GoParser::GoParser() {
    // Package declaration pattern
    packagePattern_ = QRegExp("^\\s*package\\s+(\\w+)");
    
    // Function definition pattern: func functionName(parameters) returnType
    functionPattern_ = QRegExp("^\\s*func\\s+(\\w+)\\s*\\(([^)]*)\\)\\s*([^{]*)?\\s*\\{?");
    
    // Method definition pattern: func (receiver Type) methodName(parameters) returnType
    methodPattern_ = QRegExp("^\\s*func\\s*\\(\\s*(\\w+)\\s+(\\*?\\w+)\\s*\\)\\s*(\\w+)\\s*\\(([^)]*)\\)\\s*([^{]*)?\\s*\\{?");
    
    // Struct definition pattern: type StructName struct {
    structPattern_ = QRegExp("^\\s*type\\s+(\\w+)\\s+struct\\s*\\{");
    
    // Interface definition pattern: type InterfaceName interface {
    interfacePattern_ = QRegExp("^\\s*type\\s+(\\w+)\\s+interface\\s*\\{");
    
    // Function call pattern: functionName(
    functionCallPattern_ = QRegExp("(\\w+)\\s*\\(");
    
    // Parameter pattern: name type or name ...type
    parameterPattern_ = QRegExp("(\\w+)\\s+(\\.\\.\\.)?(\\*?[\\w\\[\\]]+)");
}

GoParser::~GoParser() {
    clear();
}

bool GoParser::parseFile(const std::string& content) {
    clear();
    
    if (content.empty()) {
        return false;
    }
    
    try {
        // Remove comments but keep structure
        std::string cleanContent = removeComments(content);
        
        // Parse package declaration
        parsePackage(cleanContent);
        
        // Parse structs
        parseStructs(cleanContent);
        
        // Parse interfaces
        parseInterfaces(cleanContent);
        
        // Parse functions
        parseFunctions(cleanContent);
        
        // Parse function calls
        parseFunctionCalls(cleanContent);
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void GoParser::parsePackage(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    
    for (const std::string& line : lines) {
        QString qline = QString::fromStdString(line);
        
        if (packagePattern_.indexIn(qline) != -1) {
            packageName_ = packagePattern_.cap(1).toStdString();
            break;
        }
    }
}

void GoParser::parseFunctions(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line);
        
        // Skip blank lines and comments
        if (isBlankOrComment(line)) {
            continue;
        }
        
        // Check for method definition first (has receiver)
        if (methodPattern_.indexIn(qline) != -1) {
            GoFunction func = parseFunctionDefinition(lines, i);
            if (!func.name.empty()) {
                func.isMethod = true;
                func.receiverName = methodPattern_.cap(1).toStdString();
                func.receiverType = methodPattern_.cap(2).toStdString();
                functions_.push_back(func);
            }
        }
        // Check for regular function definition
        else if (functionPattern_.indexIn(qline) != -1) {
            GoFunction func = parseFunctionDefinition(lines, i);
            if (!func.name.empty()) {
                func.isMethod = false;
                functions_.push_back(func);
            }
        }
    }
}

void GoParser::parseStructs(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line);
        
        if (structPattern_.indexIn(qline) != -1) {
            GoStruct strct;
            strct.name = structPattern_.cap(1).toStdString();
            strct.lineNumber = i + 1;
            strct.isExported = isExported(strct.name);
            strct.comment = extractComment(lines, i);
            
            // Parse struct fields (simplified - just collect field names)
            for (size_t j = i + 1; j < lines.size(); ++j) {
                const std::string& fieldLine = lines[j];
                QString qFieldLine = QString::fromStdString(fieldLine).trimmed();
                
                if (qFieldLine == "}") {
                    break; // End of struct
                }
                
                if (!qFieldLine.isEmpty() && !qFieldLine.startsWith("//")) {
                    // Simple field parsing - extract field name
                    QStringList parts = qFieldLine.split(QRegExp("\\s+"));
                    if (!parts.isEmpty()) {
                        strct.fields.push_back(parts[0].toStdString());
                    }
                }
            }
            
            structs_.push_back(strct);
        }
    }
}

void GoParser::parseInterfaces(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line);
        
        if (interfacePattern_.indexIn(qline) != -1) {
            GoInterface iface;
            iface.name = interfacePattern_.cap(1).toStdString();
            iface.lineNumber = i + 1;
            iface.isExported = isExported(iface.name);
            iface.comment = extractComment(lines, i);
            
            // Parse interface methods (simplified)
            for (size_t j = i + 1; j < lines.size(); ++j) {
                const std::string& methodLine = lines[j];
                QString qMethodLine = QString::fromStdString(methodLine).trimmed();
                
                if (qMethodLine == "}") {
                    break; // End of interface
                }
                
                if (!qMethodLine.isEmpty() && !qMethodLine.startsWith("//")) {
                    // Simple method parsing - extract method signature
                    iface.methods.push_back(qMethodLine.toStdString());
                }
            }
            
            interfaces_.push_back(iface);
        }
    }
}

void GoParser::parseFunctionCalls(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    std::string currentFunction;
    int braceLevel = 0;
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line);
        
        // Check if this is a function definition
        if (functionPattern_.indexIn(qline) != -1) {
            currentFunction = functionPattern_.cap(1).toStdString();
            braceLevel = 0;
        } else if (methodPattern_.indexIn(qline) != -1) {
            currentFunction = methodPattern_.cap(3).toStdString();
            braceLevel = 0;
        }
        
        // Count braces to track function scope
        braceLevel += std::count(line.begin(), line.end(), '{');
        braceLevel -= std::count(line.begin(), line.end(), '}');
        
        // If we're inside a function, look for function calls
        if (!currentFunction.empty() && braceLevel > 0) {
            int pos = 0;
            while ((pos = functionCallPattern_.indexIn(qline, pos)) != -1) {
                QString calledFunc = functionCallPattern_.cap(1);
                std::string calledFuncStr = calledFunc.toStdString();
                
                // Avoid Go keywords and built-ins
                if (calledFuncStr != "if" && calledFuncStr != "for" && 
                    calledFuncStr != "switch" && calledFuncStr != "select" &&
                    calledFuncStr != "go" && calledFuncStr != "defer" &&
                    calledFuncStr != "make" && calledFuncStr != "new" &&
                    calledFuncStr != "len" && calledFuncStr != "cap" &&
                    calledFuncStr != "append" && calledFuncStr != "copy" &&
                    calledFuncStr != "delete" && calledFuncStr != "panic" &&
                    calledFuncStr != "recover" && calledFuncStr != "print" &&
                    calledFuncStr != "println") {
                    functionCalls_[currentFunction].push_back(calledFuncStr);
                }
                
                pos += functionCallPattern_.matchedLength();
            }
        }
        
        // Reset when we exit function scope
        if (braceLevel <= 0 && !currentFunction.empty()) {
            currentFunction.clear();
        }
    }
}

GoFunction GoParser::parseFunctionDefinition(const std::vector<std::string>& lines, int startLine) {
    GoFunction func;
    const std::string& line = lines[startLine];
    QString qline = QString::fromStdString(line);
    
    // Check if it's a method (with receiver)
    if (methodPattern_.indexIn(qline) != -1) {
        func.name = methodPattern_.cap(3).toStdString();
        func.receiverName = methodPattern_.cap(1).toStdString();
        func.receiverType = methodPattern_.cap(2).toStdString();
        func.isMethod = true;
        
        // Parse parameters
        QString paramStr = methodPattern_.cap(4);
        func.parameters = parseParameters(paramStr.toStdString());
        
        // Parse return types
        QString returnStr = methodPattern_.cap(5);
        func.returnTypes = parseReturnTypes(returnStr.toStdString());
    }
    // Regular function
    else if (functionPattern_.indexIn(qline) != -1) {
        func.name = functionPattern_.cap(1).toStdString();
        func.isMethod = false;
        
        // Parse parameters
        QString paramStr = functionPattern_.cap(2);
        func.parameters = parseParameters(paramStr.toStdString());
        
        // Parse return types
        QString returnStr = functionPattern_.cap(3);
        func.returnTypes = parseReturnTypes(returnStr.toStdString());
    }
    
    func.lineNumber = startLine + 1;
    func.isExported = isExported(func.name);
    func.packageName = packageName_;
    func.comment = extractComment(lines, startLine);
    
    return func;
}

std::vector<GoParameter> GoParser::parseParameters(const std::string& paramStr) {
    std::vector<GoParameter> parameters;
    
    if (paramStr.empty()) {
        return parameters;
    }
    
    QString qParamStr = QString::fromStdString(paramStr);
    QStringList paramList = qParamStr.split(",");
    
    for (const QString& param : paramList) {
        QString trimmed = param.trimmed();
        if (trimmed.isEmpty()) continue;
        
        GoParameter p;
        p.isVariadic = false;
        
        if (parameterPattern_.indexIn(trimmed) != -1) {
            p.name = parameterPattern_.cap(1).toStdString();
            p.isVariadic = !parameterPattern_.cap(2).isEmpty();
            p.type = parameterPattern_.cap(3).toStdString();
        } else {
            // Simple parsing - try to extract name and type
            QStringList parts = trimmed.split(QRegExp("\\s+"));
            if (parts.size() >= 2) {
                p.name = parts[0].toStdString();
                p.type = parts[1].toStdString();
                if (p.type.substr(0, 3) == "...") {
                    p.isVariadic = true;
                    p.type = p.type.substr(3); // Remove "..."
                }
            } else if (parts.size() == 1) {
                // Just a type, no name
                p.type = parts[0].toStdString();
            }
        }
        
        if (!p.type.empty()) {
            parameters.push_back(p);
        }
    }
    
    return parameters;
}

std::vector<std::string> GoParser::parseReturnTypes(const std::string& returnStr) {
    std::vector<std::string> returnTypes;
    
    QString qReturnStr = QString::fromStdString(returnStr).trimmed();
    if (qReturnStr.isEmpty()) {
        return returnTypes;
    }
    
    // Remove parentheses if present (multiple return values)
    if (qReturnStr.startsWith("(") && qReturnStr.endsWith(")")) {
        qReturnStr = qReturnStr.mid(1, qReturnStr.length() - 2);
    }
    
    QStringList types = qReturnStr.split(",");
    for (const QString& type : types) {
        QString trimmed = type.trimmed();
        if (!trimmed.isEmpty()) {
            returnTypes.push_back(trimmed.toStdString());
        }
    }
    
    return returnTypes;
}

std::string GoParser::removeComments(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    std::string result;
    
    bool inBlockComment = false;
    
    for (const std::string& line : lines) {
        std::string processedLine = line;
        
        if (!inBlockComment) {
            // Handle single-line comments
            size_t commentPos = processedLine.find("//");
            if (commentPos != std::string::npos) {
                processedLine = processedLine.substr(0, commentPos);
            }
            
            // Handle start of block comments
            size_t blockStart = processedLine.find("/*");
            if (blockStart != std::string::npos) {
                size_t blockEnd = processedLine.find("*/", blockStart + 2);
                if (blockEnd != std::string::npos) {
                    // Block comment starts and ends on same line
                    processedLine = processedLine.substr(0, blockStart) + 
                                  processedLine.substr(blockEnd + 2);
                } else {
                    // Block comment starts but doesn't end
                    processedLine = processedLine.substr(0, blockStart);
                    inBlockComment = true;
                }
            }
        } else {
            // We're in a block comment, look for end
            size_t blockEnd = processedLine.find("*/");
            if (blockEnd != std::string::npos) {
                processedLine = processedLine.substr(blockEnd + 2);
                inBlockComment = false;
            } else {
                processedLine = "";
            }
        }
        
        result += processedLine + "\n";
    }
    
    return result;
}

std::string GoParser::normalizeWhitespace(const std::string& text) {
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

std::vector<std::string> GoParser::splitLines(const std::string& content) {
    std::vector<std::string> lines;
    std::stringstream ss(content);
    std::string line;
    
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    
    return lines;
}

std::string GoParser::extractComment(const std::vector<std::string>& lines, int startLine) {
    std::string comment;
    
    // Look for comment before the function
    for (int i = startLine - 1; i >= 0; --i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line).trimmed();
        
        if (qline.startsWith("//")) {
            comment = qline.mid(2).trimmed().toStdString() + " " + comment;
        } else if (!qline.isEmpty()) {
            break; // Hit non-comment, non-blank line
        }
    }
    
    return comment;
}

bool GoParser::isBlankOrComment(const std::string& line) {
    QString qline = QString::fromStdString(line).trimmed();
    return qline.isEmpty() || qline.startsWith("//") || qline.startsWith("/*");
}

bool GoParser::isExported(const std::string& name) {
    return !name.empty() && std::isupper(name[0]);
}

std::string GoParser::getFunctionSignature(const GoFunction& func) const {
    std::string signature;
    
    if (func.isMethod) {
        signature += "func (" + func.receiverName + " " + func.receiverType + ") ";
    } else {
        signature += "func ";
    }
    
    signature += func.name + "(";
    
    for (size_t i = 0; i < func.parameters.size(); ++i) {
        if (i > 0) signature += ", ";
        
        if (!func.parameters[i].name.empty()) {
            signature += func.parameters[i].name + " ";
        }
        
        if (func.parameters[i].isVariadic) {
            signature += "...";
        }
        
        signature += func.parameters[i].type;
    }
    
    signature += ")";
    
    if (!func.returnTypes.empty()) {
        if (func.returnTypes.size() == 1) {
            signature += " " + func.returnTypes[0];
        } else {
            signature += " (";
            for (size_t i = 0; i < func.returnTypes.size(); ++i) {
                if (i > 0) signature += ", ";
                signature += func.returnTypes[i];
            }
            signature += ")";
        }
    }
    
    return signature;
}

std::vector<std::string> GoParser::getCalledFunctions(const std::string& functionName) const {
    auto it = functionCalls_.find(functionName);
    if (it != functionCalls_.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::string> GoParser::getCallingFunctions(const std::string& functionName) const {
    std::vector<std::string> callers;
    
    for (const auto& pair : functionCalls_) {
        const auto& calledFunctions = pair.second;
        if (std::find(calledFunctions.begin(), calledFunctions.end(), functionName) != calledFunctions.end()) {
            callers.push_back(pair.first);
        }
    }
    
    return callers;
}

void GoParser::clear() {
    functions_.clear();
    structs_.clear();
    interfaces_.clear();
    functionCalls_.clear();
    packageName_.clear();
} 