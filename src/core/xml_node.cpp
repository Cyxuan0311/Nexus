#include <sstream>
#include <algorithm>
#include "xml_node.h"

XmlNode::XmlNode(const std::string& name, NodeType type)
    : name_(name), type_(type) {
}

void XmlNode::addAttribute(const std::string& key, const std::string& value) {
    attributes_[key] = value;
}

std::string XmlNode::getAttribute(const std::string& key) const {
    auto it = attributes_.find(key);
    return (it != attributes_.end()) ? it->second : "";
}

bool XmlNode::hasAttribute(const std::string& key) const {
    return attributes_.find(key) != attributes_.end();
}

void XmlNode::addChild(std::shared_ptr<XmlNode> child) {
    if (child) {
        child->setParent(shared_from_this());
        children_.push_back(child);
    }
}

void XmlNode::removeChild(std::shared_ptr<XmlNode> child) {
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        children_.erase(it);
    }
}

std::shared_ptr<XmlNode> XmlNode::findChild(const std::string& name) const {
    for (const auto& child : children_) {
        if (child->getName() == name) {
            return child;
        }
    }
    return nullptr;
}

int XmlNode::getDepth() const {
    int depth = 0;
    auto current = parent_.lock();
    while (current) {
        depth++;
        current = current->getParent();
    }
    return depth;
}

std::string XmlNode::getPath() const {
    std::vector<std::string> pathParts;
    auto current = shared_from_this();
    
    while (current) {
        pathParts.insert(pathParts.begin(), current->getName());
        current = current->getParent();
    }
    
    std::stringstream ss;
    for (size_t i = 0; i < pathParts.size(); ++i) {
        if (i > 0) ss << "/";
        ss << pathParts[i];
    }
    return ss.str();
} 