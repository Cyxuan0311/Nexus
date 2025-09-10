#ifndef XML_HIGHLIGHTER_H
#define XML_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>

class XmlHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit XmlHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightingRule {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat tagFormat;
    QTextCharFormat attributeFormat;
    QTextCharFormat valueFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat entityFormat;
    QTextCharFormat cdataFormat;
    QTextCharFormat processingInstructionFormat;
};

#endif // XML_HIGHLIGHTER_H 