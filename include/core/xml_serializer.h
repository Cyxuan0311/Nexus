#ifndef XML_SERIALIZER_H
#define XML_SERIALIZER_H

#include "xml_node.h"
#include <string>
#include <memory>
#include <map>

class XmlSerializer {
public:
    enum class Format {
        XML,
        JSON,
        YAML,
        CSV
    };
    
    enum class OutputStyle {
        Compact,    // 紧凑格式
        Pretty,     // 美化格式
        Minified    // 最小化格式
    };

    XmlSerializer();
    ~XmlSerializer() = default;

    // XML序列化
    std::string serializeToXml(const std::shared_ptr<XmlNode>& node, 
                              OutputStyle style = OutputStyle::Pretty) const;
    
    // JSON序列化
    std::string serializeToJson(const std::shared_ptr<XmlNode>& node,
                               OutputStyle style = OutputStyle::Pretty) const;
    
    // YAML序列化
    std::string serializeToYaml(const std::shared_ptr<XmlNode>& node,
                               OutputStyle style = OutputStyle::Pretty) const;
    
    // CSV序列化 (适用于表格数据)
    std::string serializeToCsv(const std::shared_ptr<XmlNode>& node) const;
    
    // 通用序列化接口
    std::string serialize(const std::shared_ptr<XmlNode>& node,
                         Format format = Format::XML,
                         OutputStyle style = OutputStyle::Pretty) const;

    // 反序列化
    std::shared_ptr<XmlNode> deserializeFromXml(const std::string& content);
    std::shared_ptr<XmlNode> deserializeFromJson(const std::string& content);
    std::shared_ptr<XmlNode> deserializeFromYaml(const std::string& content);
    std::shared_ptr<XmlNode> deserializeFromCsv(const std::string& content);

    // 验证功能
    bool validateXml(const std::string& xmlContent) const;
    bool validateAgainstSchema(const std::string& xmlContent, 
                              const std::string& schemaPath) const;

    // 转换功能
    std::shared_ptr<XmlNode> convertFromJson(const std::string& jsonContent);
    std::shared_ptr<XmlNode> convertFromYaml(const std::string& yamlContent);
    std::string convertToJson(const std::shared_ptr<XmlNode>& node) const;
    std::string convertToYaml(const std::shared_ptr<XmlNode>& node) const;

private:
    // 内部辅助方法
    std::string _serializeXmlNode(const std::shared_ptr<XmlNode>& node, 
                                 int indent = 0, 
                                 OutputStyle style = OutputStyle::Pretty) const;
    std::string _serializeJsonNode(const std::shared_ptr<XmlNode>& node,
                                  int indent = 0,
                                  OutputStyle style = OutputStyle::Pretty) const;
    std::string _serializeYamlNode(const std::shared_ptr<XmlNode>& node,
                                  int indent = 0,
                                  OutputStyle style = OutputStyle::Pretty) const;
    std::string _serializeCsvNode(const std::shared_ptr<XmlNode>& node) const;
    
    std::string _getIndent(int level, OutputStyle style) const;
    std::string _escapeXmlString(const std::string& str) const;
    std::string _escapeJsonString(const std::string& str) const;
    std::string _escapeYamlString(const std::string& str) const;
    
    // 配置选项
    struct SerializationConfig {
        bool includeComments = true;
        bool includeProcessingInstructions = true;
        bool preserveWhitespace = false;
        std::string rootElementName = "root";
        std::string textElementName = "text";
        std::string attributePrefix = "@";
    };
    
    SerializationConfig config_;
};

#endif // XML_SERIALIZER_H 