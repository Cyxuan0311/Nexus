#include "xml_parser.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <regex>

XmlParser::XmlParser() {
}

std::shared_ptr<XmlNode> XmlParser::parseFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        errorMessage_ = "Cannot open file: " + filename;
        return nullptr;
    }
    
    clearError();
    std::string content{std::istreambuf_iterator<char>(file),
                        std::istreambuf_iterator<char>()};
    return parseString(content);
}

std::shared_ptr<XmlNode> XmlParser::parseString(const std::string& xmlContent) {
    clearError();
    
    std::istringstream stream(xmlContent);
    skipWhitespace(stream);
    
    // Check for XML declaration
    if (peekNextChar(stream) == '<' && stream.peek() == '?') {
        parseProcessingInstruction(stream);
        skipWhitespace(stream);
    }
    
    // Parse root element
    if (peekNextChar(stream) == '<') {
        return parseElement(stream);
    }
    
    errorMessage_ = "No root element found";
    return nullptr;
}

std::shared_ptr<XmlNode> XmlParser::parseElement(std::istream& stream) {
    char c = getNextChar(stream);
    if (c != '<') {
        errorMessage_ = "Expected '<' at start of element";
        return nullptr;
    }
    
    // Check for closing tag
    if (peekNextChar(stream) == '/') {
        getNextChar(stream); // consume '/'
        std::string closingTag = parseTagName(stream);
        skipWhitespace(stream);
        if (getNextChar(stream) != '>') {
            errorMessage_ = "Expected '>' in closing tag";
            return nullptr;
        }
        return nullptr; // End of element
    }
    
    // Check for comment
    if (peekNextChar(stream) == '!') {
        getNextChar(stream); // consume '!'
        if (peekNextChar(stream) == '-' && stream.peek() == '-') {
            parseComment(stream);
            return parseElement(stream);
        }
    }
    
    // Parse tag name
    std::string tagName = parseTagName(stream);
    if (tagName.empty()) {
        errorMessage_ = "Invalid tag name";
        return nullptr;
    }
    
    auto node = std::make_shared<XmlNode>(tagName, XmlNode::NodeType::Element);
    
    // Parse attributes
    auto attributes = parseAttributes(stream);
    for (const auto& attr : attributes) {
        node->addAttribute(attr.first, attr.second);
    }
    
    skipWhitespace(stream);
    
    // Check for self-closing tag
    if (peekNextChar(stream) == '/') {
        getNextChar(stream); // consume '/'
        if (getNextChar(stream) != '>') {
            errorMessage_ = "Expected '>' after '/' in self-closing tag";
            return nullptr;
        }
        return node;
    }
    
    if (getNextChar(stream) != '>') {
        errorMessage_ = "Expected '>' after tag name and attributes";
        return nullptr;
    }
    
    // Parse children
    while (true) {
        skipWhitespace(stream);
        
        char nextChar = peekNextChar(stream);
        if (nextChar == '<') {
            if (stream.peek() == '/') {
                // Closing tag
                getNextChar(stream); // consume '<'
                getNextChar(stream); // consume '/'
                std::string closingTag = parseTagName(stream);
                if (closingTag != tagName) {
                    errorMessage_ = "Mismatched closing tag: expected " + tagName + ", got " + closingTag;
                    return nullptr;
                }
                skipWhitespace(stream);
                if (getNextChar(stream) != '>') {
                    errorMessage_ = "Expected '>' in closing tag";
                    return nullptr;
                }
                break;
            } else {
                // Child element
                auto childNode = parseElement(stream);
                if (childNode) {
                    node->addChild(childNode);
                }
            }
        } else if (nextChar == '\0') {
            errorMessage_ = "Unexpected end of file";
            return nullptr;
        } else {
            // Text content
            std::string text = parseText(stream);
            if (!text.empty()) {
                auto textNode = std::make_shared<XmlNode>("", XmlNode::NodeType::Text);
                textNode->setValue(text);
                node->addChild(textNode);
            }
        }
    }
    
    return node;
}

std::string XmlParser::parseTagName(std::istream& stream) {
    std::string name;
    char c;
    
    while ((c = peekNextChar(stream)) && (isalnum(c) || c == '_' || c == '-')) {
        name += getNextChar(stream);
    }
    
    return name;
}

