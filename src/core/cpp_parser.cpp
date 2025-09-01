#include "cpp_parser.h"
#include <algorithm>
#include <sstream>
#include <QRegExp>
#include <QString>

CppParser::CppParser() {
    // 函数定义模式：修饰符 返回类型 函数名(参数)
    functionPattern_ = QRegExp(
        "(?:^|\\s+)(?:virtual\\s+|static\\s+|inline\\s+)?"  // 可选修饰符
        "([\\w:]+(?:\\s*[*&])?)"                            // 返回类型
        "\\s+([\\w~]+)"                                     // 函数名
        "\\s*\\(([^)]*)\\)"                                 // 参数列表
        "\\s*(?:const)?\\s*[{;]"                           // 可选const和函数体开始或声明结束
    );
    
    // 类定义模式
    classPattern_ = QRegExp("class\\s+([\\w]+)\\s*(?::\\s*(.*))?\\s*\\{");
    
    // 函数调用模式
    functionCallPattern_ = QRegExp("([\\w]+)\\s*\\(");
    
    // 参数模式
    parameterPattern_ = QRegExp("((?:const\\s+)?[\\w:]+(?:\\s*[*&])?)\\s+([\\w]+)(?:\\s*=\\s*([^,)]*))?");
}

CppParser::~CppParser() {
    clear();
}

bool CppParser::parseFile(const std::string& content) {
    clear();
    
    if (content.empty()) {
        return false;
    }
    
    try {
        // 移除注释
        std::string cleanContent = removeComments(content);
        
        // 解析类
        parseClasses(cleanContent);
        
        // 解析函数
        parseFunctions(cleanContent);
        
        // 解析函数调用关系
        parseFunctionCalls(cleanContent);
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void CppParser::parseFunctions(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line);
        
        // 跳过注释、预处理器指令和空行
        if (line.empty() || line[0] == '#' || 
            line.find("//") == 0 || line.find("/*") != std::string::npos) {
            continue;
        }
        
        // 查找函数定义
        if (functionPattern_.indexIn(qline) != -1) {
            CppFunction func = parseFunctionDeclaration(line, i + 1);
            if (!func.name.empty()) {
                functions_.push_back(func);
            }
        }
    }
}

void CppParser::parseClasses(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line);
        
        if (classPattern_.indexIn(qline) != -1) {
            CppClass cls;
            cls.name = classPattern_.cap(1).toStdString();
            cls.lineNumber = i + 1;
            
            // 解析基类
            if (!classPattern_.cap(2).isEmpty()) {
                QString baseClasses = classPattern_.cap(2);
                QStringList bases = baseClasses.split(",");
                for (const QString& base : bases) {
                    QString trimmed = base.trimmed();
                    // 移除访问修饰符
                    trimmed.replace(QRegExp("^(public|private|protected)\\s+"), "");
                    cls.baseClasses.push_back(trimmed.toStdString());
                }
            }
            
            classes_.push_back(cls);
        }
    }
}

void CppParser::parseFunctionCalls(const std::string& content) {
    std::vector<std::string> lines = splitLines(content);
    std::string currentFunction;
    
    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        QString qline = QString::fromStdString(line);
        
        // 检查是否是函数定义开始
        if (functionPattern_.indexIn(qline) != -1) {
            currentFunction = functionPattern_.cap(2).toStdString();
        }
        
        // 在函数体内查找函数调用
        if (!currentFunction.empty() && line.find("{") != std::string::npos) {
            // 开始解析函数体
            size_t braceCount = std::count(line.begin(), line.end(), '{') - 
                                std::count(line.begin(), line.end(), '}');
            
            for (size_t j = i + 1; j < lines.size() && braceCount > 0; ++j) {
                const std::string& bodyLine = lines[j];
                QString qBodyLine = QString::fromStdString(bodyLine);
                
                braceCount += std::count(bodyLine.begin(), bodyLine.end(), '{') - 
                              std::count(bodyLine.begin(), bodyLine.end(), '}');
                
                // 查找函数调用
                int pos = 0;
                while ((pos = functionCallPattern_.indexIn(qBodyLine, pos)) != -1) {
                    QString calledFunc = functionCallPattern_.cap(1);
                    std::string calledFuncStr = calledFunc.toStdString();
                    
                    // 避免关键字和常见的非函数调用
                    if (calledFuncStr != "if" && calledFuncStr != "while" && 
                        calledFuncStr != "for" && calledFuncStr != "switch" &&
                        calledFuncStr != "return" && calledFuncStr != "sizeof") {
                        functionCalls_[currentFunction].push_back(calledFuncStr);
                    }
                    
                    pos += functionCallPattern_.matchedLength();
                }
            }
            
            currentFunction.clear();
        }
    }
}

