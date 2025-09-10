#ifndef CPP_PARSER_H
#define CPP_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <QRegExp>
#include <QStringList>

struct CppParameter {
    std::string type;
    std::string name;
    std::string defaultValue;
};

struct CppFunction {
    std::string name;
    std::string returnType;
    std::vector<CppParameter> parameters;
    std::vector<std::string> calledFunctions;
    int lineNumber;
    std::string className;  // 如果是类成员函数
    bool isStatic;
    bool isVirtual;
    bool isConst;
    std::string accessLevel;  // public, private, protected
};

struct CppClass {
    std::string name;
    std::vector<std::string> baseClasses;
    std::vector<CppFunction> methods;
    std::vector<std::string> memberVariables;
    int lineNumber;
};

class CppParser {
public:
    CppParser();
    ~CppParser();
    
    // 解析 C++ 文件
    bool parseFile(const std::string& content);
    
    // 获取解析结果
    const std::vector<CppFunction>& getFunctions() const { return functions_; }
    const std::vector<CppClass>& getClasses() const { return classes_; }
    const std::map<std::string, std::vector<std::string>>& getFunctionCalls() const { return functionCalls_; }
    
    // 辅助函数
    std::string getFunctionSignature(const CppFunction& func) const;
    std::vector<std::string> getCalledFunctions(const std::string& functionName) const;
    std::vector<std::string> getCallingFunctions(const std::string& functionName) const;
    
    // 清空解析结果
    void clear();

private:
    std::vector<CppFunction> functions_;
    std::vector<CppClass> classes_;
    std::map<std::string, std::vector<std::string>> functionCalls_;  // function -> list of called functions
    
    // 解析辅助函数
    void parseFunctions(const std::string& content);
    void parseClasses(const std::string& content);
    void parseFunctionCalls(const std::string& content);
    
    CppFunction parseFunctionDeclaration(const std::string& line, int lineNumber);
    std::vector<CppParameter> parseParameters(const std::string& paramStr);
    std::string removeComments(const std::string& content);
    std::string normalizeWhitespace(const std::string& text);
    std::vector<std::string> splitLines(const std::string& content);
    
    // 正则表达式模式
    QRegExp functionPattern_;
    QRegExp classPattern_;
    QRegExp functionCallPattern_;
    QRegExp parameterPattern_;
};

#endif // CPP_PARSER_H 