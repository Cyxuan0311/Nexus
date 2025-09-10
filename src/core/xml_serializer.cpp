#include "xml_serializer.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>
#include <QtGlobal>

XmlSerializer::XmlSerializer() {
}

std::string XmlSerializer::serializeToXml(const std::shared_ptr<XmlNode>& node, 
                                         OutputStyle style) const {
    return _serializeXmlNode(node, 0, style);
}

std::string XmlSerializer::serializeToJson(const std::shared_ptr<XmlNode>& node,
                                          OutputStyle style) const {
    return _serializeJsonNode(node, 0, style);
}

std::string XmlSerializer::serializeToYaml(const std::shared_ptr<XmlNode>& node,
                                          OutputStyle style) const {
    return _serializeYamlNode(node, 0, style);
}

std::string XmlSerializer::serializeToCsv(const std::shared_ptr<XmlNode>& node) const {
    return _serializeCsvNode(node);
}

std::string XmlSerializer::serialize(const std::shared_ptr<XmlNode>& node,
                                    Format format,
                                    OutputStyle style) const {
    switch (format) {
        case Format::XML:
            return serializeToXml(node, style);
        case Format::JSON:
            return serializeToJson(node, style);
        case Format::YAML:
            return serializeToYaml(node, style);
        case Format::CSV:
            return serializeToCsv(node);
        default:
            return serializeToXml(node, style);
    }
}

std::shared_ptr<XmlNode> XmlSerializer::deserializeFromXml(const std::string& content) const {
    Q_UNUSED(content);
    // Use existing XmlParser
    // Need to modify XmlParser to support parsing from string
    return nullptr; // TODO: implement
}

std::shared_ptr<XmlNode> XmlSerializer::deserializeFromJson(const std::string& content) const {
    Q_UNUSED(content);
    // TODO: implement JSON deserialization
    return nullptr;
}

std::shared_ptr<XmlNode> XmlSerializer::deserializeFromYaml(const std::string& content) const {
    Q_UNUSED(content);
    // TODO: implement YAML deserialization
    return nullptr;
}

std::shared_ptr<XmlNode> XmlSerializer::deserializeFromCsv(const std::string& content) const {
    Q_UNUSED(content);
    // TODO: implement CSV deserialization
    return nullptr;
}

bool XmlSerializer::validateXml(const std::string& xmlContent) const {
    try {
        // Try to parse XML, if successful then valid
        auto node = deserializeFromXml(xmlContent);
        return node != nullptr;
    } catch (...) {
        return false;
    }
}

bool XmlSerializer::validateAgainstSchema(const std::string& xmlContent, 
                                         const std::string& schemaPath) const {
    Q_UNUSED(xmlContent);
    Q_UNUSED(schemaPath);
    // TODO: implement XML Schema validation
    return false;
}

std::shared_ptr<XmlNode> XmlSerializer::convertFromJson(const std::string& jsonContent) {
    return deserializeFromJson(jsonContent);
}

std::shared_ptr<XmlNode> XmlSerializer::convertFromYaml(const std::string& yamlContent) {
    return deserializeFromYaml(yamlContent);
}

std::string XmlSerializer::convertToJson(const std::shared_ptr<XmlNode>& node) const {
    return serializeToJson(node);
}

std::string XmlSerializer::convertToYaml(const std::shared_ptr<XmlNode>& node) const {
    return serializeToYaml(node);
}

// Private method implementations

