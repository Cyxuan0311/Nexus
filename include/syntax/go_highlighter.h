#ifndef GO_HIGHLIGHTER_H
#define GO_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>
#include <QTextDocument>

struct GoHighlightingRule {
    QRegExp pattern;
    QTextCharFormat format;
};

class GoHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit GoHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    QVector<GoHighlightingRule> highlightingRules_;
    
    // Format definitions
    QTextCharFormat keywordFormat_;
    QTextCharFormat typeFormat_;
    QTextCharFormat builtinFormat_;
    QTextCharFormat stringFormat_;
    QTextCharFormat rawStringFormat_;
    QTextCharFormat runeLiteralFormat_;
    QTextCharFormat numberFormat_;
    QTextCharFormat functionFormat_;
    QTextCharFormat commentFormat_;
    QTextCharFormat operatorFormat_;
    QTextCharFormat packageFormat_;
    QTextCharFormat importFormat_;
    QTextCharFormat interfaceFormat_;
    QTextCharFormat structFormat_;
    
    // Multi-line comment handling
    QRegExp commentStartExpression_;
    QRegExp commentEndExpression_;
};

#endif // GO_HIGHLIGHTER_H 