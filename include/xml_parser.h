#ifndef XML_PARSER_H
#define XML_PARSER_H

#include "xml_node.h"
#include <string>
#include <memory>
#include <fstream>

class XmlParser {
public:
    XmlParser();
    ~XmlParser() = default;

    // Main parsing methods
    std::shared_ptr<XmlNode> parseFile(const std::string& filename);
    std::shared_ptr<XmlNode> parseString(const std::string& xmlContent);

    // Error handling
    bool hasError() const { return !errorMessage_.empty(); }
    const std::string& getErrorMessage() const { return errorMessage_; }
    void clearError() { errorMessage_.clear(); }

    // Utility methods
    std::string nodeToString(const std::shared_ptr<XmlNode>& node, int indent = 0) const;

private:
    std::string errorMessage_;
    
    // Helper methods for parsing
    std::shared_ptr<XmlNode> parseElement(std::istream& stream);
    std::string parseTagName(std::istream& stream);
    std::map<std::string, std::string> parseAttributes(std::istream& stream);
    std::string parseText(std::istream& stream);
    std::string parseComment(std::istream& stream);
    std::string parseProcessingInstruction(std::istream& stream);
    
    // Utility methods
    void skipWhitespace(std::istream& stream);
    char peekNextChar(std::istream& stream);
    char getNextChar(std::istream& stream);
    bool isWhitespace(char c);
    std::string unescapeXml(const std::string& text);
    std::string escapeXml(const std::string& text);
};

#endif // XML_PARSER_H 