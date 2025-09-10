#ifndef XML_NODE_H
#define XML_NODE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <QMetaType>

class XmlNode : public std::enable_shared_from_this<XmlNode> {
public:
    enum class NodeType {
        Element,
        Text,
        Comment,
        ProcessingInstruction,
        Document
    };

    XmlNode(const std::string& name = "", NodeType type = NodeType::Element);
    ~XmlNode() = default;

    // Getters
    const std::string& getName() const { return name_; }
    const std::string& getValue() const { return value_; }
    NodeType getType() const { return type_; }
    const std::map<std::string, std::string>& getAttributes() const { return attributes_; }
    const std::vector<std::shared_ptr<XmlNode>>& getChildren() const { return children_; }
    std::shared_ptr<XmlNode> getParent() const { return parent_.lock(); }

    // Setters
    void setName(const std::string& name) { name_ = name; }
    void setValue(const std::string& value) { value_ = value; }
    void setType(NodeType type) { type_ = type; }
    void setParent(std::shared_ptr<XmlNode> parent) { parent_ = parent; }

    // Attribute management
    void addAttribute(const std::string& key, const std::string& value);
    std::string getAttribute(const std::string& key) const;
    bool hasAttribute(const std::string& key) const;

    // Child management
    void addChild(std::shared_ptr<XmlNode> child);
    void removeChild(std::shared_ptr<XmlNode> child);
    std::shared_ptr<XmlNode> findChild(const std::string& name) const;

    // Utility methods
    bool isLeaf() const { return children_.empty(); }
    int getDepth() const;
    std::string getPath() const;

private:
    std::string name_;
    std::string value_;
    NodeType type_;
    std::map<std::string, std::string> attributes_;
    std::vector<std::shared_ptr<XmlNode>> children_;
    std::weak_ptr<XmlNode> parent_;
};

Q_DECLARE_METATYPE(std::shared_ptr<XmlNode>)

#endif // XML_NODE_H 