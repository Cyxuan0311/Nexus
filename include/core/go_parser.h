#ifndef GO_PARSER_H
#define GO_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <QRegExp>

struct GoParameter {
    std::string name;
    std::string type;
    bool isVariadic;  // ...type
};

struct GoFunction {
    std::string name;
    std::vector<std::string> returnTypes;  // Go can have multiple return values
    std::vector<GoParameter> parameters;
    std::vector<std::string> calledFunctions;
    int lineNumber;
    std::string receiverType;  // For methods (receiver type)
    std::string receiverName;  // For methods (receiver name)
    bool isMethod;
    bool isExported;  // starts with uppercase letter
    std::string packageName;
    std::string comment;
};

struct GoStruct {
    std::string name;
    std::vector<std::string> fields;
    std::vector<GoFunction> methods;
    int lineNumber;
    bool isExported;
    std::string comment;
};

struct GoInterface {
    std::string name;
    std::vector<std::string> methods;
    int lineNumber;
    bool isExported;
    std::string comment;
};

class GoParser {
public:
    GoParser();
    ~GoParser();
    
    // Parse Go file
    bool parseFile(const std::string& content);
    
    // Get parsing results
    const std::vector<GoFunction>& getFunctions() const { return functions_; }
    const std::vector<GoStruct>& getStructs() const { return structs_; }
    const std::vector<GoInterface>& getInterfaces() const { return interfaces_; }
    const std::map<std::string, std::vector<std::string>>& getFunctionCalls() const { return functionCalls_; }
    const std::string& getPackageName() const { return packageName_; }
    
    // Helper functions
    std::string getFunctionSignature(const GoFunction& func) const;
    std::vector<std::string> getCalledFunctions(const std::string& functionName) const;
    std::vector<std::string> getCallingFunctions(const std::string& functionName) const;
    
    // Clear parsing results
    void clear();

private:
    std::vector<GoFunction> functions_;
    std::vector<GoStruct> structs_;
    std::vector<GoInterface> interfaces_;
    std::map<std::string, std::vector<std::string>> functionCalls_;  // function -> list of called functions
    std::string packageName_;
    
    // Parsing helper functions
    void parsePackage(const std::string& content);
    void parseFunctions(const std::string& content);
    void parseStructs(const std::string& content);
    void parseInterfaces(const std::string& content);
    void parseFunctionCalls(const std::string& content);
    
    GoFunction parseFunctionDefinition(const std::vector<std::string>& lines, int startLine);
    std::vector<GoParameter> parseParameters(const std::string& paramStr);
    std::vector<std::string> parseReturnTypes(const std::string& returnStr);
    std::string removeComments(const std::string& content);
    std::string normalizeWhitespace(const std::string& text);
    std::vector<std::string> splitLines(const std::string& content);
    std::string extractComment(const std::vector<std::string>& lines, int startLine);
    bool isBlankOrComment(const std::string& line);
    bool isExported(const std::string& name);
    
    // Regular expression patterns
    QRegExp packagePattern_;
    QRegExp functionPattern_;
    QRegExp methodPattern_;
    QRegExp structPattern_;
    QRegExp interfacePattern_;
    QRegExp functionCallPattern_;
    QRegExp parameterPattern_;
};

#endif // GO_PARSER_H 