std::string XmlSerializer::_serializeXmlNode(const std::shared_ptr<XmlNode>& node, 
                                            int indent, 
                                            OutputStyle style) const {
    if (!node) return "";
    
    std::stringstream ss;
    std::string indentStr = _getIndent(indent, style);
    
    switch (node->getType()) {
        case XmlNode::NodeType::Element: {
            ss << indentStr << "<" << node->getName();
            
            // Add attributes
            for (const auto& attr : node->getAttributes()) {
                ss << " " << attr.first << "=\"" << _escapeXmlString(attr.second) << "\"";
            }
            
            if (node->isLeaf() && node->getValue().empty()) {
                ss << " />";
                if (style != OutputStyle::Compact) ss << "\n";
            } else {
                ss << ">";
                
                if (!node->getValue().empty()) {
                    ss << _escapeXmlString(node->getValue());
                }
                
                bool hasChildren = false;
                for (const auto& child : node->getChildren()) {
                    if (child->getType() == XmlNode::NodeType::Element) {
                        if (!hasChildren && style != OutputStyle::Compact) {
                            ss << "\n";
                        }
                        ss << _serializeXmlNode(child, indent + 1, style);
                        hasChildren = true;
                    }
                }
                
                // Handle text child nodes
                for (const auto& child : node->getChildren()) {
                    if (child->getType() == XmlNode::NodeType::Text) {
                        ss << _escapeXmlString(child->getValue());
                    }
                }
                
                if (hasChildren && style != OutputStyle::Compact) {
                    ss << indentStr;
                }
                ss << "</" << node->getName() << ">";
                if (style != OutputStyle::Compact) ss << "\n";
            }
            break;
        }
        case XmlNode::NodeType::Text:
            ss << _escapeXmlString(node->getValue());
            break;
        case XmlNode::NodeType::Comment:
            if (config_.includeComments) {
                ss << indentStr << "<!-- " << node->getValue() << " -->";
                if (style != OutputStyle::Compact) ss << "\n";
            }
            break;
        default:
            break;
    }
    
    return ss.str();
}

std::string XmlSerializer::_serializeJsonNode(const std::shared_ptr<XmlNode>& node,
                                             int indent,
                                             OutputStyle style) const {
    if (!node) return "null";
    
    std::stringstream ss;
    std::string indentStr = _getIndent(indent, style);
    std::string childIndentStr = _getIndent(indent + 1, style);
    
    switch (node->getType()) {
        case XmlNode::NodeType::Element: {
            ss << indentStr << "{\n";
            
            // Add element name
            ss << childIndentStr << "\"@name\": \"" << _escapeJsonString(node->getName()) << "\"";
            
            // Add attributes
            if (!node->getAttributes().empty()) {
                ss << ",\n" << childIndentStr << "\"@attributes\": {\n";
                std::string attrIndentStr = _getIndent(indent + 2, style);
                bool first = true;
                for (const auto& attr : node->getAttributes()) {
                    if (!first) ss << ",\n";
                    ss << attrIndentStr << "\"" << _escapeJsonString(attr.first) << "\": \"" 
                       << _escapeJsonString(attr.second) << "\"";
                    first = false;
                }
                ss << "\n" << childIndentStr << "}";
            }
            
            // Add text content
            if (!node->getValue().empty()) {
                ss << ",\n" << childIndentStr << "\"@text\": \"" 
                   << _escapeJsonString(node->getValue()) << "\"";
            }
            
            // Add child elements
            std::vector<std::shared_ptr<XmlNode>> elementChildren;
            std::vector<std::shared_ptr<XmlNode>> textChildren;
            
            for (const auto& child : node->getChildren()) {
                if (child->getType() == XmlNode::NodeType::Element) {
                    elementChildren.push_back(child);
                } else if (child->getType() == XmlNode::NodeType::Text) {
                    textChildren.push_back(child);
                }
            }
            
            if (!elementChildren.empty()) {
                ss << ",\n" << childIndentStr << "\"@children\": [\n";
                for (size_t i = 0; i < elementChildren.size(); ++i) {
                    if (i > 0) ss << ",\n";
                    ss << _serializeJsonNode(elementChildren[i], indent + 2, style);
                }
                ss << "\n" << childIndentStr << "]";
            }
            
            ss << "\n" << indentStr << "}";
            break;
        }
        case XmlNode::NodeType::Text:
            ss << "\"" << _escapeJsonString(node->getValue()) << "\"";
            break;
        default:
            ss << "null";
            break;
    }
    
    return ss.str();
}

