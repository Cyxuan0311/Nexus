#include "python_highlighter.h"
#include <QTextDocument>

PythonHighlighter::PythonHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    
    // VSCode-like Python colors
    
    // Keywords (purple)
    keywordFormat_.setForeground(QColor("#C586C0"));
    keywordFormat_.setFontWeight(QFont::Bold);
    
    // Built-in functions and types (blue)
    builtinFormat_.setForeground(QColor("#569CD6"));
    builtinFormat_.setFontWeight(QFont::Bold);
    
    // String literals (orange)
    stringFormat_.setForeground(QColor("#CE9178"));
    
    // Raw strings (orange)
    rawStringFormat_.setForeground(QColor("#CE9178"));
    
    // Numbers (light green)
    numberFormat_.setForeground(QColor("#B5CEA8"));
    
    // Function names (yellow)
    functionFormat_.setForeground(QColor("#DCDCAA"));
    functionFormat_.setFontWeight(QFont::Bold);
    
    // Class names (light blue)
    classFormat_.setForeground(QColor("#4EC9B0"));
    classFormat_.setFontWeight(QFont::Bold);
    
    // Comments (green)
    commentFormat_.setForeground(QColor("#6A9955"));
    commentFormat_.setFontItalic(true);
    
    // Operators (white)
    operatorFormat_.setForeground(QColor("#D4D4D4"));
    
    // Decorators (yellow)
    decoratorFormat_.setForeground(QColor("#DCDCAA"));
    decoratorFormat_.setFontWeight(QFont::Bold);
    
    // Self keyword (purple, italic)
    selfFormat_.setForeground(QColor("#C586C0"));
    selfFormat_.setFontItalic(true);
    
    // Import statements (purple)
    importFormat_.setForeground(QColor("#C586C0"));
    importFormat_.setFontWeight(QFont::Bold);
    
    PythonHighlightingRule rule;
    
    // Python keywords
    QStringList keywordPatterns = {
        "\\bFalse\\b", "\\bNone\\b", "\\bTrue\\b", "\\band\\b", "\\bas\\b",
        "\\bassert\\b", "\\bbreak\\b", "\\bclass\\b", "\\bcontinue\\b", "\\bdef\\b",
        "\\bdel\\b", "\\belif\\b", "\\belse\\b", "\\bexcept\\b", "\\bfinally\\b",
        "\\bfor\\b", "\\bfrom\\b", "\\bglobal\\b", "\\bif\\b", "\\bimport\\b",
        "\\bin\\b", "\\bis\\b", "\\blambda\\b", "\\bnonlocal\\b", "\\bnot\\b",
        "\\bor\\b", "\\bpass\\b", "\\braise\\b", "\\breturn\\b", "\\btry\\b",
        "\\bwhile\\b", "\\bwith\\b", "\\byield\\b", "\\basync\\b", "\\bawait\\b"
    };
    
    for (const QString& pattern : keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat_;
        highlightingRules_.append(rule);
    }
    
    // Built-in functions and types
    QStringList builtinPatterns = {
        "\\babs\\b", "\\ball\\b", "\\bany\\b", "\\bbin\\b", "\\bbool\\b",
        "\\bbytearray\\b", "\\bbytes\\b", "\\bcallable\\b", "\\bchr\\b", "\\bclassmethod\\b",
        "\\bcompile\\b", "\\bcomplex\\b", "\\bdelattr\\b", "\\bdict\\b", "\\bdir\\b",
        "\\bdivmod\\b", "\\benumerate\\b", "\\beval\\b", "\\bexec\\b", "\\bfilter\\b",
        "\\bfloat\\b", "\\bformat\\b", "\\bfrozenset\\b", "\\bgetattr\\b", "\\bglobals\\b",
        "\\bhasattr\\b", "\\bhash\\b", "\\bhelp\\b", "\\bhex\\b", "\\bid\\b",
        "\\binput\\b", "\\bint\\b", "\\bisinstance\\b", "\\bissubclass\\b", "\\biter\\b",
        "\\blen\\b", "\\blist\\b", "\\blocals\\b", "\\bmap\\b", "\\bmax\\b",
        "\\bmemoryview\\b", "\\bmin\\b", "\\bnext\\b", "\\bobject\\b", "\\boct\\b",
        "\\bopen\\b", "\\bord\\b", "\\bpow\\b", "\\bprint\\b", "\\bproperty\\b",
        "\\brange\\b", "\\brepr\\b", "\\breversed\\b", "\\bround\\b", "\\bset\\b",
        "\\bsetattr\\b", "\\bslice\\b", "\\bsorted\\b", "\\bstaticmethod\\b", "\\bstr\\b",
        "\\bsum\\b", "\\bsuper\\b", "\\btuple\\b", "\\btype\\b", "\\bvars\\b", "\\bzip\\b"
    };
    
    for (const QString& pattern : builtinPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = builtinFormat_;
        highlightingRules_.append(rule);
    }
    
    // Self keyword
    rule.pattern = QRegExp("\\bself\\b");
    rule.format = selfFormat_;
    highlightingRules_.append(rule);
    
    // Decorators
    rule.pattern = QRegExp("@\\w+");
    rule.format = decoratorFormat_;
    highlightingRules_.append(rule);
    
    // Function definitions
    rule.pattern = QRegExp("\\bdef\\s+(\\w+)");
    rule.format = functionFormat_;
    highlightingRules_.append(rule);
    
    // Class definitions
    rule.pattern = QRegExp("\\bclass\\s+(\\w+)");
    rule.format = classFormat_;
    highlightingRules_.append(rule);
    
    // Function calls
    rule.pattern = QRegExp("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()");
    rule.format = functionFormat_;
    highlightingRules_.append(rule);
    
    // String literals - single quotes
    rule.pattern = QRegExp("'([^'\\\\]|\\\\.)*'");
    rule.format = stringFormat_;
    highlightingRules_.append(rule);
    
    // String literals - double quotes
    rule.pattern = QRegExp("\"([^\"\\\\]|\\\\.)*\"");
    rule.format = stringFormat_;
    highlightingRules_.append(rule);
    
    // Raw strings
    rule.pattern = QRegExp("r['\"]([^'\"\\\\]|\\\\.)*['\"]");
    rule.format = rawStringFormat_;
    highlightingRules_.append(rule);
    
    // f-strings
    rule.pattern = QRegExp("f['\"]([^'\"\\\\]|\\\\.)*['\"]");
    rule.format = stringFormat_;
    highlightingRules_.append(rule);
    
    // Numbers (integers, floats, scientific notation)
    rule.pattern = QRegExp("\\b(?:0[xX][0-9a-fA-F]+|0[oO][0-7]+|0[bB][01]+|\\d+(?:\\.\\d*)?(?:[eE][+-]?\\d+)?[jJ]?)\\b");
    rule.format = numberFormat_;
    highlightingRules_.append(rule);
    
    // Operators
    rule.pattern = QRegExp("[+\\-*/%=<>!&|^~]|\\*\\*|//|<<|>>|<=|>=|==|!=|\\+=|\\-=|\\*=|/=|%=|&=|\\|=|\\^=|<<=|>>=|\\*\\*=|//=");
    rule.format = operatorFormat_;
    highlightingRules_.append(rule);
    
    // Single line comments
    rule.pattern = QRegExp("#.*");
    rule.format = commentFormat_;
    highlightingRules_.append(rule);
    
    // Multi-line strings (triple quotes)
    tripleQuoteStartExpression_ = QRegExp("\"\"\"");
    tripleQuoteEndExpression_ = QRegExp("\"\"\"");
    tripleSingleQuoteStartExpression_ = QRegExp("'''");
    tripleSingleQuoteEndExpression_ = QRegExp("'''");
}

