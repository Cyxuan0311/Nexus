#include "markdown_highlighter.h"
#include <QTextDocument>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument* parent)
	: QSyntaxHighlighter(parent) {
	// VSCode-like colors
	headingFormat_.setForeground(QColor("#4EC9B0"));
	headingFormat_.setFontWeight(QFont::Bold);

	boldFormat_.setForeground(QColor("#D7BA7D"));
	boldFormat_.setFontWeight(QFont::Bold);

	italicFormat_.setForeground(QColor("#D7BA7D"));
	italicFormat_.setFontItalic(true);

	codeFormat_.setForeground(QColor("#C586C0"));
	codeFormat_.setFontFamily("Consolas");

	inlineCodeFormat_.setForeground(QColor("#C586C0"));
	inlineCodeFormat_.setFontFamily("Consolas");

	linkTextFormat_.setForeground(QColor("#569CD6"));
	linkTextFormat_.setFontWeight(QFont::Bold);

	linkUrlFormat_.setForeground(QColor("#4FC1FF"));

	listMarkerFormat_.setForeground(QColor("#4EC9B0"));
	listMarkerFormat_.setFontWeight(QFont::Bold);

	quoteFormat_.setForeground(QColor("#6A9955"));
	quoteFormat_.setFontItalic(true);

	hrFormat_.setForeground(QColor("#6A9955"));

	HighlightingRule rule;

	// Headings: #, ##, ### ... up to ######
	rule.pattern = QRegExp("^(#{1,6})\\s+.*$");
	rule.format = headingFormat_;
	highlightingRules_.append(rule);

	// Bold: **text** or __text__
	rule.pattern = QRegExp("\\*\\*[^*]+\\*\\*|__[^_]+__");
	rule.format = boldFormat_;
	highlightingRules_.append(rule);

	// Italic: *text* or _text_
	rule.pattern = QRegExp("(?<!\\*)\\*[^*]+\\*(?!\\*)|(?<!_)_[^_]+_(?!_)");
	rule.format = italicFormat_;
	highlightingRules_.append(rule);

	// Inline code: `code`
	rule.pattern = QRegExp("`[^`]+`");
	rule.format = inlineCodeFormat_;
	highlightingRules_.append(rule);

	// Links: [text](url)
	rule.pattern = QRegExp("\\[[^\\]]+\\](?=\\()" );
	rule.format = linkTextFormat_;
	highlightingRules_.append(rule);

	rule.pattern = QRegExp("\\([^\\)]+\\)");
	rule.format = linkUrlFormat_;
	highlightingRules_.append(rule);

	// Lists: -, *, + at line start or digits.
	rule.pattern = QRegExp("^(\\s*)([-*+]\\s+|[0-9]+\\.\\s+).*$");
	rule.format = listMarkerFormat_;
	highlightingRules_.append(rule);

	// Blockquote: >
	rule.pattern = QRegExp("^>.*$");
	rule.format = quoteFormat_;
	highlightingRules_.append(rule);

	// Horizontal rule: --- or ***
	rule.pattern = QRegExp("^(\\s*)(---|\*\*\*)\\s*$");
	rule.format = hrFormat_;
	highlightingRules_.append(rule);
}

void MarkdownHighlighter::highlightBlock(const QString& text) {
	for (const HighlightingRule& rule : highlightingRules_) {
		QRegExp expression(rule.pattern);
		int index = expression.indexIn(text);
		while (index >= 0) {
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = expression.indexIn(text, index + length);
		}
	}

	// Code blocks fenced by ```
	static QRegExp fenceStart("^```.*$");
	static QRegExp fenceEnd("^```$");

	setCurrentBlockState(0);
	int startIndex = 0;
	if (previousBlockState() != 1) {
		startIndex = fenceStart.indexIn(text);
	}
	int index = startIndex;
	while (index >= 0) {
		int endIndex = fenceEnd.indexIn(text, index + 1);
		int length;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			length = text.length() - index;
		} else {
			length = endIndex - index + fenceEnd.matchedLength();
		}
		setFormat(index, length, codeFormat_);
		index = fenceStart.indexIn(text, index + length);
	}
} 