std::map<std::string, std::string> XmlParser::parseAttributes(std::istream& stream) {
    std::map<std::string, std::string> attributes;
    
    while (true) {
        skipWhitespace(stream);
        
        char c = peekNextChar(stream);
        if (c == '>' || c == '/') {
            break;
        }
        
        // Parse attribute name
        std::string key = parseTagName(stream);
        if (key.empty()) {
            break;
        }
        
        skipWhitespace(stream);
        
        // Expect '='
        if (getNextChar(stream) != '=') {
            errorMessage_ = "Expected '=' after attribute name";
            break;
        }
        
        skipWhitespace(stream);
        
        // Parse attribute value
        char quote = getNextChar(stream);
        if (quote != '"' && quote != '\'') {
            errorMessage_ = "Expected quote around attribute value";
            break;
        }
        
        std::string value;
        while ((c = getNextChar(stream)) != quote && c != '\0') {
            value += c;
        }
        
        if (c == '\0') {
            errorMessage_ = "Unterminated attribute value";
            break;
        }
        
        attributes[key] = unescapeXml(value);
    }
    
    return attributes;
}

std::string XmlParser::parseText(std::istream& stream) {
    std::string text;
    char c;
    
    while ((c = peekNextChar(stream)) && c != '<' && c != '\0') {
        text += getNextChar(stream);
    }
    
    // Trim whitespace
    text.erase(0, text.find_first_not_of(" \t\n\r"));
    text.erase(text.find_last_not_of(" \t\n\r") + 1);
    
    return unescapeXml(text);
}

std::string XmlParser::parseComment(std::istream& stream) {
    std::string comment;
    
    // Already consumed '!' and first '-', now consume second '-'
    getNextChar(stream); // consume second '-'
    
    char c;
    while ((c = getNextChar(stream)) != '\0') {
        if (c == '-' && peekNextChar(stream) == '-' && stream.peek() == '>') {
            getNextChar(stream); // consume second '-'
            getNextChar(stream); // consume '>'
            break;
        }
        comment += c;
    }
    
    return comment;
}

std::string XmlParser::parseProcessingInstruction(std::istream& stream) {
    std::string pi;
    getNextChar(stream); // consume '?'
    
    char c;
    while ((c = getNextChar(stream)) != '\0') {
        if (c == '?' && peekNextChar(stream) == '>') {
            getNextChar(stream); // consume '>'
            break;
        }
        pi += c;
    }
    
    return pi;
}

void XmlParser::skipWhitespace(std::istream& stream) {
    char c;
    while ((c = peekNextChar(stream)) && isWhitespace(c)) {
        getNextChar(stream);
    }
}

char XmlParser::peekNextChar(std::istream& stream) {
    return stream.peek();
}

char XmlParser::getNextChar(std::istream& stream) {
    char c;
    stream.get(c);
    return c;
}

bool XmlParser::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

std::string XmlParser::unescapeXml(const std::string& text) {
    std::string result = text;
    
    // Replace XML entities
    std::map<std::string, std::string> entities = {
        {"&lt;", "<"},
        {"&gt;", ">"},
        {"&amp;", "&"},
        {"&quot;", "\""},
        {"&apos;", "'"}
    };
    
    for (const auto& entity : entities) {
        size_t pos = 0;
        while ((pos = result.find(entity.first, pos)) != std::string::npos) {
            result.replace(pos, entity.first.length(), entity.second);
            pos += entity.second.length();
        }
    }
    
    return result;
}

std::string XmlParser::escapeXml(const std::string& text) const {
    std::string result = text;
    
    // Replace special characters with XML entities
    std::map<std::string, std::string> entities = {
        {"<", "&lt;"},
        {">", "&gt;"},
        {"&", "&amp;"},
        {"\"", "&quot;"},
        {"'", "&apos;"}
    };
    
    for (const auto& entity : entities) {
        size_t pos = 0;
        while ((pos = result.find(entity.first, pos)) != std::string::npos) {
            result.replace(pos, entity.first.length(), entity.second);
            pos += entity.second.length();
        }
    }
    
    return result;
}

std::string XmlParser::nodeToString(const std::shared_ptr<XmlNode>& node, int indent) const {
    if (!node) return "";
    
    std::string result;
    std::string indentStr(indent * 2, ' ');
    
    switch (node->getType()) {
        case XmlNode::NodeType::Element: {
            result += indentStr + "<" + node->getName();
            
            // Add attributes
            for (const auto& attr : node->getAttributes()) {
                result += " " + attr.first + "=\"" + escapeXml(attr.second) + "\"";
            }
            
            if (node->isLeaf() && node->getValue().empty()) {
                result += " />\n";
            } else {
                result += ">";
                
                if (!node->getValue().empty()) {
                    result += escapeXml(node->getValue());
                }
                
                for (const auto& child : node->getChildren()) {
                    result += nodeToString(child, indent + 1);
                }
                
                result += "</" + node->getName() + ">\n";
            }
            break;
        }
        case XmlNode::NodeType::Text:
            result += indentStr + escapeXml(node->getValue()) + "\n";
            break;
        case XmlNode::NodeType::Comment:
            result += indentStr + "<!-- " + node->getValue() + " -->\n";
            break;
        default:
            break;
    }
    
    return result;
} 