CppFunction CppParser::parseFunctionDeclaration(const std::string& line, int lineNumber) {
    CppFunction func;
    QString qline = QString::fromStdString(line);
    
    if (functionPattern_.indexIn(qline) != -1) {
        func.returnType = functionPattern_.cap(1).toStdString();
        func.name = functionPattern_.cap(2).toStdString();
        func.lineNumber = lineNumber;
        
        // 解析修饰符
        func.isStatic = line.find("static") != std::string::npos;
        func.isVirtual = line.find("virtual") != std::string::npos;
        func.isConst = line.find("const") != std::string::npos;
        
        // 解析参数
        QString paramStr = functionPattern_.cap(3);
        func.parameters = parseParameters(paramStr.toStdString());
    }
    
    return func;
}

std::vector<CppParameter> CppParser::parseParameters(const std::string& paramStr) {
    std::vector<CppParameter> parameters;
    
    if (paramStr.empty() || paramStr == "void") {
        return parameters;
    }
    
    QString qParamStr = QString::fromStdString(paramStr);
    QStringList paramList = qParamStr.split(",");
    
    for (const QString& param : paramList) {
        QString trimmed = param.trimmed();
        if (trimmed.isEmpty()) continue;
        
        if (parameterPattern_.indexIn(trimmed) != -1) {
            CppParameter p;
            p.type = parameterPattern_.cap(1).toStdString();
            p.name = parameterPattern_.cap(2).toStdString();
            p.defaultValue = parameterPattern_.cap(3).toStdString();
            parameters.push_back(p);
        } else {
            // 简单解析：假设最后一个单词是参数名
            QStringList words = trimmed.split(QRegExp("\\s+"));
            if (words.size() >= 2) {
                CppParameter p;
                p.name = words.last().toStdString();
                words.removeLast();
                p.type = words.join(" ").toStdString();
                parameters.push_back(p);
            }
        }
    }
    
    return parameters;
}

std::string CppParser::removeComments(const std::string& content) {
    std::string result = content;
    
    // 移除单行注释
    size_t pos = 0;
    while ((pos = result.find("//", pos)) != std::string::npos) {
        size_t endPos = result.find('\n', pos);
        if (endPos == std::string::npos) {
            result.erase(pos);
            break;
        }
        result.erase(pos, endPos - pos);
    }
    
    // 移除多行注释
    pos = 0;
    while ((pos = result.find("/*", pos)) != std::string::npos) {
        size_t endPos = result.find("*/", pos);
        if (endPos == std::string::npos) {
            result.erase(pos);
            break;
        }
        result.erase(pos, endPos - pos + 2);
    }
    
    return result;
}

std::string CppParser::normalizeWhitespace(const std::string& text) {
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

std::vector<std::string> CppParser::splitLines(const std::string& content) {
    std::vector<std::string> lines;
    std::stringstream ss(content);
    std::string line;
    
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    
    return lines;
}

std::string CppParser::getFunctionSignature(const CppFunction& func) const {
    std::string signature = func.returnType + " " + func.name + "(";
    
    for (size_t i = 0; i < func.parameters.size(); ++i) {
        if (i > 0) signature += ", ";
        signature += func.parameters[i].type + " " + func.parameters[i].name;
        if (!func.parameters[i].defaultValue.empty()) {
            signature += " = " + func.parameters[i].defaultValue;
        }
    }
    
    signature += ")";
    
    if (func.isConst) {
        signature += " const";
    }
    
    return signature;
}

std::vector<std::string> CppParser::getCalledFunctions(const std::string& functionName) const {
    auto it = functionCalls_.find(functionName);
    if (it != functionCalls_.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::string> CppParser::getCallingFunctions(const std::string& functionName) const {
    std::vector<std::string> callers;
    
    for (const auto& pair : functionCalls_) {
        const auto& calledFunctions = pair.second;
        if (std::find(calledFunctions.begin(), calledFunctions.end(), functionName) != calledFunctions.end()) {
            callers.push_back(pair.first);
        }
    }
    
    return callers;
}

void CppParser::clear() {
    functions_.clear();
    classes_.clear();
    functionCalls_.clear();
} 