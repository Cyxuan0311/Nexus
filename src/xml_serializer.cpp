#include "xml_serializer.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>

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

std::shared_ptr<XmlNode> XmlSerializer::deserializeFromXml(const std::string& content) {
    // 使用现有的XmlParser
    // 这里需要修改XmlParser以支持从字符串解析
    return nullptr; // TODO: 实现
}

std::shared_ptr<XmlNode> XmlSerializer::deserializeFromJson(const std::string& content) {
    // TODO: 实现JSON反序列化
    return nullptr;
}

std::shared_ptr<XmlNode> XmlSerializer::deserializeFromYaml(const std::string& content) {
    // TODO: 实现YAML反序列化
    return nullptr;
}

std::shared_ptr<XmlNode> XmlSerializer::deserializeFromCsv(const std::string& content) {
    // TODO: 实现CSV反序列化
    return nullptr;
}

bool XmlSerializer::validateXml(const std::string& xmlContent) const {
    try {
        // 尝试解析XML，如果成功则有效
        auto node = deserializeFromXml(xmlContent);
        return node != nullptr;
    } catch (...) {
        return false;
    }
}

bool XmlSerializer::validateAgainstSchema(const std::string& xmlContent, 
                                         const std::string& schemaPath) const {
    // TODO: 实现XML Schema验证
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

// 私有方法实现

std::string XmlSerializer::_serializeXmlNode(const std::shared_ptr<XmlNode>& node, 
                                            int indent, 
                                            OutputStyle style) const {
    if (!node) return "";
    
    std::stringstream ss;
    std::string indentStr = _getIndent(indent, style);
    
    switch (node->getType()) {
        case XmlNode::NodeType::Element: {
            ss << indentStr << "<" << node->getName();
            
            // 添加属性
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
                
                // 处理文本子节点
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
            
            // 添加元素名称
            ss << childIndentStr << "\"@name\": \"" << _escapeJsonString(node->getName()) << "\"";
            
            // 添加属性
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
            
            // 添加文本内容
            if (!node->getValue().empty()) {
                ss << ",\n" << childIndentStr << "\"@text\": \"" 
                   << _escapeJsonString(node->getValue()) << "\"";
            }
            
            // 添加子元素
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
            
            // 添加属性
            if (!node->getAttributes().empty()) {
                ss << indentStr << "  attributes:\n";
                for (const auto& attr : node->getAttributes()) {
                    ss << indentStr << "    " << attr.first << ": \"" 
                       << _escapeYamlString(attr.second) << "\"\n";
                }
            }
            
            // 添加文本内容
            if (!node->getValue().empty()) {
                ss << indentStr << "  text: \"" << _escapeYamlString(node->getValue()) << "\"\n";
            }
            
            // 添加子元素
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
    
    // 简单的CSV序列化，适用于表格数据
    if (node->getType() == XmlNode::NodeType::Element) {
        // 假设这是一个表格结构
        std::vector<std::string> headers;
        std::vector<std::vector<std::string>> rows;
        
        // 提取表头
        for (const auto& child : node->getChildren()) {
            if (child->getType() == XmlNode::NodeType::Element) {
                headers.push_back(child->getName());
            }
        }
        
        // 写入CSV头
        for (size_t i = 0; i < headers.size(); ++i) {
            if (i > 0) ss << ",";
            ss << "\"" << headers[i] << "\"";
        }
        ss << "\n";
        
        // 写入数据行
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
    
    // 替换XML特殊字符
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
    
    // 替换JSON特殊字符
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
    
    // 替换YAML特殊字符
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