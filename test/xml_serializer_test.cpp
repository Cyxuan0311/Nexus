#include <gtest/gtest.h>
#include "xml_serializer.h"
#include "xml_parser.h"

class XmlSerializerTest : public ::testing::Test {
protected:
    XmlSerializer serializer_;
    XmlParser parser_;
};

TEST_F(XmlSerializerTest, SerializeSimpleXml) {
    std::string xml = "<root>Hello World</root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    
    std::string serialized = serializer_.serializeToXml(node);
    EXPECT_FALSE(serialized.empty());
    EXPECT_NE(serialized.find("<root>"), std::string::npos);
    EXPECT_NE(serialized.find("Hello World"), std::string::npos);
    EXPECT_NE(serialized.find("</root>"), std::string::npos);
}

TEST_F(XmlSerializerTest, SerializeToJson) {
    std::string xml = "<root id=\"1\">Content</root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    
    std::string json = serializer_.serializeToJson(node);
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json.find("@name"), std::string::npos);
    EXPECT_NE(json.find("root"), std::string::npos);
    EXPECT_NE(json.find("@attributes"), std::string::npos);
    EXPECT_NE(json.find("id"), std::string::npos);
}

TEST_F(XmlSerializerTest, SerializeToYaml) {
    std::string xml = "<root><child>Value</child></root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    
    std::string yaml = serializer_.serializeToYaml(node);
    EXPECT_FALSE(yaml.empty());
    EXPECT_NE(yaml.find("root:"), std::string::npos);
    EXPECT_NE(yaml.find("child:"), std::string::npos);
}

TEST_F(XmlSerializerTest, SerializeToCsv) {
    std::string xml = "<table><row><name>John</name><age>30</age></row></table>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    
    std::string csv = serializer_.serializeToCsv(node);
    EXPECT_FALSE(csv.empty());
    // CSV should contain headers and data
    EXPECT_NE(csv.find("name"), std::string::npos);
    EXPECT_NE(csv.find("age"), std::string::npos);
}

TEST_F(XmlSerializerTest, OutputStyles) {
    std::string xml = "<root><child>Value</child></root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    
    // Test different output styles
    std::string pretty = serializer_.serializeToXml(node, XmlSerializer::OutputStyle::Pretty);
    std::string compact = serializer_.serializeToXml(node, XmlSerializer::OutputStyle::Compact);
    
    EXPECT_FALSE(pretty.empty());
    EXPECT_FALSE(compact.empty());
    
    // Pretty should have more whitespace/newlines
    EXPECT_GT(pretty.length(), compact.length());
}

TEST_F(XmlSerializerTest, FormatConversion) {
    std::string xml = "<root id=\"1\">Content</root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    
    // Test format conversion
    std::string json = serializer_.convertToJson(node);
    std::string yaml = serializer_.convertToYaml(node);
    
    EXPECT_FALSE(json.empty());
    EXPECT_FALSE(yaml.empty());
    
    // Both should contain the root element name
    EXPECT_NE(json.find("root"), std::string::npos);
    EXPECT_NE(yaml.find("root"), std::string::npos);
} 