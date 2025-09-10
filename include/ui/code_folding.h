#ifndef CODE_FOLDING_H
#define CODE_FOLDING_H

#include <QPlainTextEdit>
#include <QTextBlock>
#include <QPainter>
#include <QMouseEvent>
#include <QSet>
#include <QWidget>

class FoldingTextEdit;

class CodeFoldingArea : public QWidget {
    Q_OBJECT

public:
    explicit CodeFoldingArea(FoldingTextEdit* editor);
    
    void foldAll();
    void unfoldAll();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

public slots:
    void updateFoldableLines();

private:
    FoldingTextEdit* editor_;
    QSet<int> foldableLines_;
    QSet<int> foldedBlocks_;
    
    // Helper methods
    int getBlockNumberAtY(int y) const;
    bool isFoldableXmlTag(const QString& text) const;
    void foldBlock(int blockNumber);
    void unfoldBlock(int blockNumber);
    QString extractTagName(const QString& text) const;
    QTextBlock findClosingTag(const QTextBlock& startBlock, const QString& tagName) const;
    bool isBlockInFold(int blockNumber) const;
};

class FoldingTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit FoldingTextEdit(QWidget* parent = nullptr);
    
    void foldAll();
    void unfoldAll();
    CodeFoldingArea* getFoldingArea() const;
    
    // Public access to protected methods
    QRectF blockBoundingGeometry(const QTextBlock& block) const;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    CodeFoldingArea* foldingArea_;
};

#endif // CODE_FOLDING_H 