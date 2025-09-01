#ifndef MARKDOWN_HIGHLIGHTER_H
#define MARKDOWN_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>

class MarkdownHighlighter : public QSyntaxHighlighter {
	Q_OBJECT

public:
	explicit MarkdownHighlighter(QTextDocument* parent = nullptr);

protected:
	void highlightBlock(const QString& text) override;

private:
	struct HighlightingRule {
		QRegExp pattern;
		QTextCharFormat format;
	};

	QVector<HighlightingRule> highlightingRules_;

	QTextCharFormat headingFormat_;
	QTextCharFormat boldFormat_;
	QTextCharFormat italicFormat_;
	QTextCharFormat codeFormat_;
	QTextCharFormat inlineCodeFormat_;
	QTextCharFormat linkTextFormat_;
	QTextCharFormat linkUrlFormat_;
	QTextCharFormat listMarkerFormat_;
	QTextCharFormat quoteFormat_;
	QTextCharFormat hrFormat_;
};

#endif // MARKDOWN_HIGHLIGHTER_H 