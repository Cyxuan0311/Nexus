#ifndef PYTHON_HIGHLIGHTER_H
#define PYTHON_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>
#include <QTextDocument>

struct PythonHighlightingRule {
    QRegExp pattern;
    QTextCharFormat format;
};

class PythonHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit PythonHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    QVector<PythonHighlightingRule> highlightingRules_;
    
    // Format definitions
    QTextCharFormat keywordFormat_;
    QTextCharFormat builtinFormat_;
    QTextCharFormat stringFormat_;
    QTextCharFormat rawStringFormat_;
    QTextCharFormat numberFormat_;
    QTextCharFormat functionFormat_;
    QTextCharFormat classFormat_;
    QTextCharFormat commentFormat_;
    QTextCharFormat operatorFormat_;
    QTextCharFormat decoratorFormat_;
    QTextCharFormat selfFormat_;
    QTextCharFormat importFormat_;
    
    // Multi-line string handling
    QRegExp tripleQuoteStartExpression_;
    QRegExp tripleQuoteEndExpression_;
    QRegExp tripleSingleQuoteStartExpression_;
    QRegExp tripleSingleQuoteEndExpression_;
};

#endif // PYTHON_HIGHLIGHTER_H 