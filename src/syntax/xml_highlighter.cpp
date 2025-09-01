#include "xml_highlighter.h"
#include <QTextDocument>

XmlHighlighter::XmlHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    
    // XML Tag format (green)
    tagFormat.setForeground(QColor("#4EC9B0"));
    tagFormat.setFontWeight(QFont::Bold);
    
    // Attribute format (blue)
    attributeFormat.setForeground(QColor("#569CD6"));
    attributeFormat.setFontWeight(QFont::Normal);
    
    // Value format (orange)
    valueFormat.setForeground(QColor("#CE9178"));
    valueFormat.setFontWeight(QFont::Normal);
    
    // Comment format (gray)
    commentFormat.setForeground(QColor("#6A9955"));
    commentFormat.setFontItalic(true);
    
    // Entity format (purple)
    entityFormat.setForeground(QColor("#C586C0"));
    entityFormat.setFontWeight(QFont::Bold);
    
    // CDATA format (yellow)
    cdataFormat.setForeground(QColor("#DCDCAA"));
    cdataFormat.setFontWeight(QFont::Normal);
    
    // Processing instruction format (cyan)
    processingInstructionFormat.setForeground(QColor("#4FC1FF"));
    processingInstructionFormat.setFontWeight(QFont::Normal);
    
    // Setup highlighting rules
    HighlightingRule rule;
    
    // XML Tags
    rule.pattern = QRegExp("<[^>]*>");
    rule.format = tagFormat;
    highlightingRules.append(rule);
    
    // XML Attributes
    rule.pattern = QRegExp("\\b\\w+\\s*=");
    rule.format = attributeFormat;
    highlightingRules.append(rule);
    
    // Attribute values
    rule.pattern = QRegExp("=\\s*\"[^\"]*\"");
    rule.format = valueFormat;
    highlightingRules.append(rule);
    
    // XML Entities
    rule.pattern = QRegExp("&[a-zA-Z]+;");
    rule.format = entityFormat;
    highlightingRules.append(rule);
    
    // CDATA sections
    rule.pattern = QRegExp("<!\\[CDATA\\[.*\\]\\]>");
    rule.format = cdataFormat;
    highlightingRules.append(rule);
    
    // Processing instructions
    rule.pattern = QRegExp("<\\?.*\\?>");
    rule.format = processingInstructionFormat;
    highlightingRules.append(rule);
    
    // Comments
    commentStartExpression = QRegExp("<!--");
    commentEndExpression = QRegExp("-->");
}

void XmlHighlighter::highlightBlock(const QString& text) {
    // Apply highlighting rules
    for (const HighlightingRule& rule : highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    
    // Handle multi-line comments
    setCurrentBlockState(0);
    
    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = commentStartExpression.indexIn(text);
    }
    
    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
        }
        
        setFormat(startIndex, commentLength, commentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
} 