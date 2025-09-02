#include "go_highlighter.h"
#include <QTextDocument>

GoHighlighter::GoHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    
    // VSCode-like Go colors
    
    // Keywords (purple)
    keywordFormat_.setForeground(QColor("#C586C0"));
    keywordFormat_.setFontWeight(QFont::Bold);
    
    // Basic types (blue)
    typeFormat_.setForeground(QColor("#569CD6"));
    typeFormat_.setFontWeight(QFont::Bold);
    
    // Built-in functions (blue)
    builtinFormat_.setForeground(QColor("#569CD6"));
    builtinFormat_.setFontWeight(QFont::Bold);
    
    // String literals (orange)
    stringFormat_.setForeground(QColor("#CE9178"));
    
    // Raw string literals (orange)
    rawStringFormat_.setForeground(QColor("#CE9178"));
    
    // Rune literals (orange)
    runeLiteralFormat_.setForeground(QColor("#CE9178"));
    
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
    
    // Package names (light blue)
    packageFormat_.setForeground(QColor("#4EC9B0"));
    packageFormat_.setFontWeight(QFont::Bold);
    
    // Import statements (purple)
    importFormat_.setForeground(QColor("#C586C0"));
    importFormat_.setFontWeight(QFont::Bold);
    
    // Interface names (light blue)
    interfaceFormat_.setForeground(QColor("#4EC9B0"));
    interfaceFormat_.setFontWeight(QFont::Bold);
    
    // Struct names (light blue)
    structFormat_.setForeground(QColor("#4EC9B0"));
    structFormat_.setFontWeight(QFont::Bold);
    
    GoHighlightingRule rule;
    
    // Go keywords
    QStringList keywordPatterns = {
        "\\bbreak\\b", "\\bcase\\b", "\\bchan\\b", "\\bconst\\b", "\\bcontinue\\b",
        "\\bdefault\\b", "\\bdefer\\b", "\\belse\\b", "\\bfallthrough\\b", "\\bfor\\b",
        "\\bfunc\\b", "\\bgo\\b", "\\bgoto\\b", "\\bif\\b", "\\bimport\\b",
        "\\binterface\\b", "\\bmap\\b", "\\bpackage\\b", "\\brange\\b", "\\breturn\\b",
        "\\bselect\\b", "\\bstruct\\b", "\\bswitch\\b", "\\btype\\b", "\\bvar\\b"
    };
    
    for (const QString& pattern : keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat_;
        highlightingRules_.append(rule);
    }
    
    // Basic types
    QStringList typePatterns = {
        "\\bbool\\b", "\\bbyte\\b", "\\bcomplex64\\b", "\\bcomplex128\\b",
        "\\berror\\b", "\\bfloat32\\b", "\\bfloat64\\b", "\\bint\\b", "\\bint8\\b",
        "\\bint16\\b", "\\bint32\\b", "\\bint64\\b", "\\brune\\b", "\\bstring\\b",
        "\\buint\\b", "\\buint8\\b", "\\buint16\\b", "\\buint32\\b", "\\buint64\\b",
        "\\buintptr\\b"
    };
    
    for (const QString& pattern : typePatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = typeFormat_;
        highlightingRules_.append(rule);
    }
    
    // Built-in functions
    QStringList builtinPatterns = {
        "\\bappend\\b", "\\bcap\\b", "\\bclose\\b", "\\bcomplex\\b", "\\bcopy\\b",
        "\\bdelete\\b", "\\bimag\\b", "\\blen\\b", "\\bmake\\b", "\\bnew\\b",
        "\\bpanic\\b", "\\bprint\\b", "\\bprintln\\b", "\\breal\\b", "\\brecover\\b"
    };
    
    for (const QString& pattern : builtinPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = builtinFormat_;
        highlightingRules_.append(rule);
    }
    
    // Constants
    QStringList constantPatterns = {
        "\\btrue\\b", "\\bfalse\\b", "\\biota\\b", "\\bnil\\b"
    };
    
    for (const QString& pattern : constantPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = typeFormat_;
        highlightingRules_.append(rule);
    }
    
    // Package declaration
    rule.pattern = QRegExp("\\bpackage\\s+(\\w+)");
    rule.format = packageFormat_;
    highlightingRules_.append(rule);
    
    // Import statements
    rule.pattern = QRegExp("\\bimport\\b");
    rule.format = importFormat_;
    highlightingRules_.append(rule);
    
    // Function definitions
    rule.pattern = QRegExp("\\bfunc\\s+(\\w+)");
    rule.format = functionFormat_;
    highlightingRules_.append(rule);
    
    // Method definitions (with receiver)
    rule.pattern = QRegExp("\\bfunc\\s*\\([^)]*\\)\\s*(\\w+)");
    rule.format = functionFormat_;
    highlightingRules_.append(rule);
    
    // Function calls
    rule.pattern = QRegExp("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()");
    rule.format = functionFormat_;
    highlightingRules_.append(rule);
    
    // Type definitions
    rule.pattern = QRegExp("\\btype\\s+(\\w+)");
    rule.format = structFormat_;
    highlightingRules_.append(rule);
    
    // Struct definitions
    rule.pattern = QRegExp("\\bstruct\\s*\\{");
    rule.format = structFormat_;
    highlightingRules_.append(rule);
    
    // Interface definitions
    rule.pattern = QRegExp("\\binterface\\s*\\{");
    rule.format = interfaceFormat_;
    highlightingRules_.append(rule);
    
    // String literals - double quotes
    rule.pattern = QRegExp("\"([^\"\\\\]|\\\\.)*\"");
    rule.format = stringFormat_;
    highlightingRules_.append(rule);
    
    // Raw string literals - backticks
    rule.pattern = QRegExp("`[^`]*`");
    rule.format = rawStringFormat_;
    highlightingRules_.append(rule);
    
    // Rune literals - single quotes
    rule.pattern = QRegExp("'([^'\\\\]|\\\\.)*'");
    rule.format = runeLiteralFormat_;
    highlightingRules_.append(rule);
    
    // Numbers (integers, floats, hex, octal, binary)
    rule.pattern = QRegExp("\\b(?:0[xX][0-9a-fA-F]+|0[oO][0-7]+|0[bB][01]+|\\d+(?:\\.\\d*)?(?:[eE][+-]?\\d+)?[i]?)\\b");
    rule.format = numberFormat_;
    highlightingRules_.append(rule);
    
    // Operators
    rule.pattern = QRegExp("[+\\-*/%=<>!&|^~]|\\+\\+|\\-\\-|<<|>>|<=|>=|==|!=|&&|\\|\\||<\\-|\\.\\.\\.|:=|\\+=|\\-=|\\*=|/=|%=|&=|\\|=|\\^=|<<=|>>=|&\\^|&\\^=");
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

void GoHighlighter::highlightBlock(const QString& text) {
    // Apply single-line highlighting rules
    for (const GoHighlightingRule& rule : highlightingRules_) {
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