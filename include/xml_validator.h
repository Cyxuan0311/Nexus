#ifndef XML_VALIDATOR_H
#define XML_VALIDATOR_H

#include "xml_node.h"
#include <string>
#include <vector>
#include <memory>

struct ValidationError {
    enum class Type {
        Syntax,
        Schema,
        Namespace,
        DTD
    };
    
    Type type;
    std::string message;
    int line;
    int column;
    std::string element;
};

class XmlValidator {
public:
    XmlValidator();
    ~XmlValidator() = default;
    
    // Basic XML validation
    bool validateXml(const std::string& xmlContent);
    
    // Schema validation
    bool validateAgainstSchema(const std::string& xmlContent, 
                              const std::string& schemaPath);
    
    // DTD validation
    bool validateAgainstDTD(const std::string& xmlContent, 
                           const std::string& dtdPath);
    
    // Namespace validation
    bool validateNamespaces(const std::string& xmlContent);
    
    // Get validation errors
    const std::vector<ValidationError>& getErrors() const { return errors_; }
    void clearErrors() { errors_.clear(); }
    
    // Get validation warnings
    const std::vector<ValidationError>& getWarnings() const { return warnings_; }
    void clearWarnings() { warnings_.clear(); }

private:
    std::vector<ValidationError> errors_;
    std::vector<ValidationError> warnings_;
    
    void addError(ValidationError::Type type, const std::string& message, 
                  int line = 0, int column = 0, const std::string& element = "");
    void addWarning(ValidationError::Type type, const std::string& message, 
                   int line = 0, int column = 0, const std::string& element = "");
    
    bool validateXmlStructure(const std::shared_ptr<XmlNode>& node);
    bool validateXmlNamespaces(const std::shared_ptr<XmlNode>& node);
};

#endif // XML_VALIDATOR_H 