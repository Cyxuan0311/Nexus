#include "cpp_highlighter.h"
#include <QTextDocument>

CppHighlighter::CppHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    
    // VSCode-like C++ colors
    
    // Keywords (purple)
    keywordFormat_.setForeground(QColor("#C586C0"));
    keywordFormat_.setFontWeight(QFont::Bold);
    
    // Basic types (blue)
    typeFormat_.setForeground(QColor("#569CD6"));
    typeFormat_.setFontWeight(QFont::Bold);
    
    // Preprocessor directives (purple)
    preprocessorFormat_.setForeground(QColor("#C586C0"));
    preprocessorFormat_.setFontWeight(QFont::Bold);
    
    // String literals (orange)
    stringFormat_.setForeground(QColor("#CE9178"));
    
    // Character literals (orange)
    characterFormat_.setForeground(QColor("#CE9178"));
    
    // Numbers (light green)
    numberFormat_.setForeground(QColor("#B5CEA8"));
    
    // Function names (yellow)
    functionFormat_.setForeground(QColor("#DCDCAA"));
    functionFormat_.setFontWeight(QFont::Bold);
    
    // Comments (green)
    commentFormat_.setForeground(QColor("#6A9955"));
    commentFormat_.setFontItalic(true);
    
    // Operators (white)
    operatorFormat_.setForeground(QColor("#D4D4D4"));
    
    // Class names (light blue)
    classFormat_.setForeground(QColor("#4EC9B0"));
    classFormat_.setFontWeight(QFont::Bold);
    
    // Namespace (light blue)
    namespaceFormat_.setForeground(QColor("#4EC9B0"));
    
    CppHighlightingRule rule;
    
    // C++ keywords
    QStringList keywordPatterns = {
        "\\bauto\\b", "\\bbreak\\b", "\\bcase\\b", "\\bcatch\\b", "\\bclass\\b",
        "\\bconst\\b", "\\bconstexpr\\b", "\\bcontinue\\b", "\\bdefault\\b", "\\bdelete\\b",
        "\\bdo\\b", "\\belse\\b", "\\benum\\b", "\\bexplicit\\b", "\\bextern\\b",
        "\\bfalse\\b", "\\bfor\\b", "\\bfriend\\b", "\\bgoto\\b", "\\bif\\b",
        "\\binline\\b", "\\bmutable\\b", "\\bnamespace\\b", "\\bnew\\b", "\\bnoexcept\\b",
        "\\bnullptr\\b", "\\boperator\\b", "\\bprivate\\b", "\\bprotected\\b", "\\bpublic\\b",
        "\\breturn\\b", "\\bsizeof\\b", "\\bstatic\\b", "\\bstruct\\b", "\\bswitch\\b",
        "\\btemplate\\b", "\\bthis\\b", "\\bthrow\\b", "\\btrue\\b", "\\btry\\b",
        "\\btypedef\\b", "\\btypename\\b", "\\bunion\\b", "\\busing\\b", "\\bvirtual\\b",
        "\\bvolatile\\b", "\\bwhile\\b", "\\boverride\\b", "\\bfinal\\b", "\\bconsteval\\b",
        "\\bconstinit\\b", "\\bconcept\\b", "\\brequires\\b", "\\bco_await\\b", "\\bco_yield\\b",
        "\\bco_return\\b"
    };
    
    for (const QString& pattern : keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat_;
        highlightingRules_.append(rule);
    }
    
    // Basic types
    QStringList typePatterns = {
        "\\bbool\\b", "\\bchar\\b", "\\bchar8_t\\b", "\\bchar16_t\\b", "\\bchar32_t\\b",
        "\\bwchar_t\\b", "\\bshort\\b", "\\bint\\b", "\\blong\\b", "\\bfloat\\b",
        "\\bdouble\\b", "\\bvoid\\b", "\\bsigned\\b", "\\bunsigned\\b", "\\bsize_t\\b",
        "\\bptrdiff_t\\b", "\\buintptr_t\\b", "\\bintptr_t\\b", "\\buint8_t\\b",
        "\\buint16_t\\b", "\\buint32_t\\b", "\\buint64_t\\b", "\\bint8_t\\b",
        "\\bint16_t\\b", "\\bint32_t\\b", "\\bint64_t\\b"
    };
    
    for (const QString& pattern : typePatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = typeFormat_;
        highlightingRules_.append(rule);
    }
    
    // Preprocessor directives
    rule.pattern = QRegExp("^\\s*#\\w+");
    rule.format = preprocessorFormat_;
    highlightingRules_.append(rule);
    
    // String literals
    rule.pattern = QRegExp("\".*\"");
    rule.format = stringFormat_;
    highlightingRules_.append(rule);
    
    // Raw string literals
    rule.pattern = QRegExp("R\"\\([^)]*\\)\"");
    rule.format = stringFormat_;
    highlightingRules_.append(rule);
    
    // Character literals
    rule.pattern = QRegExp("'[^']*'");
    rule.format = characterFormat_;
    highlightingRules_.append(rule);
    
    // Numbers (including hex, octal, binary, floating point)
    rule.pattern = QRegExp("\\b(?:0[xX][0-9a-fA-F]+|0[bB][01]+|0[0-7]*|[1-9][0-9]*(?:\\.[0-9]*)?(?:[eE][+-]?[0-9]+)?[fFlL]?)\\b");
    rule.format = numberFormat_;
    highlightingRules_.append(rule);
    
    // Function calls and definitions
    rule.pattern = QRegExp("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()");
    rule.format = functionFormat_;
    highlightingRules_.append(rule);
    
    // Class names (uppercase words)
    rule.pattern = QRegExp("\\b[A-Z][A-Za-z0-9_]*\\b");
    rule.format = classFormat_;
    highlightingRules_.append(rule);
    
    // Namespace usage
    rule.pattern = QRegExp("\\b[a-z_][a-z0-9_]*::(?!:)");
    rule.format = namespaceFormat_;
    highlightingRules_.append(rule);
    
    // Operators
    rule.pattern = QRegExp("[+\\-*/%=<>!&|^~?:]");
    rule.format = operatorFormat_;
    highlightingRules_.append(rule);
    
    // Single line comments
    rule.pattern = QRegExp("//.*");
    rule.format = commentFormat_;
    highlightingRules_.append(rule);
    
    // Multi-line comments
    commentStartExpression_ = QRegExp("/\\*");
    commentEndExpression_ = QRegExp("\\*/");
}

void CppHighlighter::highlightBlock(const QString& text) {
    // Apply single-line highlighting rules
    for (const CppHighlightingRule& rule : highlightingRules_) {
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
        startIndex = commentStartExpression_.indexIn(text);
    }
    
    while (startIndex >= 0) {
        int endIndex = commentEndExpression_.indexIn(text, startIndex);
        int commentLength;
        
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + commentEndExpression_.matchedLength();
        }
        
        setFormat(startIndex, commentLength, commentFormat_);
        startIndex = commentStartExpression_.indexIn(text, startIndex + commentLength);
    }
} 