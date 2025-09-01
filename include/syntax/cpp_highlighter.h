#ifndef CPP_HIGHLIGHTER_H
#define CPP_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>
#include <QTextDocument>

struct CppHighlightingRule {
    QRegExp pattern;
    QTextCharFormat format;
};

class CppHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit CppHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    QVector<CppHighlightingRule> highlightingRules_;
    
    // Format definitions
    QTextCharFormat keywordFormat_;
    QTextCharFormat typeFormat_;
    QTextCharFormat preprocessorFormat_;
    QTextCharFormat stringFormat_;
    QTextCharFormat characterFormat_;
    QTextCharFormat numberFormat_;
    QTextCharFormat functionFormat_;
    QTextCharFormat commentFormat_;
    QTextCharFormat operatorFormat_;
    QTextCharFormat classFormat_;
    QTextCharFormat namespaceFormat_;
    
    // Multi-line comment handling
    QRegExp commentStartExpression_;
    QRegExp commentEndExpression_;
};

#endif // CPP_HIGHLIGHTER_H 