std::string XmlSerializer::_serializeYamlNode(const std::shared_ptr<XmlNode>& node,
                                             int indent,
                                             OutputStyle style) const {
    if (!node) return "";
    
    std::stringstream ss;
    std::string indentStr = _getIndent(indent, style);
    
    switch (node->getType()) {
        case XmlNode::NodeType::Element: {
            ss << indentStr << node->getName() << ":\n";
            
            // Add attributes
            if (!node->getAttributes().empty()) {
                ss << indentStr << "  attributes:\n";
                for (const auto& attr : node->getAttributes()) {
                    ss << indentStr << "    " << attr.first << ": \"" 
                       << _escapeYamlString(attr.second) << "\"\n";
                }
            }
            
            // Add text content
            if (!node->getValue().empty()) {
                ss << indentStr << "  text: \"" << _escapeYamlString(node->getValue()) << "\"\n";
            }
            
            // Add child elements
            for (const auto& child : node->getChildren()) {
                if (child->getType() == XmlNode::NodeType::Element) {
                    ss << _serializeYamlNode(child, indent + 2, style);
                }
            }
            break;
        }
        case XmlNode::NodeType::Text:
            ss << indentStr << "- \"" << _escapeYamlString(node->getValue()) << "\"\n";
            break;
        default:
            break;
    }
    
    return ss.str();
}

std::string XmlSerializer::_serializeCsvNode(const std::shared_ptr<XmlNode>& node) const {
    if (!node) return "";
    
    std::stringstream ss;
    
    // Simple CSV serialization, suitable for tabular data
    if (node->getType() == XmlNode::NodeType::Element) {
        // Assume this is a table structure
        std::vector<std::string> headers;
        std::vector<std::vector<std::string>> rows;
        
        // Extract table headers
        for (const auto& child : node->getChildren()) {
            if (child->getType() == XmlNode::NodeType::Element) {
                headers.push_back(child->getName());
            }
        }
        
        // Write CSV header
        for (size_t i = 0; i < headers.size(); ++i) {
            if (i > 0) ss << ",";
            ss << "\"" << headers[i] << "\"";
        }
        ss << "\n";
        
        // Write data rows
        for (const auto& child : node->getChildren()) {
            if (child->getType() == XmlNode::NodeType::Element) {
                ss << "\"" << child->getValue() << "\"";
                if (&child != &node->getChildren().back()) {
                    ss << ",";
                }
            }
        }
    }
    
    return ss.str();
}

std::string XmlSerializer::_getIndent(int level, OutputStyle style) const {
    if (style == OutputStyle::Compact) return "";
    
    std::string indent;
    int spaces = (style == OutputStyle::Pretty) ? 2 : 0;
    for (int i = 0; i < level * spaces; ++i) {
        indent += " ";
    }
    return indent;
}

std::string XmlSerializer::_escapeXmlString(const std::string& str) const {
    std::string result = str;
    
    // Replace XML special characters
    std::map<std::string, std::string> replacements = {
        {"&", "&amp;"},
        {"<", "&lt;"},
        {">", "&gt;"},
        {"\"", "&quot;"},
        {"'", "&apos;"}
    };
    
    for (const auto& replacement : replacements) {
        size_t pos = 0;
        while ((pos = result.find(replacement.first, pos)) != std::string::npos) {
            result.replace(pos, replacement.first.length(), replacement.second);
            pos += replacement.second.length();
        }
    }
    
    return result;
}

std::string XmlSerializer::_escapeJsonString(const std::string& str) const {
    std::string result = str;
    
    // Replace JSON special characters
    std::map<std::string, std::string> replacements = {
        {"\\", "\\\\"},
        {"\"", "\\\""},
        {"\n", "\\n"},
        {"\r", "\\r"},
        {"\t", "\\t"}
    };
    
    for (const auto& replacement : replacements) {
        size_t pos = 0;
        while ((pos = result.find(replacement.first, pos)) != std::string::npos) {
            result.replace(pos, replacement.first.length(), replacement.second);
            pos += replacement.second.length();
        }
    }
    
    return result;
}

std::string XmlSerializer::_escapeYamlString(const std::string& str) const {
    std::string result = str;
    
    // Replace YAML special characters
    std::map<std::string, std::string> replacements = {
        {"\\", "\\\\"},
        {"\"", "\\\""},
        {"\n", "\\n"},
        {"\r", "\\r"},
        {"\t", "\\t"}
    };
    
    for (const auto& replacement : replacements) {
        size_t pos = 0;
        while ((pos = result.find(replacement.first, pos)) != std::string::npos) {
            result.replace(pos, replacement.first.length(), replacement.second);
            pos += replacement.second.length();
        }
    }
    
    return result;
} 