void PythonHighlighter::highlightBlock(const QString& text) {
    // Apply single-line highlighting rules
    for (const PythonHighlightingRule& rule : highlightingRules_) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    
    // Handle multi-line strings (triple double quotes)
    setCurrentBlockState(0);
    
    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = tripleQuoteStartExpression_.indexIn(text);
    }
    
    while (startIndex >= 0) {
        int endIndex = tripleQuoteEndExpression_.indexIn(text, startIndex + 3);
        int stringLength;
        
        if (endIndex == -1) {
            setCurrentBlockState(1);
            stringLength = text.length() - startIndex;
        } else {
            stringLength = endIndex - startIndex + tripleQuoteEndExpression_.matchedLength();
        }
        
        setFormat(startIndex, stringLength, stringFormat_);
        startIndex = tripleQuoteStartExpression_.indexIn(text, startIndex + stringLength);
    }
    
    // Handle multi-line strings (triple single quotes)
    // Use state 2 for triple single quotes
    if (previousBlockState() != 2) {
        startIndex = tripleSingleQuoteStartExpression_.indexIn(text);
    } else {
        startIndex = 0;
    }
    
    while (startIndex >= 0) {
        int endIndex = tripleSingleQuoteEndExpression_.indexIn(text, startIndex + 3);
        int stringLength;
        
        if (endIndex == -1) {
            setCurrentBlockState(2);
            stringLength = text.length() - startIndex;
        } else {
            stringLength = endIndex - startIndex + tripleSingleQuoteEndExpression_.matchedLength();
        }
        
        setFormat(startIndex, stringLength, stringFormat_);
        startIndex = tripleSingleQuoteStartExpression_.indexIn(text, startIndex + stringLength);
    }
} 