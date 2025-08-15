#include <gtest/gtest.h>
#include "xml_parser.h"
#include "xml_node.h"

class XmlParserTest : public ::testing::Test {
protected:
    XmlParser parser_;
};

TEST_F(XmlParserTest, ParseSimpleElement) {
    std::string xml = "<root>Hello World</root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getName(), "root");
    EXPECT_EQ(node->getType(), XmlNode::NodeType::Element);
    EXPECT_EQ(node->getChildren().size(), 1);
    
    auto textNode = node->getChildren()[0];
    EXPECT_EQ(textNode->getType(), XmlNode::NodeType::Text);
    EXPECT_EQ(textNode->getValue(), "Hello World");
}

TEST_F(XmlParserTest, ParseElementWithAttributes) {
    std::string xml = "<root id=\"1\" name=\"test\">Content</root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getName(), "root");
    EXPECT_EQ(node->getAttributes().size(), 2);
    EXPECT_EQ(node->getAttribute("id"), "1");
    EXPECT_EQ(node->getAttribute("name"), "test");
}

TEST_F(XmlParserTest, ParseNestedElements) {
    std::string xml = "<root><child><grandchild>Text</grandchild></child></root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getName(), "root");
    EXPECT_EQ(node->getChildren().size(), 1);
    
    auto child = node->getChildren()[0];
    EXPECT_EQ(child->getName(), "child");
    EXPECT_EQ(child->getChildren().size(), 1);
    
    auto grandchild = child->getChildren()[0];
    EXPECT_EQ(grandchild->getName(), "grandchild");
    EXPECT_EQ(grandchild->getChildren().size(), 1);
    
    auto textNode = grandchild->getChildren()[0];
    EXPECT_EQ(textNode->getType(), XmlNode::NodeType::Text);
    EXPECT_EQ(textNode->getValue(), "Text");
}

TEST_F(XmlParserTest, ParseSelfClosingElement) {
    std::string xml = "<root><selfclosing /></root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getName(), "root");
    EXPECT_EQ(node->getChildren().size(), 1);
    
    auto selfClosing = node->getChildren()[0];
    EXPECT_EQ(selfClosing->getName(), "selfclosing");
    EXPECT_TRUE(selfClosing->isLeaf());
}

TEST_F(XmlParserTest, ParseElementWithMixedContent) {
    std::string xml = "<root>Text before<child>Child text</child>Text after</root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getName(), "root");
    EXPECT_EQ(node->getChildren().size(), 3);
    
    // Check text nodes
    EXPECT_EQ(node->getChildren()[0]->getType(), XmlNode::NodeType::Text);
    EXPECT_EQ(node->getChildren()[0]->getValue(), "Text before");
    
    EXPECT_EQ(node->getChildren()[1]->getType(), XmlNode::NodeType::Element);
    EXPECT_EQ(node->getChildren()[1]->getName(), "child");
    
    EXPECT_EQ(node->getChildren()[2]->getType(), XmlNode::NodeType::Text);
    EXPECT_EQ(node->getChildren()[2]->getValue(), "Text after");
}

TEST_F(XmlParserTest, ParseXmlWithDeclaration) {
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><root>Content</root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getName(), "root");
    EXPECT_EQ(node->getChildren().size(), 1);
    EXPECT_EQ(node->getChildren()[0]->getValue(), "Content");
}

TEST_F(XmlParserTest, ParseXmlWithComments) {
    std::string xml = "<root><!-- This is a comment --><child>Content</child></root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getName(), "root");
    EXPECT_EQ(node->getChildren().size(), 2);
    
    auto commentNode = node->getChildren()[0];
    EXPECT_EQ(commentNode->getType(), XmlNode::NodeType::Comment);
    EXPECT_EQ(commentNode->getValue(), " This is a comment ");
    
    auto childNode = node->getChildren()[1];
    EXPECT_EQ(childNode->getType(), XmlNode::NodeType::Element);
    EXPECT_EQ(childNode->getName(), "child");
}

TEST_F(XmlParserTest, ParseXmlWithEscapedEntities) {
    std::string xml = "<root>&lt;tag&gt; &amp; &quot;text&quot;</root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getName(), "root");
    EXPECT_EQ(node->getChildren().size(), 1);
    
    auto textNode = node->getChildren()[0];
    EXPECT_EQ(textNode->getType(), XmlNode::NodeType::Text);
    EXPECT_EQ(textNode->getValue(), "<tag> & \"text\"");
}

TEST_F(XmlParserTest, ParseInvalidXml) {
    std::string xml = "<root><unclosed>";
    auto node = parser_.parseString(xml);
    
    EXPECT_EQ(node, nullptr);
    EXPECT_TRUE(parser_.hasError());
    EXPECT_FALSE(parser_.getErrorMessage().empty());
}

TEST_F(XmlParserTest, ParseEmptyString) {
    std::string xml = "";
    auto node = parser_.parseString(xml);
    
    EXPECT_EQ(node, nullptr);
    EXPECT_TRUE(parser_.hasError());
}

TEST_F(XmlParserTest, NodePath) {
    std::string xml = "<root><parent><child>Text</child></parent></root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    auto parent = node->getChildren()[0];
    auto child = parent->getChildren()[0];
    
    EXPECT_EQ(node->getPath(), "root");
    EXPECT_EQ(parent->getPath(), "root/parent");
    EXPECT_EQ(child->getPath(), "root/parent/child");
}

TEST_F(XmlParserTest, NodeDepth) {
    std::string xml = "<root><level1><level2><level3>Text</level3></level2></level1></root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    auto level1 = node->getChildren()[0];
    auto level2 = level1->getChildren()[0];
    auto level3 = level2->getChildren()[0];
    
    EXPECT_EQ(node->getDepth(), 0);
    EXPECT_EQ(level1->getDepth(), 1);
    EXPECT_EQ(level2->getDepth(), 2);
    EXPECT_EQ(level3->getDepth(), 3);
}

TEST_F(XmlParserTest, FindChild) {
    std::string xml = "<root><child1>Text1</child1><child2>Text2</child2></root>";
    auto node = parser_.parseString(xml);
    
    ASSERT_NE(node, nullptr);
    
    auto foundChild1 = node->findChild("child1");
    ASSERT_NE(foundChild1, nullptr);
    EXPECT_EQ(foundChild1->getName(), "child1");
    
    auto foundChild2 = node->findChild("child2");
    ASSERT_NE(foundChild2, nullptr);
    EXPECT_EQ(foundChild2->getName(), "child2");
    
    auto notFound = node->findChild("nonexistent");
    EXPECT_EQ(notFound, nullptr);
} 